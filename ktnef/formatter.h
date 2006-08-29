/*
    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KTNEF_FORMATTER_H
#define KTNEF_FORMATTER_H

#include <QString>

#include "ktnef.h"

namespace KCal {
class Calendar;
class InvitationFormatterHelper;
}

namespace KTnef {

/**
  A helper class containing static methods to format TNEF attachments into diffrent formats
  like eg. a HTML representation.
*/
class KTNEF_EXPORT Formatter
{
  public:
    /// Format a TNEF attachment to an HTML mail
    static QString formatTNEFInvitation( const QByteArray& tnef,
                                         KCal::Calendar *mCalendar,
                                         KCal::InvitationFormatterHelper *helper );
    /// Transform a TNEF attachment to an iCal or vCard
    static QString msTNEFToVPart( const QByteArray& tnef );
};

}

#endif
