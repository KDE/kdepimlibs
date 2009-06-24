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

#include "sendqueued.h"

#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KJob>

#include <akonadi/collection.h>
#include <akonadi/control.h>
#include <akonadi/filteractionjob.h>

#include <outboxinterface/localfolders.h>
#include <outboxinterface/outboxactions.h>

using namespace Akonadi;
using namespace OutboxInterface;


Runner::Runner()
{
  Control::start();

  connect( LocalFolders::self(), SIGNAL( foldersReady() ),
      this, SLOT( checkFolders() ) );
  LocalFolders::self()->fetch();
}

void Runner::checkFolders()
{
  Collection outbox = LocalFolders::self()->outbox();
  kDebug() << "Got outbox" << outbox.id();

  if( !outbox.isValid() ) {
    KApplication::exit( 1 );
  }

  FilterActionJob *fjob = new FilterActionJob( outbox, new SendQueuedAction, this );
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
  KCmdLineArgs::init( argc, argv, "sendqueued", 0,
                      ki18n( "sendqueued" ), "0",
                      ki18n( "An app that sends all queued messages" ) );
  KApplication app;
  new Runner();
  return app.exec();
}


#include "sendqueued.moc"
