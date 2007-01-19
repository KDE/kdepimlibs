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
/**
  @file
  This file is part of the API for handling TNEF data and provides the
  Formatter helper class.

  @author Cornelius Schumacher
  @author Reinhold Kainhofer
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
  @brief
  A helper class containing static methods to format @acronym TNEF attachments
  into different formats like eg. a HTML representation.
*/
class KTNEF_EXPORT Formatter
{
  public:
    /**
      Format a @acronym TNEF attachment to an HTML mail.

      @param tnef is the QByteArray contain the @acronym TNEF data.
      @param cal is a pointer to a Calendar object.
      @param h is a pointer to a InvitationFormatterHelp object.
    */
    static QString formatTNEFInvitation( const QByteArray &tnef,
                                         KCal::Calendar *cal,
                                         KCal::InvitationFormatterHelper *h );

    /**
      Transform a @acronym TNEF attachment to an iCal or vCard.

      @param tnef is the QByteArray containing the @acronym TNEF data.

      @return a string containing the transformed attachment.
    */
    static QString msTNEFToVPart( const QByteArray &tnef );
};

}

#endif
