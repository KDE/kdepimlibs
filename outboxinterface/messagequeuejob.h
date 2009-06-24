/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef OUTBOXINTERFACE_MESSAGEQUEUEJOB_H
#define OUTBOXINTERFACE_MESSAGEQUEUEJOB_H

#include <outboxinterface/outboxinterface_export.h>

#include "dispatchmodeattribute.h"
#include "sentbehaviourattribute.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <KDE/KCompositeJob>

#include <akonadi/collection.h>

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

namespace OutboxInterface {

/**
  @short Provides an interface for sending email.

  This class takes a KMime::Message and some related info such as sender and
  recipient addresses, and places the message in the outbox.  The mail
  dispatcher agent will then take it from there and send it.

  This is the preferred way for applications to send email.

  This job requires some options to be set before being started.  These are
  setMessage, setTransportId, setFrom, and one of setTo, setCc, or setBcc.
  Other settings are optional: setDispatchMode, setSentBehaviour.

  Example:
  @code

  MessageQueueJob *job = new MessageQueueJob( this );
  job->setMessage( msg ); // msg is a Message::Ptr
  job->setTransportId( TransportManager::self()->defaultTransportId() );
  // Use the default dispatch mode.
  // Use the default sent-behaviour.
  job->setFrom( from ); // from is a QString
  job->setTo( to ); // to is a QStringList
  connect( job, SIGNAL(result(KJob*)), this, SLOT(jobResult(KJob*)) );
  job->start();

  @endcode

  @see DispatchModeAttribute
  @see SentBehaviourAttribute

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class OUTBOXINTERFACE_EXPORT MessageQueueJob : public KCompositeJob
{
  Q_OBJECT

  public:
    /**
      Creates a new MessageQueueJob.
      This is not an autostarting job; you need to call start() yourself.
    */
    explicit MessageQueueJob( QObject *parent = 0 );
    
    /**
      Destroys the MessageQueueJob.
      This job deletes itself after finishing.
    */
    virtual ~MessageQueueJob();

    /**
      Returns the message to be sent.
    */
    KMime::Message::Ptr message() const;

    /**
      Returns the transport id to use for sending the message.
      @see TransportManager.
    */
    int transportId() const;

    /**
      Returns the dispatch mode for this message.
      @see DispatchModeAttribute.
    */
    DispatchModeAttribute::DispatchMode dispatchMode() const;

    /**
      Returns the date and time when this message should be sent.
      Only valid if dispatchMode() is AfterDueDate.
      @see DispatchModeAttribute.
    */
    QDateTime sendDueDate() const;

    /**
      Returns the sent-behaviour of this message.
      This determines what will happen to the message after it is sent.
      @see SentBehaviourAttribute.
    */
    SentBehaviourAttribute::SentBehaviour sentBehaviour() const;

    /**
      Returns the collection to which the message will be moved after it is
      sent.
      Only valid if sentBehaviour() is MoveToCollection.
      @see SentBehaviourAttribute.
    */
    Akonadi::Collection::Id moveToCollection() const;

    /**
      Returns the address of the sender.
    */
    QString from() const;

    /**
      Returns the addresses of the "To:" receivers.
    */
    QStringList to() const;

    /**
      Returns the addresses of the "Cc:" receivers.
    */
    QStringList cc() const;

    /**
      Returns the addresses of the "Bcc:" receivers.
    */
    QStringList bcc() const;

    /**
      Sets the message to be sent.
    */
    void setMessage( KMime::Message::Ptr message );

    /**
      Sets the transport id to use for sending the message.  If you want to
      use the default transport, you must specify so explicitly:

      @code
      job->setTransportId( TransportManager::self()->defaultTransportId() );
      @endcode

      @see TransportManager.
    */
    void setTransportId( int id );

    /**
      Sets the dispatch mode for this message.
      The default dispatch mode is Immediately (meaning the message will be
      sent as soon as possible).
      @see DispatchModeAttribute.
    */
    void setDispatchMode( DispatchModeAttribute::DispatchMode mode );

    /**
      Sets the date and time when this message should be sent.

      @code
      job->setDispatchMode( DispatchModeAttribute::AfterDueDate );
      job->setDueDate( ... );
      @endcode

      @see DispatchModeAttribute.
    */
    void setDueDate( const QDateTime &date );

    /**
      Sets the sent-behaviour of this message.
      This determines what will happen to the message after it is sent.
      The default sent-behaviour is MoveToDefaultSentCollection, which moves
      the message to the default sent-mail collection.
      @see SentBehaviourAttribute.
    */
    void setSentBehaviour( SentBehaviourAttribute::SentBehaviour beh );

    /**
      Sets the collection to which the message will be moved after it is
      sent.

      @code
      job->setSentBehaviour( SentBehaviourAttribute::MoveToCollection );
      job->setMoveToCollection( ... );
      @endcode

      @see SentBehaviourAttribute.
    */
    void setMoveToCollection( Akonadi::Collection::Id cid );

    /**
      Sets the address of the sender.
    */
    void setFrom( const QString &from );

    /**
      Sets the addresses of the "To:" receivers."
    */
    void setTo( const QStringList &to );

    /**
      Sets the addresses of the "Cc:" receivers."
    */
    void setCc( const QStringList &cc );

    /**
      Sets the addresses of the "Bcc:" receivers."
    */
    void setBcc( const QStringList &bcc );

    /**
      Creates the item and places it in the outbox.
      It is now queued for sending by the mail dispatcher agent.
    */
    virtual void start();

  protected Q_SLOTS:
    /**
      Called when the ItemCreateJob subjob finishes.

      (reimplemented from KCompositeJob)
    */
    virtual void slotResult( KJob * );

  private:
    class Private;
    friend class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void doStart() )

};

} // namespace OutboxInterface

#endif // OUTBOXINTERFACE_MESSAGEQUEUEJOB_H
