/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AKONADI_IDLEJOB_H
#define AKONADI_IDLEJOB_H

#include "job.h"

#include <akonadi/private/idle_p.h>
#include <akonadi/item.h>
#include <akonadi/collection.h>

#include <QtCore/QSharedDataPointer>

namespace Akonadi
{

class IdleNotification
{
  public:
    IdleNotification();
    IdleNotification( const IdleNotification &other );
    ~IdleNotification();
    IdleNotification &operator=( const IdleNotification &other );

    bool isValid() const;

    Idle::Type type() const;
    void setType( Idle::Type type );
    Idle::Operation operation() const;
    void setOperation( Idle::Operation operation );
    QSet<QByteArray> changedParts() const;
    void setChangedParts( const QSet<QByteArray> &parts );
    Entity::Id destinationCollection() const;
    void setDestinationCollection( Entity::Id id );
    Entity::Id sourceCollection() const;
    void setSourceCollection( Entity::Id id );
    QByteArray resource() const;
    void setResource( const QByteArray &resource );
    QByteArray destinationResource() const;
    void setDestinationResource( const QByteArray &destinationResource );
    QSet<QByteArray> addedFlags() const;
    void setAddedFlags( const QSet<QByteArray> &addedFlags );
    QSet<QByteArray> removedFlags() const;
    void setRemovedFlags( const QSet<QByteArray> &removedFlags );
    QList<Item> items() const;
    void setItems( const QList<Item> &items );
    void addItem( const Item &item );

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

class IdleJobPrivate;

class IdleJob : public Akonadi::Job
{
    Q_OBJECT

  public:
    explicit IdleJob( Akonadi::Session *session );
    virtual ~IdleJob();

    void addMonitoredCollection( const Collection &collection );
    void removeMonitoredCollection( const Collection &collection );
    void addMonitoredItem( Entity::Id id );
    void removeMonitoredItem( Entity::Id id );
    void addMonitoredMimeType( const QString &mimeType );
    void removeMonitoredMimeType( const QString &mimeType );
    void addIgnoredSession( const QByteArray &sessionId );
    void removeIgnoredSession( const QByteArray &sessionId );
    void addMonitoredResource( const QByteArray &resource );
    void removeMonitoredResource( const QByteArray &resource );
    void setAllMonitored( bool monitored );

    void sendData( const QByteArray &data );

  Q_SIGNALS:
    void notify( const IdleNotification &notification );

  protected:
    virtual void doStart();
    virtual void doHandleResponse( const QByteArray &tag, const QByteArray &data );

  private:
    Q_DECLARE_PRIVATE( IdleJob )

    Q_PRIVATE_SLOT( d_func(), void _k_updateFilter() );
};

}

#endif // AKONADI_IDLEJOB_H
