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
#include <QProcess>

using namespace MailTransport;

PrecommandJob::PrecommandJob(const QString & precommand, QObject * parent) :
    KJob( parent ),
    mProcess( 0 ),
    mPrecommand( precommand )
{
  mProcess = new QProcess( this );
  connect( mProcess, SIGNAL(started()), SLOT(slotStarted()) );
  connect( mProcess, SIGNAL(error(QProcess::ProcessError error)),
           SLOT(slotError(QProcess::ProcessError error)));
  connect( mProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
           SLOT(slotFinished(int, QProcess::ExitStatus)) );
}

PrecommandJob::~ PrecommandJob()
{
  delete mProcess;
}

void PrecommandJob::start()
{
  mProcess->start( mPrecommand );
}

void PrecommandJob::slotStarted()
{
  emit infoMessage( this, i18n("Executing precommand"),
                    i18n("Executing precommand '%1'.", mPrecommand ) );
}

void PrecommandJob::slotEror( QProcess::ProcessError error)
{
  setError( UserDefinedError );
  setErrorText( i18n("Could not execute precommand '%1'.", mPrecommand ) );
  kDebug(5324) << "Execution precommand has failed: " << error << endl;
  emitResult();
}

bool PrecommandJob::doKill()
{
  delete mProcess;
  mProcess = 0;
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
                  mProcess->exitStatus()) );
  }
  emitResult();
}

#include "precommandjob.moc"
