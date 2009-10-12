/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Incidence class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef INCIDENCE_H
#define INCIDENCE_H

#include "kcal_export.h"
#include "incidencebase.h"
#include "alarm.h"
#include "attachment.h"
#include "recurrence.h"

#include <QtCore/QList>

namespace boost {
  template <typename T> class shared_ptr;
}

namespace KCal {

/**
  @brief
  Provides the abstract base class common to non-FreeBusy (Events, To-dos,
  Journals) calendar components known as incidences.

  Several properties are not allowed for VFREEBUSY objects (see rfc:2445),
  so they are not in IncidenceBase. The hierarchy is:

  IncidenceBase
  + FreeBusy
  + Incidence
    + Event
    + Todo
    + Journal

  So IncidenceBase contains all properties that are common to all classes,
  and Incidence contains all additional properties that are common to
  Events, Todos and Journals, but are not allowed for FreeBusy entries.
*/
class KCAL_EXPORT Incidence //krazy:exclude=dpointer since nested class templates confuse krazy
  : public IncidenceBase, public Recurrence::RecurrenceObserver
{
  public:
    /**
      Template for a class that implements a visitor for adding an Incidence
      to a resource supporting addEvent(), addTodo() and addJournal() calls.
    */
    //@cond PRIVATE
    template<class T>
    class AddVisitor : public IncidenceBase::Visitor
    {
      public:
        AddVisitor( T *r ) : mResource( r ) {}

        bool visit( Event *e )
        {
          return mResource->addEvent( e );
        }
        bool visit( Todo *t )
        {
          return mResource->addTodo( t );
        }
        bool visit( Journal *j )
        {
          return mResource->addJournal( j );
        }
        bool visit( FreeBusy * )
        {
          return false;
        }

      private:
        T *mResource;
    };
    //@endcond

    /**
      Template for a class that implements a visitor for deleting an Incidence
      from a resource supporting deleteEvent(), deleteTodo() and deleteJournal()
      calls.
    */
    //@cond PRIVATE
    template<class T>
    class DeleteVisitor : public IncidenceBase::Visitor
    {
      public:
        DeleteVisitor( T *r ) : mResource( r ) {}

        bool visit( Event *e )
        {
          mResource->deleteEvent( e );
          return true;
        }
        bool visit( Todo *t )
        {
          mResource->deleteTodo( t );
          return true;
        }
        bool visit( Journal *j )
        {
          mResource->deleteJournal( j );
          return true;
        }
        bool visit( FreeBusy * )
        {
          return false;
        }

      private:
        T *mResource;
    };
    //@endcond

    /**
      The different types of overall incidence status or confirmation.
      The meaning is specific to the incidence type in context.
    */
    enum Status {
      StatusNone,           /**< No status */
      StatusTentative,      /**< event is tentative */
      StatusConfirmed,      /**< event is definite */
      StatusCompleted,      /**< to-do completed */
      StatusNeedsAction,    /**< to-do needs action */
      StatusCanceled,       /**< event or to-do canceled; journal removed */
      StatusInProcess,      /**< to-do in process */
      StatusDraft,          /**< journal is draft */
      StatusFinal,          /**< journal is final */
      StatusX               /**< a non-standard status string */
    };

    /**
      The different types of incidence access classifications.
    */
    enum Secrecy {
      SecrecyPublic=0,      /**< Not secret (default) */
      SecrecyPrivate=1,     /**< Secret to the owner */
      SecrecyConfidential=2 /**< Secret to the owner and some others */
    };

    /**
      List of incidences.
    */
    typedef ListBase<Incidence> List;

    /**
      A shared pointer to an Incidence.
    */
    typedef boost::shared_ptr<Incidence> Ptr;

    /**
      A shared pointer to a non-mutable Incidence.
    */
    typedef boost::shared_ptr<const Incidence> ConstPtr;

    /**
      Constructs an empty incidence.*
    */
    Incidence();

    /**
      Copy constructor.
      @param other is the incidence to copy.
    */
    Incidence( const Incidence &other );

    /**
      Destroys an incidence.
    */
    ~Incidence();

    /**
      Returns an exact copy of this incidence. The returned object is owned
      by the caller.
    */
    virtual Incidence *clone() = 0; //TODO KDE5: make this const

    /**
      Set readonly state of incidence.

      @param readonly If true, the incidence is set to readonly, if false the
                      incidence is set to readwrite.
    */
    void setReadOnly( bool readonly );

    /**
      @copydoc
      IncidenceBase::setAllDay().
    */
    void setAllDay( bool allDay );

    /**
      Recreate event. The event is made a new unique event, but already stored
      event information is preserved. Sets uniquie id, creation date, last
      modification date and revision number.
    */
    void recreate();

    /**
      Sets the incidence creation date/time. It is stored as a UTC date/time.

      @param dt is the creation date/time.
      @see created().
    */
    void setCreated( const KDateTime &dt );

    /**
      Returns the incidence creation date/time.
      @see setCreated().
    */
    KDateTime created() const;

    /**
      Sets the number of revisions this incidence has seen.

      @param rev is the incidence revision number.
      @see revision().
    */
    void setRevision( int rev );

    /**
      Returns the number of revisions this incidence has seen.
      @see setRevision().
    */
    int revision() const;

    /**
      Sets the incidence starting date/time.

      @param dt is the starting date/time.
      @see IncidenceBase::dtStart().
    */
    virtual void setDtStart( const KDateTime &dt );

    /**
      Returns the incidence ending date/time.
      @see IncidenceBase::dtStart().
    */
    virtual KDateTime dtEnd() const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    /**
      Sets the incidence description.

      @param description is the incidence description string.
      @param isRich if true indicates the description string contains richtext.
      @see description().
    */
    void setDescription( const QString &description, bool isRich );

    /**
      Sets the incidence description and tries to guess if the description
      is rich text.

      @param description is the incidence description string.
      @see description().
      @since 4.1
    */
    void setDescription( const QString &description );

    /**
      Returns the incidence description.
      @see setDescription().
      @see richDescription().
    */
    QString description() const;

    /**
      Returns the incidence description in rich text format.
      @see setDescription().
      @see description().
      @since 4.1
    */
    QString richDescription() const;

    /**
      Returns true if incidence description contains RichText; false otherwise.
      @see setDescription(), description().
    */
    bool descriptionIsRich() const;

    /**
      Sets the incidence summary.

      @param summary is the incidence summary string.
      @param isRich if true indicates the summary string contains richtext.
      @see summary().
    */
    void setSummary( const QString &summary, bool isRich );

    /**
      Sets the incidence summary and tries to guess if the summary is richtext.

      @param summary is the incidence summary string.
      @see summary().
      @since 4.1
    */
    void setSummary( const QString &summary );

    /**
      Returns the incidence summary.
      @see setSummary().
      @see richSummary().
    */
    QString summary() const;

    /**
      Returns the incidence summary in rich text format.
      @see setSummary().
      @see summary().
      @since 4.1
    */
    QString richSummary() const;

    /**
      Returns true if incidence summary contains RichText; false otherwise.
      @see setSummary(), summary().
    */
    bool summaryIsRich() const;

    /**
      Sets the incidence location. Do _not_ use with journals.

      @param location is the incidence location string.
      @param isRich if true indicates the location string contains richtext.
      @see location().
    */
    void setLocation( const QString &location, bool isRich );

    /**
      Sets the incidence location and tries to guess if the location is
      richtext. Do _not_ use with journals.

      @param location is the incidence location string.
      @see location().
      @since 4.1
    */
    void setLocation( const QString &location );

    /**
      Returns the incidence location. Do _not_ use with journals.
      @see setLocation().
      @see richLocation().
    */
    QString location() const;

    /**
      Returns the incidence location in rich text format.
      @see setLocation().
      @see location().
      @since 4.1
    */
    QString richLocation() const;

    /**
      Returns true if incidence location contains RichText; false otherwise.
      @see setLocation(), location().
    */
    bool locationIsRich() const;

    /**
      Sets the incidence category list.

      @param categories is a list of category strings.
      @see setCategories( const QString &), categories().
    */
    void setCategories( const QStringList &categories );

    /**
      Sets the incidence category list based on a comma delimited string.

      @param catStr is a QString containing a list of categories which
      are delimited by a comma character.
      @see setCategories( const QStringList &), categories().
    */
    void setCategories( const QString &catStr );

    /**
      Returns the incidence categories as a list of strings.
      @see setCategories( const QStringList &), setCategories( Const QString &).
    */
    QStringList categories() const;

    /**
      Returns the incidence categories as a comma separated string.
      @see categories().
    */
    QString categoriesStr() const;

    /**
      Relates another incidence to this one, by UID. This function should only
      be used when constructing a calendar before the related incidence exists.

      @param uid is a QString containing a UID for another incidence.
      @see relatedToUid(), setRelatedTo().
    */
    void setRelatedToUid( const QString &uid );

    /**
      Returns a UID string for the incidence that is related to this one.
      This function should only be used when constructing a calendar before
      the related incidence exists.
      @see setRelatedToUid(), relatedTo().
    */
    QString relatedToUid() const;

    /**
      Relates another incidence to this one. This function should only be
      used when constructing a calendar before the related incidence exists.

      @param incidence is a pointer to another incidence object.
      @see relatedTo(), setRelatedToUid().
    */
    void setRelatedTo( Incidence *incidence );

    /**
      Returns a pointer for the incidence that is related to this one.
      This function should only be used when constructing a calendar before
      the related incidence exists.
      @see setRelatedTo(), relatedToUid().
    */
    Incidence *relatedTo() const;

    /**
      Returns a list of all incidences related to this one.
      @see addRelation, removeRelation().
    */
    Incidence::List relations() const;

    /**
      Adds an incidence that is related to this one.

      @param incidence is a pointer to an Incidence object.
      @see removeRelation(), relations().
    */
    void addRelation( Incidence *incidence );

    /**
      Removes an incidence that is related to this one.

      @param incidence is a pointer to an Incidence object.
      @see addRelation(), relations().
    */
    void removeRelation( Incidence *incidence );

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%  Recurrence-related methods
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    /**
      Returns the recurrence rule associated with this incidence. If there is
      none, returns an appropriate (non-0) object.
    */
    Recurrence *recurrence() const;

    /**
      Removes all recurrence and exception rules and dates.
    */
    void clearRecurrence();

    /**
      @copydoc
      Recurrence::recurs()
    */
    bool recurs() const;

    /**
      @copydoc
      Recurrence::recurrenceType()
    */
    ushort recurrenceType() const;

    /**
      @copydoc
      Recurrence::recursOn()
    */
    virtual bool recursOn( const QDate &date, const KDateTime::Spec &timeSpec ) const;

    /**
      @copydoc
      Recurrence::recursAt()
    */
    bool recursAt( const KDateTime &dt ) const;

    /**
      Calculates the start date/time for all recurrences that happen at some
      time on the given date (might start before that date, but end on or
      after the given date).

      @param date the date when the incidence should occur
      @param timeSpec time specification for @p date.
      @return the start date/time of all occurrences that overlap with the
      given date; an empty list if the incidence does not overlap with the
      date at all.
    */
    virtual QList<KDateTime> startDateTimesForDate(
      const QDate &date,
      const KDateTime::Spec &timeSpec = KDateTime::LocalZone ) const;

    /**
      Calculates the start date/time for all recurrences that happen at the
      given time.

      @param datetime the date/time when the incidence should occur.
      @return the start date/time of all occurrences that overlap with the
      given date/time; an empty list if the incidence does not happen at the
      given time at all.
    */
    virtual QList<KDateTime> startDateTimesForDateTime(
      const KDateTime &datetime ) const;

    /**
      Returns the end date/time of the incidence occurrence if it starts at
      specified date/time.

      @param startDt is the specified starting date/time.
      @return the corresponding end date/time for the occurrence; or the start
      date/time if the end date/time is invalid; or the end date/time if
      the start date/time is invalid.
    */
    virtual KDateTime endDateForStart( const KDateTime &startDt ) const;

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%  Attachment-related methods
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    /**
      Adds an attachment to the incidence.

      @param attachment is a pointer to a valid Attachment object.
      @see deleteAttachment().
    */
    void addAttachment( Attachment *attachment );

    /**
      Removes the specified attachment from the incidence.  Additionally,
      the memory used by the attachment is freed.

      @param attachment is a pointer to a valid Attachment object.
      @see addAttachment(), deleteAttachments().
    */
    void deleteAttachment( Attachment *attachment );

    /**
      Removes all attachments of the specified MIME type from the incidence.
      The memory used by all the removed attachments is freed.

      @param mime is a QString containing the MIME type.
      @see deleteAttachment().
    */
    void deleteAttachments( const QString &mime );

    /**
      Returns a list of all incidence attachments.
      @see attachments( const QString &).
    */
    Attachment::List attachments() const;

    /**
      Returns a list of all incidence attachments with the specified MIME type.

      @param mime is a QString containing the MIME type.
      @see attachments().
    */
    Attachment::List attachments( const QString &mime ) const;

    /**
      Removes all attachments and frees the memory used by them.
      @see deleteAttachment( Attachment *), deleteAttachments( const QString &).
    */
    void clearAttachments();

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%  Secrecy and Status methods
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    /**
      Sets the incidence #Secrecy.

      @param secrecy is the incidence #Secrecy to set.
      @see secrecy(), secrecyStr().
    */
    void setSecrecy( Secrecy secrecy );

    /**
      Returns the incidence #Secrecy.
      @see setSecrecy(), secrecyStr().
    */
    Secrecy secrecy() const;

    /**
      Returns the incidence #Secrecy as translated string.
      @see secrecy().
    */
    QString secrecyStr() const;

    /**
      Returns a list of all available #Secrecy types as a list of translated
      strings.
      @see secrecyName().
    */
    static QStringList secrecyList();

    /**
      Returns the translated string form of a specified #Secrecy.

      @param secrecy is a #Secrecy type.
      @see secrecyList().
    */
    static QString secrecyName( Secrecy secrecy );

    /**
      Sets the incidence status to a standard #Status value.
      Note that StatusX cannot be specified.

      @param status is the incidence #Status to set.
      @see status(), setCustomStatus().
    */
    void setStatus( Status status );

    /**
      Sets the incidence #Status to a non-standard status value.

      @param status is a non-standard status string. If empty,
      the incidence #Status will be set to StatusNone.
      @see setStatus(), status().
    */
    void setCustomStatus( const QString &status );

    /**
      Returns the incidence #Status.
      @see setStatus(), setCustomStatus(), statusStr().
    */
    Status status() const;

    /**
      Returns the incidence #Status as translated string.
      @see status().
    */
    QString statusStr() const;

    /**
      Returns the translated string form of a specified #Status.

      @param status is a #Status type.
    */
    static QString statusName( Status status );

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%  Other methods
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    /**
      Sets a list of incidence resources. (Note: resources in this context
      means items used by the incidence such as money, fuel, hours, etc).

      @param resources is a list of resource strings.
      @see resources().
    */
    void setResources( const QStringList &resources );

    /**
      Returns the incidence resources as a list of strings.
      @see setResources().
    */
    QStringList resources() const;

    /**
      Sets the incidences priority. The priority must be an integer value
      between 0 and 9, where 0 is undefined, 1 is the highest, and 9 is the
      lowest priority (decreasing order).

      @param priority is the incidence priority to set.
      @see priority().
    */
    void setPriority( int priority );

    /**
      Returns the incidence priority.
      @see setPriority().
    */
    int priority() const;

    /**
      Returns true if the incidence has geo data, otherwise return false.
      @see setHasGeo(), setGeoLatitude(float), setGeoLongitude(float).
      @since 4.3
    */
    bool hasGeo() const;

    /**
      Sets if the incidence has geo data.
      @param hasGeo true if incidence has geo data, otherwise false
      @see hasGeo(), geoLatitude(), geoLongitude().
      @since 4.3
    */
    void setHasGeo( bool hasGeo );

    /**
      Set the incidences geoLatitude.
      @param geolatitude is the incidence geolatitude to set
      @see geoLatitude().
      @since 4.3
    */
    void setGeoLatitude( float geolatitude );

    /**
      Returns the incidence geoLatidude.
      @return incidences geolatitude value
      @see setGeoLatitude().
      @since 4.3
    */
    float &geoLatitude() const;

    /**
      Set the incidencesgeoLongitude.
      @param geolongitude is the incidence geolongitude to set
      @see geoLongitude().
      @since 4.3
    */
    void setGeoLongitude( float geolongitude );

    /**
      Returns the incidence geoLongitude.
      @return incidences geolongitude value
      @see setGeoLongitude().
      @since 4.3
    */
    float &geoLongitude() const;

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%  Alarm-related methods
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    /**
      Returns a list of all incidence alarms.
    */
    const Alarm::List &alarms() const;

    /**
      Create a new incidence alarm.
    */
    Alarm *newAlarm();

    /**
      Adds an alarm to the incidence.

      @param alarm is a pointer to a valid Alarm object.
      @see removeAlarm().
    */
    void addAlarm( Alarm *alarm );

    /**
      Removes the specified alarm from the incidence.

      @param alarm is a pointer to a valid Alarm object.
      @see addAlarm().
    */
    void removeAlarm( Alarm *alarm );

    /**
      Removes all alarms.
      @see removeAlarm().
    */
    void clearAlarms();

    /**
      Returns true if any of the incidence alarms are enabled; false otherwise.
    */
    bool isAlarmEnabled() const;

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%  Other methods
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    /**
      Set the incidence scheduling ID. Do _not_ use with journals.
      This is used for accepted invitations as the place to store the UID
      of the invitation. It is later used again if updates to the
      invitation comes in.
      If we did not set a new UID on incidences from invitations, we can
      end up with more than one resource having events with the same UID,
      if you have access to other peoples resources.

      @param sid is a QString containing the scheduling ID.
      @see schedulingID().
    */
    void setSchedulingID( const QString &sid );

    /**
      Returns the incidence scheduling ID. Do _not_ use with journals.
      If a scheduling ID is not set, then return the incidence UID.
      @see setSchedulingID().
    */
    QString schedulingID() const;

    /**
      Observer interface for the recurrence class. If the recurrence is
      changed, this method will be called for the incidence the recurrence
      object belongs to.

      @param recurrence is a pointer to a valid Recurrence object.
    */
    virtual void recurrenceUpdated( Recurrence *recurrence );

    /**
      Assignment operator.

      @warning Not polymorphic. Use AssignmentVisitor for correct
      assignment of an instance of type IncidenceBase to another
      instance of type Incidence.

      @param other is the Incidence to assign.

      @see AssignmentVisitor
     */
    Incidence &operator=( const Incidence &other ); // KDE5: make protected to
                                                    // prevent accidental usage

    /**
      Compares this with Incidence @p ib for equality.

      @warning Not polymorphic. Use ComparisonVisitor for correct
      comparison of two instances of type Incidence.

      @param incidence is the Incidence to compare.

      @see ComparisonVisitor
    */
    bool operator==( const Incidence &incidence ) const; // KDE5: make protected to
                                                         // prevent accidental usage

  protected:
    /**
      Returns the end date/time of the base incidence (e.g. due date/time for
      to-dos, end date/time for events).
      This method must be reimplemented by derived classes.
    */
    virtual KDateTime endDateRecurrenceBase() const
    {
      return dtStart();
    }

  private:
    void init( const Incidence &other );
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
