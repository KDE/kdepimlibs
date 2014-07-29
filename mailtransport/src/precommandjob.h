/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    Based on KMail code by:
    Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
    Copyright (c) 2000-2002 Michael Haeckel <haeckel@kde.org>

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

#ifndef MAILTRANSPORT_PRECOMMANDJOB_H
#define MAILTRANSPORT_PRECOMMANDJOB_H

#include "mailtransport_export.h"

#include <KJob>

class PreCommandJobPrivate;

namespace MailTransport
{

/**
  Job to execute a command.
  This is used often for sending or receiving mails, for example to set up
  a tunnel of VPN connection.
  Basically this is just a KJob wrapper around a QProcess.

  @since 4.4
 */
class MAILTRANSPORT_EXPORT PrecommandJob : public KJob
{
    Q_OBJECT

public:
    /**
      Creates a new precommand job.
      @param precommand The command to run.
      @param parent The parent object.
    */
    explicit PrecommandJob(const QString &precommand, QObject *parent = 0);

    /**
      Destroys this job.
    */
    virtual ~PrecommandJob();

    /**
      Executes the precommand.
      Reimplemented from KJob.
    */
    virtual void start();

protected:

    /**
      Reimplemented from KJob.
    */
    virtual bool doKill();

private:
    friend class ::PreCommandJobPrivate;
    PreCommandJobPrivate *const d;
    Q_PRIVATE_SLOT(d, void slotFinished(int, QProcess::ExitStatus))
    Q_PRIVATE_SLOT(d, void slotStarted())
    Q_PRIVATE_SLOT(d, void slotError(QProcess::ProcessError error))
};

} // namespace MailTransport

#endif // MAILTRANSPORT_PRECOMMANDJOB_H
