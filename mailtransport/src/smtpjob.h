/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>

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

#ifndef MAILTRANSPORT_SMTPJOB_H
#define MAILTRANSPORT_SMTPJOB_H

#include <transportjob.h>

namespace KIO
{
class Job;
class Slave;
}

class SmtpJobPrivate;

namespace MailTransport
{

class SmtpSession;

/**
  Mail transport job for SMTP.
  Internally, all jobs for a specific transport are queued to use the same
  KIO::Slave. This avoids multiple simultaneous connections to the server,
  which is not always allowed. Also, re-using an already existing connection
  avoids the login overhead and can improve performance.

  Precommands are automatically executed, once per opening a connection to the
  server (not necessarily once per message).

  @deprecated Use MessageQueueJob for sending e-mail.
*/
class MAILTRANSPORT_DEPRECATED_EXPORT SmtpJob : public TransportJob
{
    Q_OBJECT
public:
    /**
      Creates a SmtpJob.
      @param transport The transport settings.
      @param parent The parent object.
    */
    explicit SmtpJob(Transport *transport, QObject *parent = Q_NULLPTR);

    /**
      Deletes this job.
    */
    virtual ~SmtpJob();

protected:
    virtual void doStart();
    virtual bool doKill();

protected Q_SLOTS:
    virtual void slotResult(KJob *job);
    void slaveError(KIO::Slave *slave, int errorCode, const QString &errorMsg);

private:
    void startSmtpJob();

private Q_SLOTS:
    void dataRequest(KIO::Job *job, QByteArray &data);

private:
    friend class ::SmtpJobPrivate;
    SmtpJobPrivate *const d;
    Q_PRIVATE_SLOT(d, void smtpSessionResult(MailTransport::SmtpSession *))
};

} // namespace MailTransport

#endif // MAILTRANSPORT_SMTPJOB_H
