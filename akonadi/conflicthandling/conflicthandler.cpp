/*
    Copyright (c) 2010 KDAB
    Author: Tobias Koenig <tokoe@kde.org>

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

#include "conflicthandler_p.h"

#include "conflictresolvedialog_p.h"

#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/session.h>
#include <klocale.h>

using namespace Akonadi;

ConflictHandler::ConflictHandler( ConflictType type, QObject *parent )
  : QObject( parent ),
    mConflictType( type ),
    mSession( new Session( "conflict handling session", this ) )
{
}

void ConflictHandler::setConflictingItems( const Akonadi::Item &changedItem, const Akonadi::Item &conflictingItem )
{
  mChangedItem = changedItem;
  mConflictingItem = conflictingItem;
}

void ConflictHandler::start()
{
  if ( mConflictType == LocalLocalConflict || mConflictType == LocalRemoteConflict ) {
    ItemFetchJob *job = new ItemFetchJob( mConflictingItem, mSession );
    job->fetchScope().fetchFullPayload();
    job->fetchScope().setAncestorRetrieval( ItemFetchScope::Parent );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotOtherItemFetched(KJob*)) );
  } else {
    resolve();
  }
}

void ConflictHandler::slotOtherItemFetched( KJob *job )
{
  if ( job->error() ) {
    emit error( job->errorText() ); //TODO: extend error message
    return;
  }

  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( fetchJob->items().isEmpty() ) {
    emit error( i18n( "Did not find other item for conflict handling" ) );
    return;
  }

  mConflictingItem = fetchJob->items().first();
  QMetaObject::invokeMethod( this, "resolve", Qt::QueuedConnection );
}

void ConflictHandler::resolve()
{
  ConflictResolveDialog dlg;
  dlg.setConflictingItems( mChangedItem, mConflictingItem );
  dlg.exec();

  const ResolveStrategy strategy = dlg.resolveStrategy();
  switch ( strategy ) {
    case UseLocalItem:
      useLocalItem();
      break;
    case UseOtherItem:
      useOtherItem();
      break;
    case UseBothItems:
      useBothItems();
      break;
  }
}

void ConflictHandler::useLocalItem()
{
  // We have to overwrite the other item inside the Akonadi storage with the local
  // item. To make this happen, we have to set the revision of the local item to
  // the one of the other item to let the Akonadi server accept it.

  Item newItem( mChangedItem );
  newItem.setRevision( mConflictingItem.revision() );

  ItemModifyJob *job = new ItemModifyJob( newItem, mSession );
  connect( job, SIGNAL(result(KJob*)), SLOT(slotUseLocalItemFinished(KJob*)) );
}

void ConflictHandler::slotUseLocalItemFinished( KJob *job )
{
  if ( job->error() ) {
    emit error( job->errorText() ); //TODO: extend error message
  } else {
    emit conflictResolved();
  }
}

void ConflictHandler::useOtherItem()
{
  // We can just ignore the local item here and leave everything as it is.
  emit conflictResolved();
}

void ConflictHandler::useBothItems()
{
  // We have to create a new item for the local item under the collection that has
  // been retrieved when we fetched the other item.
  ItemCreateJob *job = new ItemCreateJob( mChangedItem, mConflictingItem.parentCollection(), mSession );
  connect( job, SIGNAL(result(KJob*)), SLOT(slotUseBothItemsFinished(KJob*)) );
}

void ConflictHandler::slotUseBothItemsFinished( KJob *job )
{
  if ( job->error() ) {
    emit error( job->errorText() ); //TODO: extend error message
  } else {
    emit conflictResolved();
  }
}

#include "moc_conflicthandler_p.cpp"
