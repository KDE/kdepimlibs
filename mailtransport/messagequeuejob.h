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

#ifndef MAILTRANSPORT_MESSAGEQUEUEJOB_H
#define MAILTRANSPORT_MESSAGEQUEUEJOB_H

#include <mailtransport/mailtransport_export.h>

#include "dispatchmodeattribute.h"
#include "sentbehaviourattribute.h"
#include "transportattribute.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <KDE/KCompositeJob>

#include <akonadi/collection.h>
#include <akonadi/kmime/addressattribute.h>

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

namespace MailTransport {

/**
  @short Provides an interface for sending email.

  This class takes a KMime::Message and some related info such as sender and
  recipient addresses, and places the message in the outbox.  The mail
  dispatcher agent will then take it from there and send it.

  This is the preferred way for applications to send email.

  This job requires some options to be set before being started.  Modify the
  attributes of this job to change these options.
 
  You need to set the transport of the transport attribute, the from address of
  the address attribute and one of the to, cc or bcc addresses of the address
  attribute. Also, you need to call setMessage().
  Optionally, you can change the dispatch mode attribute or the sent behaviour
  attribute. 

  Example:
  @code

  MessageQueueJob *job = new MessageQueueJob( this );
  job->setMessage( msg ); // msg is a Message::Ptr
  job->transportAttribute().setTransportId( TransportManager::self()->defaultTransportId() );
  // Use the default dispatch mode.
  // Use the default sent-behaviour.
  job->addressAttribute().setFrom( from ); // from is a QString
  job->addressAttribute().setTo( to ); // to is a QStringList
  connect( job, SIGNAL(result(KJob*)), this, SLOT(jobResult(KJob*)) );
  job->start();

  @endcode

  @see DispatchModeAttribute
  @see SentBehaviourAttribute
  @see TransportAttribute
  @see AddressAttribute

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_EXPORT MessageQueueJob : public KCompositeJob
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
      Returns a reference to the dispatch mode attribue for this message.
      Modify the returned attribute to change the dispatch mode.
    */
    DispatchModeAttribute& dispatchModeAttribute();

    /**
      Returns a reference to the address attribue for this message.
      Modify the returned attribute to change the receivers or the from
      address.
    */
    Akonadi::AddressAttribute& addressAttribute();

    /**
      Returns a reference to the transport attribue for this message.
      Modify the returned attribute to change the transport used for
      sending the mail.
    */
    TransportAttribute& transportAttribute();

    /**
      Returns a reference to the sent behaviour attribue for this message.
      Modify the returned attribute to change the sent behaviour.
    */
    SentBehaviourAttribute& sentBehaviourAttribute();

    /**
      Sets the message to be sent.
    */
    void setMessage( KMime::Message::Ptr message );

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

    Q_PRIVATE_SLOT( d, void outboxRequestResult( KJob* ) )

};

} // namespace MailTransport

#endif // MAILTRANSPORT_MESSAGEQUEUEJOB_H
