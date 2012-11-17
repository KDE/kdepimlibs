/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/
#ifndef AKONADI_INCIDENCECHANGER_H
#define AKONADI_INCIDENCECHANGER_H

#include "akonadi-calendar_export.h"

#include <akonadi/item.h>
#include <akonadi/collection.h>
#include <kcalcore/incidence.h>

#include <QWidget>

namespace Akonadi {

/**
 * @short IncidenceChanger is the prefered way to easily create, modify and delete incidences.
 *
 * It hides the communication with akonadi from the library user.
 *
 * It provides the following features that ItemCreateJob, ItemModifyJob and
 * ItemDeleteJob do not:
 * - Sending groupware ( iTip ) messages to attendees and organizers.
 * - Aware of recurrences, allowing to only change one occurrence.
 * - Undo/Redo
 * - Group operations which are executed in an atomic manner.
 * - Collection ACLs
 * - Error dialogs with calendaring lingo
 *
 * In the context of this API, "change", means "creation", "deletion" or incidence "modification".
 *
 * @code
 * IncidenceChanger *changer = new IncidenceChanger( parent );
 * connect( changer,
 *          SIGNAL(createFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
 *          SLOT(slotCreateFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );
 * 
 * connect( changer,
 *          SIGNAL(deleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)),
 *          SLOT(slotDeleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)) );
 *
 * connect( changer,SIGNAL(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
 *          SLOT(slotModifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );
 *
 * changer->setDestinationPolicy( IncidenceChanger::DestinationPolicyAsk );
 *
 * KCalCore::Incidence::Ptr incidence = (...);
 * int changeId = changer->createIncidence( incidence, Akonadi::Collection() );
 *
 * 
 * if ( changeId == -1 )
 * {
 *  // Invalid parameters, incidence is null.
 * }
 * 
 * @endcode
 *
 * @author Sérgio Martins <iamsergio@gmail.com>
 * @since 4.11
 */

class History;

class AKONADI_CALENDAR_EXPORT IncidenceChanger : public QObject
{
  Q_OBJECT
public:
  /**
    * This enum describes result codes which are returned by createFinished(),
    * modifyfinished() and deleteFinished() signals.
    */
  enum ResultCode {
    ResultCodeSuccess = 0,
    ResultCodeJobError, ///< ItemCreateJob, ItemModifyJob or ItemDeleteJob weren't successfull
    ResultCodeAlreadyDeleted, ///< That incidence was already deleted, or currently being deleted.
    ResultCodeInvalidDefaultCollection, ///< Default collection is invalid and DestinationPolicyNeverAsk was used
    ResultCodeRolledback, ///< One change belonging to an atomic operation failed. All other changes were rolled back.
    ResultCodePermissions, ///< The parent collection doesn't have ACLs for this operation
    ResultCodeUserCanceled, ///< User canceled the operation
    ResultCodeInvalidUserCollection, ///< User somehow chose an invalid collection in the collection dialog ( should not happen )
    ResultCodeModificationDiscarded, ///< A new modification came in, the old one is discarded
    ResultCodeDuplicateId ///< Duplicate Akonadi::Item::Ids must be unique in group operations
  };

  /**
    * This enum describes destination policies.
    * Destination policies control how the createIncidence() method chooses the
    * collection where the item will be created.
    */
  enum DestinationPolicy {
    DestinationPolicyDefault, ///< The default collection is used, if it's invalid, the user is prompted. @see setDefaultCollection().
    DestinationPolicyAsk,     ///< User is always asked which collection to use.
    DestinationPolicyNeverAsk ///< The default collection is used, if it's invalid, an error is returned, and the incidence isn't added.
  };

  /**
    * This enum describes change types.
    */
  enum ChangeType {
    ChangeTypeCreate,   ///> Represents an incidence creation.
    ChangeTypeModify,   ///> Represents an incidence modification.
    ChangeTypeDelete    ///> Represents an incidence deletion.
  };

  /**
    * Creates a new IncidenceChanger instance.
    * @param parent parent QObject
    */
  explicit IncidenceChanger( QObject *parent = 0 );

  /**
    * Destroys this IncidenceChanger instance.
    */
  ~IncidenceChanger();

