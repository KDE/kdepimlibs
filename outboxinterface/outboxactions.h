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

#ifndef OUTBOXINTERFACE_OUTBOXACTIONS_H
#define OUTBOXINTERFACE_OUTBOXACTIONS_H

#include <outboxinterface/outboxinterface_export.h>

#include <akonadi/itemfetchscope.h>
#include <akonadi/job.h>
#include <akonadi/filteractionjob.h>

namespace OutboxInterface
{

/**
  FilterActionJob functor that finds all messages with a DispatchMode of Never
  and assigns them a DispatchMode of Immediately.

  This is used to send "queued" messages on demand.
*/
class OUTBOXINTERFACE_EXPORT SendQueuedAction : public Akonadi::FilterAction
{
  public:
    virtual Akonadi::ItemFetchScope fetchScope() const;
    virtual bool itemAccepted( const Akonadi::Item &item ) const;
    virtual Akonadi::Job *itemAction( const Akonadi::Item &item ) const;
};


/**
  FilterActionJob functor that finds all messages with an ErrorAttribute,
  removes the attribute, and sets the "queued" flag.

  This is used to send failed messages again.
*/
class OUTBOXINTERFACE_EXPORT ClearErrorAction : public Akonadi::FilterAction
{
  public:
    virtual Akonadi::ItemFetchScope fetchScope() const;
    virtual bool itemAccepted( const Akonadi::Item &item ) const;
    virtual Akonadi::Job *itemAction( const Akonadi::Item &item ) const;
};

}

#endif
