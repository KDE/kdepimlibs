/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Kevin Krammer <kevin.krammer@gmx.at>

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

#ifndef AKONADI_AGENTBASE_P_H
#define AKONADI_AGENTBASE_P_H

#include "agentbase.h"
#include "tracerinterface.h"

#include <klocale.h>

#include <solid/networking.h>

class QSettings;

namespace Akonadi {

/**
 * @internal
 */
class AgentBasePrivate : public QObject
{
  Q_OBJECT
  public:
    AgentBasePrivate( AgentBase *parent );
    virtual ~AgentBasePrivate();
    void init();
    virtual void delayedInit();

    void slotStatus( int status, const QString &message );
    void slotPercent( int progress );
    void slotWarning( const QString& message );
    void slotError( const QString& message );
    void slotNetworkStatusChange( Solid::Networking::Status );
    void slotResumedFromSuspend();

    virtual void changeProcessed();

    QString defaultReadyMessage() const
    {
      if ( mOnline ) {
        return i18nc( "@info:status Application ready for work", "Ready" );
      }
      return i18nc( "@info:status", "Offline" );
    }

    QString defaultSyncingMessage() const
    {
      return i18nc( "@info:status", "Syncing..." );
    }

    QString defaultErrorMessage() const
    {
      return i18nc( "@info:status", "Error." );
    }

    void setProgramName();

    AgentBase *q_ptr;
    Q_DECLARE_PUBLIC( AgentBase )

    QString mId;
    QString mName;
    QString mResourceTypeName;

    /// Use sessionBus() to access the connection.
    QDBusConnection mDBusConnection;

    int mStatusCode;
    QString mStatusMessage;

    uint mProgress;
    QString mProgressMessage;

    bool mNeedsNetwork;
    bool mOnline;
    bool mDesiredOnlineState;

    QSettings *mSettings;

    ChangeRecorder *mChangeRecorder;

    org::freedesktop::Akonadi::Tracer *mTracer;

    AgentBase::Observer *mObserver;

  public Q_SLOTS:
    virtual void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection );
    virtual void itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers );
    virtual void itemMoved( const Akonadi::Item &, const Akonadi::Collection &source, const Akonadi::Collection &destination );
    virtual void itemRemoved( const Akonadi::Item &item );
    void itemLinked( const Akonadi::Item &item, const Akonadi::Collection &collection );
    void itemUnlinked( const Akonadi::Item &item, const Akonadi::Collection &collection );

    virtual void collectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection &parent );
    virtual void collectionChanged( const Akonadi::Collection &collection );
    virtual void collectionChanged( const Akonadi::Collection &collection, const QSet<QByteArray> &changedAttributes );
    virtual void collectionMoved( const Akonadi::Collection &collection, const Akonadi::Collection &source, const Akonadi::Collection &destination );
    virtual void collectionRemoved( const Akonadi::Collection &collection );
    void collectionSubscribed( const Akonadi::Collection &collection, const Akonadi::Collection &parent );
    void collectionUnsubscribed( const Akonadi::Collection &collection );
};

}

#endif
