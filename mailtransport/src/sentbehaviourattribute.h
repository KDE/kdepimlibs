/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MAILTRANSPORT_SENTBEHAVIOURATTRIBUTE_H
#define MAILTRANSPORT_SENTBEHAVIOURATTRIBUTE_H

#include <mailtransport_export.h>

#include <attribute.h>
#include <collection.h>

namespace MailTransport
{

/**
  Attribute determining what will happen to a message after it is sent.  The
  message can be deleted from the Outbox, moved to the default sent-mail
  collection, or moved to a custom collection.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_EXPORT SentBehaviourAttribute : public Akonadi::Attribute
{
public:
    /**
      What to do with the item in the outbox after it has been sent successfully.
    */
    enum SentBehaviour {
        Delete,                      ///< Delete the item from the outbox.
        MoveToCollection,            ///< Move the item to a custom collection.
        MoveToDefaultSentCollection  ///< Move the item to the default sent-mail collection.
    };

    /**
      Creates a new SentBehaviourAttribute.
    */
    explicit SentBehaviourAttribute(SentBehaviour beh = MoveToDefaultSentCollection,
                                    Akonadi::Collection moveToCollection = Akonadi::Collection(-1));

    /**
      Destroys the SentBehaviourAttribute.
    */
    virtual ~SentBehaviourAttribute();

    /* reimpl */
    SentBehaviourAttribute *clone() const Q_DECL_OVERRIDE;
    QByteArray type() const Q_DECL_OVERRIDE;
    QByteArray serialized() const Q_DECL_OVERRIDE;
    void deserialize(const QByteArray &data) Q_DECL_OVERRIDE;

    /**
      Returns the sent-behaviour of the message.
      @see SentBehaviour.
    */
    SentBehaviour sentBehaviour() const;

    /**
      Sets the sent-behaviour of the message.
      @param beh the sent-behaviour to set
      @see SentBehaviour.
    */
    void setSentBehaviour(SentBehaviour beh);

    /**
      Returns the collection to which the item should be moved after it is sent.
      Only valid if sentBehaviour() is MoveToCollection.
    */
    Akonadi::Collection moveToCollection() const;

    /**
      Sets the collection to which the item should be moved after it is sent.
      Make sure you set the SentBehaviour to MoveToCollection first.
      @param moveToCollection target collection for "move to" operation
      @see setSentBehaviour.
    */
    void setMoveToCollection(Akonadi::Collection moveToCollection);

private:
    class Private;
    Private *const d;

};

} // namespace MailTransport

#endif // MAILTRANSPORT_SENTBEHAVIOURATTRIBUTE_H
