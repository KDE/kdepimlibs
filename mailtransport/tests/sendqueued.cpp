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

#include <QApplication>
#include <KLocale>
#include <KJob>

#include <collection.h>
#include <control.h>
#include <filteractionjob_p.h>
#include <dispatcherinterface.h>
#include <specialmailcollections.h>
#include <specialmailcollectionsrequestjob.h>

using namespace Akonadi;
using namespace MailTransport;

Runner::Runner()
{
  Control::start();

  SpecialMailCollectionsRequestJob *rjob = new SpecialMailCollectionsRequestJob( this );
  rjob->requestDefaultCollection( SpecialMailCollections::Outbox );
  connect( rjob, SIGNAL(result(KJob*)), this, SLOT(checkFolders()) );
  rjob->start();
}

void Runner::checkFolders()
{
  DispatcherInterface().dispatchManually();
}

int main( int argc, char **argv )
{
  QApplication app(argc, argv);
  app.setApplicationName(QLatin1String("sendqueued"));

  new Runner();
  return app.exec();
}