  /**
    * Creates a new incidence.
    *
    * @param incidence Incidence to create, must be valid.
    * @param collection Collection where the incidence will be created. If invalid, one according
    *                   to the DestinationPolicy will be used. You can know which collection was
    *                   used by calling lastCollectionUsed();
    * @param parent widget parent to be used in dialogs.
    *
    * @return Returns an integer which identifies this change. This identifier is useful
    *         to correlate this operation with the IncidenceChanger::createFinished() signal.
    *
    *         Returns -1 if @p incidence is invalid. The createFinished() signal
    *         won't be emitted in this case.
    */
  int createIncidence( const KCalCore::Incidence::Ptr &incidence,
                        const Akonadi::Collection &collection = Akonadi::Collection(),
                        QWidget *parent = 0 );

  /**
    * Deletes an incidence. If it's recurring, all occurrences are deleted.
    *
    * @param item Item to delete. Item must be valid.
    * @param parent Parent to be used in dialogs.
    *
    * @return Returns an integer which identifies this deletion. This identifier is useful
    *         to correlate this deletion with the IncidenceChanger::deleteFinished() signal.
    *
    *         Returns -1 if item is invalid. The deleteFinished() signal won't be emitted in this
    *         case.
    */
  int deleteIncidence( const Akonadi::Item &item, QWidget *parent = 0 );

  /**
    * Deletes a list of Items.
    *
    * @param items List of items do delete. They must be valid.
    * @param parent Parent to be used in dialogs.
    * @return Returns an integer which identifies this deletion. This identifier is useful
    *         to correlate this operation with the IncidenceChanger::deleteFinished() signal.
    *
    *         Returns -1 if any item is invalid or if @p items is empty. The deleteFinished() signal
    *         won't be emitted in this case.
    */
  int deleteIncidences( const Akonadi::Item::List &items, QWidget *parent = 0 );

  /**
    * Modifies an incidence.
    *
    * @param item A valid item, with the new payload.
    * @param originalPayload The payload before the modification. If invalid it won't be recorded
    *                        to the undo stack and groupware functionality won't be used for this
    *                        deletion.
    * @param parent Parent to be used in dialogs.
    *
    * @return Returns an integer which identifies this modification. This identifier is useful
    *         to correlate this operation with the IncidenceChanger::modifyFinished() signal.
    *
    *         Returns -1 if the item doesn't have a valid payload. The modifyFinished() signal
    *         won't be emitted in this case.
    */
  int modifyIncidence( const Akonadi::Item &item,
                        const KCalCore::Incidence::Ptr &originalPayload = KCalCore::Incidence::Ptr(),
                        QWidget *parent = 0 );

  /**
    * Some incidence operations require more than one change. Like dissociating
    * occurrences, which needs an incidence add and an incidence change.
    *
    * If you want to prevent that the same dialogs are presented multiple times
    * use this function to start a batch operation.
    *
    * If one change belonging to a batch operation fails, all other changes
    * are rolled back.
    *
    * @param operationDescription Describes what the atomic operation does.
    *        This will be what incidenceChanger->history()->descriptionForNextUndo()
    *        if you have history enabled.
    *
    * @see endAtomicOperation()
    */
  void startAtomicOperation( const QString &operationDescription = QString() );

  /**
    * Tells IncidenceChanger you finished doing changes that belong to a
    * batch operation.
    *
    * @see startAtomicOperation()
    */
  void endAtomicOperation();

  /**
    * Sets the default collection.
    * @param collection The collection to be used in createIncidence() if the
    *        proper destination policy is set.
    * @see createIncidence()
    * @see destinationPolicy()
    * @see defaultCollection()
    */
  void setDefaultCollection( const Akonadi::Collection &collection );

  /**
    * Returns the defaultCollection.
    * If none is set, an invalid Collection is returned.
    * @see setDefaultCollection()
    * @see DestinationPolicy
    */
  Akonadi::Collection defaultCollection() const;

  /**
    * Sets the destination policy to use. The destination policy determines the
    * collection to use in createIncidence()
    *
    * @see createIncidence()
    * @see destinationPolicy()
    */
  void setDestinationPolicy( DestinationPolicy destinationPolicy );

  /**
    * Returns the current destination policy.
    * If none is set, DestinationPolicyDefault is returned.
    * @see setDestinationPolicy()
    * @see DestinationPolicy
    */
  DestinationPolicy destinationPolicy() const;

  /**
    * Sets if IncidenceChanger should show error dialogs.
    */
  void setShowDialogsOnError( bool enable );

  /**
    * Returns true if error dialogs are shown by IncidenceChanger.
    * The default is true.
    *
    * @see setShowDialogsOnError()
    */
  bool showDialogsOnError() const;

