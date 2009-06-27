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

#ifndef MAILTRANSPORT_AKONADIJOB_H
#define MAILTRANSPORT_AKONADIJOB_H

#include <mailtransport/transportjob.h>

#include <akonadi/item.h>

class AkonadiJobPrivate;

namespace MailTransport {

/**
  Mail transport job for an Akonadi-based transport.
  Unlike SmtpJob and SendmailJob, you also need to call setItemId() before
  starting this job.  In other words, the item you want to send has to be
  created in advance.

  TODO API:
  * Should we provide setItem or let the job assemble the item out of data()?
  * What to do about from, to, cc, bcc? Should there be some kind of standard
    attribute containing these?  Currently data(), from(), to() etc. are
    completely ignored.
*/
class MAILTRANSPORT_EXPORT AkonadiJob : public TransportJob
{
  Q_OBJECT
  public:
    /**
      Creates an AkonadiJob.
      @param transport The transport object to use.
      @param parent The parent object.
    */
    explicit AkonadiJob( Transport *transport, QObject *parent = 0 );

    /**
      Destroys this job.
    */
    virtual ~AkonadiJob();

    /**
      The id of the item to send.
    */
    Akonadi::Item::Id itemId() const;

    /**
      Set the id of the item to send.
      @param itemId id of the item to send
    */
    void setItemId( Akonadi::Item::Id id );

  protected:
    /** reimpl */
    virtual void doStart();

  private Q_SLOTS:
    void resourceResult( qlonglong itemId, bool success, const QString &message );

  private:
    AkonadiJobPrivate *const d;

};

} // namespace MailTransport

#endif // MAILTRANSPORT_AKONADIJOB_H
