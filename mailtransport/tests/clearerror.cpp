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

#include "clearerror.h"

#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KJob>

#include <akonadi/collection.h>
#include <akonadi/control.h>
#include <akonadi/filteractionjob.h>
#include <akonadi/kmime/localfolders.h>
#include <akonadi/kmime/localfoldersrequestjob.h>

#include <mailtransport/outboxactions.h>

using namespace Akonadi;
using namespace MailTransport;


Runner::Runner()
{
  Control::start();

  LocalFoldersRequestJob *rjob = new LocalFoldersRequestJob( this );
  rjob->requestDefaultFolder( LocalFolders::Outbox );
  connect( rjob, SIGNAL(result(KJob*)), this, SLOT(checkFolders()) );
  rjob->start();
}

void Runner::checkFolders()
{
  Collection outbox = LocalFolders::self()->defaultFolder( LocalFolders::Outbox );
  kDebug() << "Got outbox" << outbox.id();

  if( !outbox.isValid() ) {
    kError() << "Failed to get outbox folder.";
    KApplication::exit( 1 );
  }

  FilterActionJob *fjob = new FilterActionJob( outbox, new ClearErrorAction, this );
  connect( fjob, SIGNAL(result(KJob*)), this, SLOT(jobResult(KJob*)) );
}

void Runner::jobResult( KJob *job )
{
  if( job->error() ) {
    kDebug() << "Job error:" << job->errorString();
    KApplication::exit( 2 );
  } else {
    kDebug() << "Job success.";
    KApplication::exit( 0 );
  }
}

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "clearerror", 0,
                      ki18n( "clearerror" ), "0",
                      ki18n( "An app that re-queues failed items from the outbox" ) );
  KApplication app;
  new Runner();
  return app.exec();
}


#include "clearerror.moc"