  /**
    * Sets if IncidenceChanger should honour collection's ACLs by disallowing changes if
    * necessary.
    */
  void setRespectsCollectionRights( bool respect );

  /**
    * Returns true if IncidenceChanger honours collection's ACLs by disallowing
    * changes if necessary.
    *
    * The default is true.
    * @see setRespectsCollectionRights()
    * @see ResultCode::ResultCodePermissions
    */
  bool respectsCollectionRights() const;

  /**
    * Enable or disable history.
    * With history enabled all changes are recored into the undo/redo stack.
    *
    * @see history()
    * @see historyEnabled()
    */
  void setHistoryEnabled( bool enable );

  /**
    * Returns true if changes are added into the undo stack.
    * Default is true.
    *
    * @see history()
    * @see historyEnabled()
    */
  bool historyEnabled() const;

  /**
    * Returns a pointer to the history object.
    * It's always valid.
    * Ownership remains with IncidenceChanger.
    */
  History* history() const;

  /**
    * For performance reasons, IncidenceChanger internaly caches the ids of the last deleted items,
    * to avoid creating useless delete jobs.
    *
    * This function exposes that functionality so it can be used in other scenarios.
    * One popular scenario is when you're using an ETM and the user is deleting items very fast,
    * ETM doesn't know about the deletions immediately, so it can happen that some items are
    * deleted more than once, resulting in an error.
    *
    * @return true if the item was deleted recently, false otherwise.
    */
  bool deletedRecently( Akonadi::Item::Id ) const;

  /**
    * Enables or disabled groupware communication.
    * With groupware communication enabled, invitations and update e-mails will be sent to each
    * attendee.
    */
  void setGroupwareCommunication( bool enabled );

  /**
    * Returns if we're using groupware communication.
    * @see setGroupwareCommuniation()
    */
  bool groupwareCommunication() const;

  /**
    * Returns the collection that the last createIncidence() used.
    * Will invalid if no incidences were created yet.
    *
    * @see createIncidence().
    */
  Akonadi::Collection lastCollectionUsed() const;

Q_SIGNALS:
  /**
    * Emitted when IncidenceChanger creates an Incidence in akonadi.
    *
    * @param changeId the unique identifier of this change, returned by createIncidence().
    * @param item the created item, might be invalid if the @p resultCode is not ResultCodeSuccess
    * @param resultCode success/error code
    * @param errorString if @p resultCode is not ResultCodeSuccess, contains an i18n'ed error
    *        message. If you enabled error dialogs, this string was already presented to the user.
    */
  void createFinished( int changeId,
                        const Akonadi::Item &item,
                        Akonadi::IncidenceChanger::ResultCode resultCode,
                        const QString &errorString );
  /**
    * Emitted when IncidenceChanger modifies an Incidence in akonadi.
    *
    * @param changeId the unique identifier of this change, returned by modifyIncidence().
    * @param item the modified item, might be invalid if the @p resultCode is not ResultCodeSuccess
    * @param resultCode success/error code
    * @param errorString if @p resultCode is not ResultCodeSuccess, contains an i18n'ed error
    *        message. If you enabled error dialogs, this string was already presented to the user.
    */
  void modifyFinished( int changeId,
                        const Akonadi::Item &item,
                        Akonadi::IncidenceChanger::ResultCode resultCode,
                        const QString &errorString );
  /**
    * Emitted when IncidenceChanger deletes an Incidence in akonadi.
    *
    * @param changeId the unique identifier of this change, returned by deletedIncidence().
    * @param itemIdList list of deleted item ids, might be emptu if the @p resultCode is not
    *                   ResultCodeSuccess
    * @param resultCode success/error code
    * @param errorString if @p resultCode is not ResultCodeSuccess, contains an i18n'ed error
    *        message. If you enabled error dialogs, this string was already presented to the user.
    */
  void deleteFinished( int changeId,
                        const QVector<Akonadi::Item::Id> &itemIdList,
                        Akonadi::IncidenceChanger::ResultCode resultCode,
                        const QString &errorString );

private:
  //@cond PRIVATE
  friend class History;
  // used internally by the History class
  explicit IncidenceChanger( bool enableHistory, QObject *parent = 0 );
  class Private;
  Private *const d;
  //@endcond
};

}

Q_DECLARE_METATYPE( Akonadi::IncidenceChanger::DestinationPolicy )
Q_DECLARE_METATYPE( Akonadi::IncidenceChanger::ResultCode )
Q_DECLARE_METATYPE( Akonadi::IncidenceChanger::ChangeType )

#endif
