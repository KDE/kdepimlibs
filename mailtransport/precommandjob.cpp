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

#include "precommandjob.h"

#include <klocale.h>
#include <KProcess>

using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class PreCommandJobPrivate
{
  public:
    KProcess *process;
    QString precommand;
};

PrecommandJob::PrecommandJob(const QString & precommand, QObject * parent) :
    KJob( parent ), d( new PreCommandJobPrivate )
{
  d->precommand = precommand;
  d->process = new KProcess( this );
  connect( d->process, SIGNAL(started()), SLOT(slotStarted()) );
  connect( d->process, SIGNAL(error(QProcess::ProcessError error)),
           SLOT(slotError(QProcess::ProcessError error)));
  connect( d->process, SIGNAL(finished(int, QProcess::ExitStatus)),
           SLOT(slotFinished(int, QProcess::ExitStatus)) );
}

PrecommandJob::~ PrecommandJob()
{
  delete d;
}

void PrecommandJob::start()
{
  d->process->setShellCommand( d->precommand );
  d->process->start();
}

void PrecommandJob::slotStarted()
{
  emit infoMessage( this, i18n("Executing precommand"),
                    i18n("Executing precommand '%1'.", d->precommand ) );
}

void PrecommandJob::slotError( QProcess::ProcessError error)
{
  setError( UserDefinedError );
  setErrorText( i18n("Could not execute precommand '%1'.", d->precommand ) );
  kDebug(5324) << "Execution precommand has failed:" << error;
  emitResult();
}

bool PrecommandJob::doKill()
{
  delete d->process;
  d->process = 0;
  return true;
}

void PrecommandJob::slotFinished(int exitCode,
                                 QProcess::ExitStatus exitStatus )
{
  if ( exitStatus == QProcess::CrashExit ) {
    setError( UserDefinedError );
    setErrorText( i18n("The precommand crashed." ));
  } else if ( exitCode != 0 ) {
    setError( UserDefinedError );
    setErrorText( i18n("The precommand exited with code %1.",
                  d->process->exitStatus()) );
  }
  emitResult();
}

#include "precommandjob.moc"
