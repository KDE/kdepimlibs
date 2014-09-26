/*
 * Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef AKONADI_MESSAGEFLAGS_H
#define AKONADI_MESSAGEFLAGS_H

#include "akonadi-mime_export.h"

namespace Akonadi
{
/**
 * @short Contains predefined message flag identifiers.
 *
 * This namespace contains identifiers of message flags that
 *  are used internally in the Akonadi server.
 */
namespace MessageFlags
{
/**
 * The flag for a message being seen (i.e. opened by user).
 */
AKONADI_MIME_EXPORT extern const char *const Seen;

/**
 * The flag for a message being deleted by the user.
 */
AKONADI_MIME_EXPORT extern const char *const Deleted;

/**
 * The flag for a message being replied to by the user.
 * @deprecated use Replied instead.
 */
AKONADI_MIME_EXPORT extern const char *const Answered;

/**
 * The flag for a message being marked as flagged.
 */
AKONADI_MIME_EXPORT extern const char *const Flagged;

/**
 * The flag for a message being marked with an error.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const HasError;

/**
 * The flag for a message being marked as having an attachment.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const HasAttachment;

/**
 * The flag for a message being marked as having an invitation.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const HasInvitation;

/**
 * The flag for a message being marked as sent.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Sent;

/**
 * The flag for a message being marked as queued.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Queued;

/**
 * The flag for a message being marked as replied.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Replied;

/**
 * The flag for a message being marked as forwarded.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Forwarded;

/**
 * The flag for a message being marked as action item to act on.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const ToAct;

/**
 * The flag for a message being marked as watched.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Watched;

/**
 * The flag for a message being marked as ignored.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Ignored;

/**
 * The flag for a message being marked as signed.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Signed;

/**
 * The flag for a message being marked as encrypted.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Encrypted;

/**
 * The flag for a message being marked as spam.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Spam;

/**
 * The flag for a message being marked as ham.
 * @since 4.6
 */
AKONADI_MIME_EXPORT extern const char *const Ham;
}
}

#endif
