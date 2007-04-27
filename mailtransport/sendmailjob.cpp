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

#include "sendmailjob.h"
#include "transport.h"

#include <klocale.h>
#include <k3process.h>

#include <qbuffer.h>

using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class SendMailJobPrivate
{
  public:
    K3Process* process;
    QString lastError;
};

SendmailJob::SendmailJob(Transport * transport, QObject * parent) :
    TransportJob( transport, parent ), d( new SendMailJobPrivate )
{
  d->process = new K3Process( this );
  connect( d->process, SIGNAL(processExited(K3Process*)), SLOT(sendmailExited()) );
  connect( d->process, SIGNAL(wroteStdin(K3Process*)), SLOT(wroteStdin()) );
  connect( d->process, SIGNAL(receivedStderr(K3Process*,char*,int)),
           SLOT(receivedStdErr(K3Process*,char*,int)) );
}

SendmailJob::~ SendmailJob()
{
  delete d;
}

void SendmailJob::doStart()
{
  *d->process << transport()->host() << "-i"
          << "-f" << sender() << to() << cc() << bcc();
  if ( !d->process->start( K3Process::NotifyOnExit, K3Process::All ) ) {
    setError( UserDefinedError );
    setErrorText( i18n("Failed to execute mailer program %1",
                  transport()->host()) );
    emitResult();
  }
  setTotalAmount( KJob::Bytes, data().length() );
  wroteStdin();
}

void SendmailJob::sendmailExited()
{
  if ( !d->process->normalExit() || !d->process->exitStatus() == 0 ) {
    setError( UserDefinedError );
    setErrorText( i18n("Sendmail exited abnormally: %1", d->lastError) );
  }
  emitResult();
}

void SendmailJob::wroteStdin()
{
  setProcessedAmount( KJob::Bytes, buffer()->pos() );
  if ( buffer()->atEnd() ) {
    d->process->closeStdin();
  } else {
    QByteArray data = buffer()->read( 1024 );
    d->process->writeStdin( data.constData(), data.length() );
  }
}

void SendmailJob::receivedStdErr(K3Process * proc, char * data, int len)
{
  Q_ASSERT( proc == d->process );
  d->lastError += QString::fromLocal8Bit( data, len );
}

bool SendmailJob::doKill()
{
  delete d->process;
  d->process = 0;
  return true;
}

#include "sendmailjob.moc"
