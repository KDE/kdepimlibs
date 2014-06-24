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

#include <KLocalizedString>

#include <QProcess>

using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class PreCommandJobPrivate
{
  public:
    PreCommandJobPrivate( PrecommandJob *parent );
    QProcess *process;
    QString precommand;
    PrecommandJob *q;

    // Slots
    void slotFinished( int, QProcess::ExitStatus );
    void slotStarted();
    void slotError( QProcess::ProcessError error );
};

PreCommandJobPrivate::PreCommandJobPrivate( PrecommandJob *parent )
  : process( 0 ), q( parent )
{
}

PrecommandJob::PrecommandJob( const QString &precommand, QObject *parent )
  : KJob( parent ), d( new PreCommandJobPrivate( this ) )
{
  d->precommand = precommand;
  d->process = new QProcess( this );
  connect( d->process, SIGNAL(started()), SLOT(slotStarted()) );
  connect( d->process, SIGNAL(error(QProcess::ProcessError)),
           SLOT(slotError(QProcess::ProcessError)) );
  connect( d->process, SIGNAL(finished(int,QProcess::ExitStatus)),
           SLOT(slotFinished(int,QProcess::ExitStatus)) );
}

PrecommandJob::~ PrecommandJob()
{
  delete d;
}

void PrecommandJob::start()
{
  d->process->start( d->precommand );
}

void PreCommandJobPrivate::slotStarted()
{
  emit q->infoMessage( q, i18n( "Executing precommand" ),
                       i18n( "Executing precommand '%1'.", precommand ) );
}

void PreCommandJobPrivate::slotError( QProcess::ProcessError error )
{
  q->setError( KJob::UserDefinedError );
  if ( error == QProcess::FailedToStart ) {
    q->setErrorText( i18n( "Unable to start precommand '%1'.", precommand ) );
  } else {
    q->setErrorText( i18n( "Error while executing precommand '%1'.", precommand ) );
  }
  q->emitResult();
}

bool PrecommandJob::doKill()
{
  delete d->process;
  d->process = 0;
  return true;
}

void PreCommandJobPrivate::slotFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
  if ( exitStatus == QProcess::CrashExit ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "The precommand crashed." ) );
  } else if ( exitCode != 0 ) {
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "The precommand exited with code %1.",
                           process->exitStatus() ) );
  }
  q->emitResult();
}

#include "moc_precommandjob.cpp"
