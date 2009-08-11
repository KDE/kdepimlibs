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

#include "outboxactions.h"

#include "dispatchmodeattribute.h"
#include "errorattribute.h"

#include <akonadi/itemmodifyjob.h>

using namespace Akonadi;
using namespace MailTransport;

ItemFetchScope SendQueuedAction::fetchScope() const
{
  ItemFetchScope scope;
  scope.fetchFullPayload( false );
  scope.fetchAttribute<DispatchModeAttribute>();
  return scope;
}

bool SendQueuedAction::itemAccepted( const Item &item ) const
{
  if( !item.hasAttribute<DispatchModeAttribute>() ) {
    kWarning() << "Item doesn't have DispatchModeAttribute.";
    return false;
  }

  return item.attribute<DispatchModeAttribute>()->dispatchMode() == DispatchModeAttribute::Never;
}

Job *SendQueuedAction::itemAction( const Item &item ) const
{
  Item cp = item;
  cp.addAttribute( new DispatchModeAttribute ); // defaults to Immediately
  return new ItemModifyJob( cp );
}



ItemFetchScope ClearErrorAction::fetchScope() const
{
  ItemFetchScope scope;
  scope.fetchFullPayload( false );
  scope.fetchAttribute<ErrorAttribute>();
  return scope;
}

bool ClearErrorAction::itemAccepted( const Item &item ) const
{
  return item.hasAttribute<ErrorAttribute>();
}

Job *ClearErrorAction::itemAction( const Item &item ) const
{
  Item cp = item;
  cp.removeAttribute<ErrorAttribute>();
  cp.clearFlag( "error" );
  cp.setFlag( "queued" );
  return new ItemModifyJob( cp );
}

