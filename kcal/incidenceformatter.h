/*
  This file is part of the kcal library.

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
#ifndef KCAL_INCIDENCEFORMATTER_H
#define KCAL_INCIDENCEFORMATTER_H

#include "kcal_export.h"
#include <QtCore/QString>

namespace KCal {
class Calendar;
class Incidence;
class IncidenceBase;

class KCAL_EXPORT InvitationFormatterHelper
{
  public:
    InvitationFormatterHelper() : d( 0 ) {}
    virtual ~InvitationFormatterHelper(){}
    virtual QString generateLinkURL( const QString &id );
    virtual QString makeLink( const QString &id, const QString &text );
    virtual Calendar* calendar() const { return 0; }

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( InvitationFormatterHelper )
    class Private;
    Private *const d;
    //@endcond
};

/**
  Helpers that provides several static methods to format an Incidence into
  different formats, like an HTML representation for KMail, a representation
  for tool tips, or a representation for the event viewer.

  @short methods to format incidences into various formats for displaying them
*/
namespace IncidenceFormatter
{
  KCAL_EXPORT QString toolTipString( IncidenceBase *incidence, bool richText = true );
  KCAL_EXPORT QString mailBodyString( IncidenceBase *incidencebase );
  KCAL_EXPORT QString extensiveDisplayString( IncidenceBase *incidence );
  KCAL_EXPORT QString formatICalInvitation( QString invitation, Calendar *mCalendar,
                                            InvitationFormatterHelper *helper );

  /**
    Format a TNEF attachment to an HTML mail

    @since 4.1
  */
  KCAL_EXPORT QString formatTNEFInvitation( const QByteArray &tnef, Calendar *mCalendar,
                                            InvitationFormatterHelper *helper );
  /**
    Transform a TNEF attachment to an iCal or vCard

    @since 4.1
  */
  KCAL_EXPORT QString msTNEFToVPart( const QByteArray &tnef );

  /**
    @since 4.1
  */
  KCAL_EXPORT QString recurrenceString( Incidence *incidence );

  class EventViewerVisitor;
  class ScheduleMessageVisitor;
  class InvitationHeaderVisitor;
  class InvitationBodyVisitor;
  class IncidenceCompareVisitor;
  class ToolTipVisitor;
  class MailBodyVisitor;
}

}

#endif
