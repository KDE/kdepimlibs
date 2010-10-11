/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_TEST_UTILS_H
#define AKONADI_TEST_UTILS_H

#include "collectionpathresolver_p.h"
#include "dbusconnectionpool.h"
#include "servermanager.h"
#include "qtest_akonadi.h"

#include <QDBusInterface>
#include <QDBusReply>

qint64 collectionIdFromPath( const QString &path )
{
  Akonadi::CollectionPathResolver *resolver = new Akonadi::CollectionPathResolver( path );
  bool success = resolver->exec();
  if ( !success ) {
    qDebug() << "path resolution for " << path << " failed: " << resolver->errorText();
    return -1;
  }
  qint64 id = resolver->collection();
  return id;
}

bool restartAkonadiServer()
{
    QDBusInterface testrunnerIface( QLatin1String( "org.kde.Akonadi.Testrunner" ),
                                    QLatin1String( "/" ),
                                    QLatin1String( "org.kde.Akonadi.Testrunner" ),
                                    Akonadi::DBusConnectionPool::threadConnection() );
    if ( !testrunnerIface.isValid() )
        kWarning() << "Unable to get a dbus interface to the testrunner!";

    QDBusReply<void> reply = testrunnerIface.call( "restartAkonadiServer" );
    if ( !reply.isValid() ) {
        kWarning() << reply.error();
        return false;
    } else if ( Akonadi::ServerManager::isRunning() ) {
        return true;
    } else {
        return QTest::kWaitForSignal( Akonadi::ServerManager::self(), SIGNAL(started()), 10000 );
    }
}


bool trackAkonadiProcess( bool track )
{
    QDBusInterface testrunnerIface( QLatin1String( "org.kde.Akonadi.Testrunner" ),
                                    QLatin1String( "/" ),
                                    QLatin1String( "org.kde.Akonadi.Testrunner" ),
                                    Akonadi::DBusConnectionPool::threadConnection() );
    if ( !testrunnerIface.isValid() )
        kWarning() << "Unable to get a dbus interface to the testrunner!";

    QDBusReply<void> reply = testrunnerIface.call( "trackAkonadiProcess", track );
    if ( !reply.isValid() ) {
        kWarning() << reply.error();
        return false;
    } else {
        return true;
    }
}

#endif
