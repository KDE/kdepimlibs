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

#ifndef MAILTRANSPORT_OUTBOXACTIONS_P_H
#define MAILTRANSPORT_OUTBOXACTIONS_P_H

#include <mailtransport_export.h>
#include <filteractionjob_p.h>
#include <transportattribute.h>

#include <itemfetchscope.h>
#include <job.h>

namespace MailTransport
{

/**
  FilterAction that finds all messages with a DispatchMode of Manual
  and assigns them a DispatchMode of Immediately.

  This is used to send "queued" messages on demand.

  @see FilterActionJob

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class SendQueuedAction : public Akonadi::FilterAction
{
public:
    /** Creates a SendQueuedAction. */
    SendQueuedAction();

    /** Destroys this object. */
    virtual ~SendQueuedAction();

    /* reimpl */
    virtual Akonadi::ItemFetchScope fetchScope() const;

    /* reimpl */
    virtual bool itemAccepted(const Akonadi::Item &item) const;

    /* reimpl */
    virtual Akonadi::Job *itemAction(const Akonadi::Item &item,
                                     Akonadi::FilterActionJob *parent) const;

private:
    class Private;
    Private *const d;
};

/**
  FilterAction that finds all messages with an ErrorAttribute,
  removes the attribute, and sets the "$QUEUED" flag.

  This is used to retry sending messages that failed.

  @see FilterActionJob

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class ClearErrorAction : public Akonadi::FilterAction
{
public:
    /** Creates a ClearErrorAction. */
    ClearErrorAction();

    /** Destroys this object. */
    virtual ~ClearErrorAction();

    /* reimpl */
    virtual Akonadi::ItemFetchScope fetchScope() const;

    /* reimpl */
    virtual bool itemAccepted(const Akonadi::Item &item) const;

    /* reimpl */
    virtual Akonadi::Job *itemAction(const Akonadi::Item &item,
                                     Akonadi::FilterActionJob *parent) const;

private:
    class Private;
    Private *const d;
};

/**
  FilterAction that changes the transport for all messages and
  sets the "$QUEUED" flag.

  This is used to send queued messages using an alternative transport.

  @see FilterActionJob

  @author Torgny Nyblom <kde nyblom org>
  @since 4.5
*/
class DispatchManualTransportAction : public Akonadi::FilterAction
{
public:
    DispatchManualTransportAction(int transportId);

    virtual ~DispatchManualTransportAction();

    /* reimpl */
    virtual Akonadi::ItemFetchScope fetchScope() const;

    /* reimpl */
    virtual bool itemAccepted(const Akonadi::Item &item) const;

    /* reimpl */
    virtual Akonadi::Job *itemAction(const Akonadi::Item &item,
                                     Akonadi::FilterActionJob *parent) const;

private:
    class Private;
    Private *const d;

    int mTransportId;
};

} // namespace MailTransport

#endif // MAILTRANSPORT_OUTBOXACTIONS_P_H
