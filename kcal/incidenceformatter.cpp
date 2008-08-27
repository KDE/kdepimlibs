/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "incidenceformatter.h"
#include "attachment.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "calendar.h"
#include "calendarlocal.h"
#include "icalformat.h"
#include "freebusy.h"
#include "calendarresources.h"

#include "kpimutils/email.h"
#include "kabc/phonenumber.h"
#include "kabc/vcardconverter.h"
#include "kabc/stdaddressbook.h"

#include <kdatetime.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include <QtCore/QBuffer>
#include <QtCore/QList>
#include <QtGui/QTextDocument>
#include <QtGui/QApplication>

#include <time.h>

using namespace KCal;

/*******************************************************************
 *  Helper functions for the extensive display (event viewer)
 *******************************************************************/

static QString eventViewerAddLink( const QString &ref, const QString &text,
                                   bool newline = true )
{
  QString tmpStr( "<a href=\"" + ref + "\">" + text + "</a>" );
  if ( newline ) {
    tmpStr += '\n';
  }
  return tmpStr;
}

static QString eventViewerAddTag( const QString &tag, const QString &text )
{
  int numLineBreaks = text.count( "\n" );
  QString str = '<' + tag + '>';
  QString tmpText = text;
  QString tmpStr = str;
  if( numLineBreaks >= 0 ) {
    if ( numLineBreaks > 0 ) {
      int pos = 0;
      QString tmp;
      for ( int i = 0; i <= numLineBreaks; i++ ) {
        pos = tmpText.indexOf( "\n" );
        tmp = tmpText.left( pos );
        tmpText = tmpText.right( tmpText.length() - pos - 1 );
        tmpStr += tmp + "<br>";
      }
    } else {
      tmpStr += tmpText;
    }
  }
  tmpStr += "</" + tag + '>';
  return tmpStr;
}

static QString eventViewerFormatCategories( Incidence *event )
{
  QString tmpStr;
  if ( !event->categoriesStr().isEmpty() ) {
    if ( event->categories().count() == 1 ) {
      tmpStr = eventViewerAddTag( "h3", i18n( "Category" ) );
    } else {
      tmpStr = eventViewerAddTag( "h3", i18n( "Categories" ) );
    }
    tmpStr += eventViewerAddTag( "p", event->categoriesStr() );
  }
  return tmpStr;
}

static QString linkPerson( const QString &email, QString name, QString uid,
                           const QString &iconPath )
{
  // Make the search, if there is an email address to search on,
  // and either name or uid is missing
  if ( !email.isEmpty() && ( name.isEmpty() || uid.isEmpty() ) ) {
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList = add_book->findByEmail( email );
    KABC::Addressee o = ( !addressList.isEmpty() ? addressList.first() : KABC::Addressee() );
    if ( !o.isEmpty() && addressList.size() < 2 ) {
      if ( name.isEmpty() ) {
        // No name set, so use the one from the addressbook
        name = o.formattedName();
      }
      uid = o.uid();
    } else {
      // Email not found in the addressbook. Don't make a link
      uid.clear();
    }
  }
  kDebug() << "formatAttendees: uid =" << uid;

  // Show the attendee
  QString tmpString = "<li>";
  if ( !uid.isEmpty() ) {
    // There is a UID, so make a link to the addressbook
    if ( name.isEmpty() ) {
      // Use the email address for text
      tmpString += eventViewerAddLink( "uid:" + uid, email );
    } else {
      tmpString += eventViewerAddLink( "uid:" + uid, name );
    }
  } else {
    // No UID, just show some text
    tmpString += ( name.isEmpty() ? email : name );
  }
  tmpString += '\n';

  // Make the mailto link
  if ( !email.isEmpty() && !iconPath.isNull() ) {
    KCal::Person person( name, email );
    KUrl mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( person.fullName() );
    tmpString += eventViewerAddLink( mailto.url(), "<img src=\"" + iconPath + "\">" );
  }
  tmpString += "</li>\n";

  return tmpString;
}

static QString eventViewerFormatAttendees( Incidence *event )
{
  QString tmpStr;
  Attendee::List attendees = event->attendees();
  if ( attendees.count() ) {
    KIconLoader *iconLoader = KIconLoader::global();
    const QString iconPath = iconLoader->iconPath( "mail-message-new", KIconLoader::Small );

    // Add organizer link
    tmpStr += eventViewerAddTag( "h4", i18n( "Organizer" ) );
    tmpStr += "<ul>";
    tmpStr += linkPerson( event->organizer().email(), event->organizer().name(),
                          QString(), iconPath );
    tmpStr += "</ul>";

    // Add attendees links
    tmpStr += eventViewerAddTag( "h4", i18n( "Attendees" ) );
    tmpStr += "<ul>";
    Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      tmpStr += linkPerson( a->email(), a->name(), a->uid(), iconPath );
      if ( !a->delegator().isEmpty() ) {
        tmpStr += i18n( " (delegated by %1)", a->delegator() );
      }
      if ( !a->delegate().isEmpty() ) {
        tmpStr += i18n( " (delegated to %1)", a->delegate() );
      }
    }
    tmpStr += "</ul>";
  }
  return tmpStr;
}

static QString eventViewerFormatAttachments( Incidence *i )
{
  QString tmpStr;
  Attachment::List as = i->attachments();
  if ( as.count() > 0 ) {
    Attachment::List::ConstIterator it;
    for ( it = as.begin(); it != as.end(); ++it ) {
      if ( (*it)->isUri() ) {
        tmpStr += eventViewerAddLink( (*it)->uri(), (*it)->label() );
        tmpStr += "<br>";
      }
    }
  }
  return tmpStr;
}

/*
  FIXME:This function depends of kaddressbook. Is necessary a new
  type of event?
*/
static QString eventViewerFormatBirthday( Event *event )
{
  if ( !event ) {
    return QString();
  }
  if ( event->customProperty( "KABC", "BIRTHDAY" ) != "YES" ) {
    return QString();
  }

  QString uid_1 = event->customProperty( "KABC", "UID-1" );
  QString name_1 = event->customProperty( "KABC", "NAME-1" );
  QString email_1= event->customProperty( "KABC", "EMAIL-1" );

  KIconLoader *iconLoader = KIconLoader::global();
  const QString iconPath = iconLoader->iconPath( "mail-message-new", KIconLoader::Small );
  //TODO: add a tart icon
  QString tmpString = "<ul>";
  tmpString += linkPerson( email_1, name_1, uid_1, iconPath );

  if ( event->customProperty( "KABC", "ANNIVERSARY" ) == "YES" ) {
    QString uid_2 = event->customProperty( "KABC", "UID-2" );
    QString name_2 = event->customProperty( "KABC", "NAME-2" );
    QString email_2= event->customProperty( "KABC", "EMAIL-2" );
    tmpString += linkPerson( email_2, name_2, uid_2, iconPath );
  }

  tmpString += "</ul>";
  return tmpString;
}

static QString eventViewerFormatHeader( Incidence *incidence )
{
  QString tmpStr = "<table><tr>";

  // show icons
  /* all of those icons currently don't exist, makes no sense currently
     to load a set of "unknown" icons. re-enable when those are available.
  {
    KIconLoader *iconLoader = KIconLoader::global();
    tmpStr += "<td>";

    if ( incidence->type() == "Todo" ) {
      tmpStr += "<img src=\"" + iconLoader->iconPath( "todo", KIconLoader::Small ) + "\">";
    }
    if ( incidence->isAlarmEnabled() ) {
      tmpStr += "<img src=\"" + iconLoader->iconPath( "bell", KIconLoader::Small ) + "\">";
    }
    if ( incidence->recurs() ) {
      tmpStr += "<img src=\"" + iconLoader->iconPath( "recur", KIconLoader::Small ) + "\">";
    }
    if ( incidence->isReadOnly() ) {
      tmpStr += "<img src=\"" + iconLoader->iconPath( "readonlyevent", KIconLoader::Small ) + "\">";
    }

    tmpStr += "</td>";
  }
  */

  tmpStr += "<td>" + eventViewerAddTag( "h2", incidence->richSummary() ) + "</td>";
  tmpStr += "</tr></table><br>";

  return tmpStr;
}

static QString eventViewerFormatEvent( Event *event )
{
  if ( !event ) {
    return QString();
  }

  QString tmpStr = eventViewerFormatHeader( event );

  tmpStr += "<table>";
  if ( !event->location().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18n( "Location" ) + "</b></td>";
    tmpStr += "<td>" + event->richLocation() + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr>";
  if ( event->allDay() ) {
    if ( event->isMultiDay() ) {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      tmpStr += "<td>" + i18nc("<beginTime> - <endTime>","%1 - %2",
                      event->dtStartDateStr( true, event->dtStart().timeSpec() ),
                      event->dtEndDateStr( true, event->dtEnd().timeSpec() ) ) + "</td>";
    } else {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Date" ) + "</b></td>";
      tmpStr += "<td>" +
                i18nc( "date as string","%1",
                       event->dtStartDateStr( true, event->dtStart().timeSpec() ) ) + "</td>";
    }
  } else {
    if ( event->isMultiDay() ) {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      tmpStr += "<td>" +
                i18nc( "<beginTime> - <endTime>","%1 - %2",
                       event->dtStartStr( true, event->dtStart().timeSpec() ),
                       event->dtEndStr( true, event->dtEnd().timeSpec() ) ) + "</td>";
    } else {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      if ( event->hasEndDate() && event->dtStart() != event->dtEnd() ) {
        tmpStr += "<td>" +
                  i18nc( "<beginTime> - <endTime>","%1 - %2",
                         event->dtStartTimeStr( true, event->dtStart().timeSpec() ),
                         event->dtEndTimeStr( true, event->dtEnd().timeSpec() ) ) + "</td>";
      } else {
        tmpStr += "<td>" + event->dtStartTimeStr( true, event->dtStart().timeSpec() ) + "</td>";
      }
      tmpStr += "</tr><tr>";
      tmpStr += "<td align=\"right\"><b>" + i18n( "Date" ) + "</b></td>";
      tmpStr += "<td>" +
                i18nc( "date as string","%1",
                       event->dtStartDateStr( true, event->dtStart().timeSpec() ) ) + "</td>";
    }
  }
  tmpStr += "</tr>";

  if ( event->customProperty( "KABC", "BIRTHDAY" ) == "YES" ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18n( "Birthday" ) + "</b></td>";
    tmpStr += "<td>" + eventViewerFormatBirthday( event ) + "</td>";
    tmpStr += "</tr>";
    tmpStr += "</table>";
    return tmpStr;
  }

  if ( !event->description().isEmpty() ) {
    tmpStr += "<tr>";
    tmpStr += "<td></td>";
    tmpStr += "<td>" + eventViewerAddTag( "p", event->richDescription() ) + "</td>";
    tmpStr += "</tr>";
  }

  if ( event->categories().count() > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>";
    tmpStr += i18np( "1&nbsp;category", "%1&nbsp;categories", event->categories().count() ) +
              "</b></td>";
    tmpStr += "<td>" + event->categoriesStr() + "</td>";
    tmpStr += "</tr>";
  }

  if ( event->recurs() ) {
    KDateTime dt = event->recurrence()->getNextDateTime( KDateTime::currentUtcDateTime() );
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>" + i18n( "Next Occurrence" )+ "</b></td>";
    tmpStr += "<td>" +
              KGlobal::locale()->formatDateTime( dt.dateTime(), KLocale::ShortDate ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "<tr><td colspan=\"2\">";
  tmpStr += eventViewerFormatAttendees( event );
  tmpStr += "</td></tr>";

  int attachmentCount = event->attachments().count();
  if ( attachmentCount > 0 ) {
    tmpStr += "<tr>";
    tmpStr += "<td align=\"right\"><b>";
    tmpStr += i18np( "1&nbsp;attachment", "%1&nbsp;attachments", attachmentCount )+ "</b></td>";
    tmpStr += "<td>" + eventViewerFormatAttachments( event ) + "</td>";
    tmpStr += "</tr>";
  }

  tmpStr += "</table>";
  tmpStr += "<p><em>" +
            i18n( "Creation date: %1", KGlobal::locale()->formatDateTime(
                    event->created().dateTime(), KLocale::ShortDate ) ) + "</em>";
  return tmpStr;
}

static QString eventViewerFormatTodo( Todo *todo )
{
  if ( !todo ) {
    return QString();
  }

  QString tmpStr = eventViewerFormatHeader( todo );

  if ( !todo->location().isEmpty() ) {
    tmpStr += eventViewerAddTag( "b", i18n(" Location: %1", todo->richLocation() ) );
    tmpStr += "<br>";
  }

  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    tmpStr += i18n( "<b>Due on:</b> %1", todo->dtDueStr( true, todo->dtDue().timeSpec() ) );
  }

  if ( !todo->description().isEmpty() ) {
    tmpStr += eventViewerAddTag( "p", todo->richDescription() );
  }

  tmpStr += eventViewerFormatCategories( todo );

  if ( todo->priority() > 0 ) {
    tmpStr += i18n( "<p><b>Priority:</b> %1</p>", todo->priority() );
  } else {
    tmpStr += i18n( "<p><b>Priority:</b> %1</p>", i18n( "Unspecified" ) );
  }

  tmpStr += i18n( "<p><i>%1 % completed</i></p>", todo->percentComplete() );

  if ( todo->recurs() ) {
    KDateTime dt = todo->recurrence()->getNextDateTime( KDateTime::currentUtcDateTime() );
    tmpStr += eventViewerAddTag( "p", "<em>" +
      i18n( "This is a recurring to-do. The next occurrence will be on %1.",
            KGlobal::locale()->formatDateTime( dt.dateTime(), KLocale::ShortDate ) ) + "</em>" );
  }
  tmpStr += eventViewerFormatAttendees( todo );
  tmpStr += eventViewerFormatAttachments( todo );
  tmpStr += "<p><em>" + i18n( "Creation date: %1",
    KGlobal::locale()->formatDateTime( todo->created().dateTime(), KLocale::ShortDate ) ) + "</em>";
  return tmpStr;
}

static QString eventViewerFormatJournal( Journal *journal )
{
  if ( !journal ) {
    return QString();
  }

  QString tmpStr;
  if ( !journal->summary().isEmpty() ) {
    tmpStr+= eventViewerAddTag( "h2", journal->richSummary() );
  }
  tmpStr += eventViewerAddTag(
    "h3", i18n( "Journal for %1",
                journal->dtStartDateStr( false, journal->dtStart().timeSpec() ) ) );
  if ( !journal->description().isEmpty() ) {
    tmpStr += eventViewerAddTag( "p", journal->richDescription() );
  }
  return tmpStr;
}

static QString eventViewerFormatFreeBusy( FreeBusy *fb )
{
  if ( !fb ) {
    return QString();
  }

  QString tmpStr(
    eventViewerAddTag(
      "h2", i18n( "Free/Busy information for %1", fb->organizer().fullName() ) ) );
  tmpStr += eventViewerAddTag(
    "h4", i18n( "Busy times in date range %1 - %2:",
                KGlobal::locale()->formatDate( fb->dtStart().date(), KLocale::ShortDate ),
                KGlobal::locale()->formatDate( fb->dtEnd().date(), KLocale::ShortDate ) ) );

  QList<Period> periods = fb->busyPeriods();

  QString text =
    eventViewerAddTag( "em",
                       eventViewerAddTag( "b", i18nc( "tag for busy periods list", "Busy:" ) ) );

  QList<Period>::iterator it;
  for ( it = periods.begin(); it != periods.end(); ++it ) {
    Period per = *it;
    if ( per.hasDuration() ) {
      int dur = per.duration().asSeconds();
      QString cont;
      if ( dur >= 3600 ) {
        cont += i18ncp( "hours part of duration", "1 hour ", "%1 hours ", dur / 3600 );
        dur %= 3600;
      }
      if ( dur >= 60 ) {
        cont += i18ncp( "minutes part duration", "1 minute ", "%1 minutes ", dur / 60 );
        dur %= 60;
      }
      if ( dur > 0 ) {
        cont += i18ncp( "seconds part of duration", "1 second", "%1 seconds", dur );
      }
      text += i18nc( "startDate for duration", "%1 for %2",
                     KGlobal::locale()->formatDateTime(
                       per.start().dateTime(), KLocale::LongDate ), cont );
      text += "<br>";
    } else {
      if ( per.start().date() == per.end().date() ) {
        text += i18nc( "date, fromTime - toTime ", "%1, %2 - %3",
                       KGlobal::locale()->formatDate( per.start().date() ),
                       KGlobal::locale()->formatTime( per.start().time() ),
                       KGlobal::locale()->formatTime( per.end().time() ) );
      } else {
        text += i18nc( "fromDateTime - toDateTime", "%1 - %2",
                       KGlobal::locale()->formatDateTime(
                         per.start().dateTime(), KLocale::LongDate ),
                       KGlobal::locale()->formatDateTime(
                         per.end().dateTime(), KLocale::LongDate ) );
      }
      text += "<br>";
    }
  }
  tmpStr += eventViewerAddTag( "p", text );
  return tmpStr;
}

//@cond PRIVATE
class KCal::IncidenceFormatter::EventViewerVisitor : public IncidenceBase::Visitor
{
  public:
    EventViewerVisitor() { mResult = ""; }
    bool act( IncidenceBase *incidence ) { return incidence->accept( *this ); }
    QString result() const { return mResult; }
  protected:
    bool visit( Event *event )
    {
      mResult = eventViewerFormatEvent( event );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = eventViewerFormatTodo( todo );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = eventViewerFormatJournal( journal );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = eventViewerFormatFreeBusy( fb );
      return !mResult.isEmpty();
    }

  protected:
    QString mResult;
};
//@endcond

QString IncidenceFormatter::extensiveDisplayString( IncidenceBase *incidence )
{
  if ( !incidence ) {
    return QString();
  }

  EventViewerVisitor v;
  if ( v.act( incidence ) ) {
    return v.result();
  } else {
    return QString();
  }
}

/*******************************************************************
 *  Helper functions for the body part formatter of kmail
 *******************************************************************/

static QString string2HTML( const QString &str )
{
  return Qt::convertFromPlainText( str, Qt::WhiteSpaceNormal );
}

static QString eventStartTimeStr( Event *event )
{
  QString tmp;
  if ( ! event->allDay() ) {
    tmp =  i18nc( "%1: Start Date, %2: Start Time", "%1 %2",
                  event->dtStartDateStr(), event->dtStartTimeStr() );
  } else {
    tmp = i18nc( "%1: Start Date", "%1 (time unspecified)", event->dtStartDateStr() );
  }
  return tmp;
}

static QString eventEndTimeStr( Event *event )
{
  QString tmp;
  if ( event->hasEndDate() && event->dtEnd().isValid() ) {
    if ( ! event->allDay() ) {
      tmp =  i18nc( "%1: End Date, %2: End Time", "%1 %2",
                    event->dtEndDateStr(), event->dtEndTimeStr() );
    } else {
      tmp = i18nc( "%1: End Date", "%1 (time unspecified)", event->dtEndDateStr() );
    }
  } else {
    tmp = i18n( "Unspecified" );
  }
  return tmp;
}

static QString invitationRow( const QString &cell1, const QString &cell2 )
{
  return "<tr><td>" + cell1 + "</td><td>" + cell2 + "</td></tr>\n";
}

static QString invitationsDetailsIncidence( Incidence *incidence )
{
  QString html;
  QString descr;
  if ( !incidence->descriptionIsRich() ) {
    descr = string2HTML( incidence->description() );
  } else {
    descr = eventViewerAddTag( "p", incidence->richDescription() );
  }
  if( !descr.isEmpty() ) {
    html += "<br/><u>" + i18n( "Description:" ) + "</u><table border=\"0\"><tr><td>&nbsp;</td><td>";
    html += descr + "</td></tr></table>";
  }
  QStringList comments = incidence->comments();
  if ( !comments.isEmpty() ) {
    html += "<br><u>" + i18n( "Comments:" ) + "</u><table border=\"0\"><tr><td>&nbsp;</td><td><ul>";
    for ( int i = 0; i < comments.count(); ++i ) {
      html += "<li>" + string2HTML( comments[i] ) + "</li>";
    }
    html += "</ul></td></tr></table>";
  }
  return html;
}

static QString invitationDetailsEvent( Event *event )
{
  // Meeting details are formatted into an HTML table
  if ( !event ) {
    return QString();
  }

  QString html;
  QString tmp;

  QString sSummary = i18n( "Summary unspecified" );
  if ( ! event->summary().isEmpty() ) {
    if ( !event->summaryIsRich() ) {
      sSummary = string2HTML( event->summary() );
    } else {
      sSummary = eventViewerAddTag( "p", event->richSummary() );
    }
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( ! event->location().isEmpty() ) {
    if ( !event->locationIsRich() ) {
      sLocation = string2HTML( event->location() );
    } else {
      sLocation = eventViewerAddTag( "p", event->richLocation() );
    }
  }

  QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );
  html = QString( "<div dir=\"%1\">\n" ).arg( dir );
  html += "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Meeting summary & location rows
  html += invitationRow( i18n( "What:" ), sSummary );
  html += invitationRow( i18n( "Where:" ), sLocation );

  // Meeting Start Time Row
  html += invitationRow( i18n( "Start Time:" ), eventStartTimeStr( event ) );

  // Meeting End Time Row
  html += invitationRow( i18n( "End Time:" ), eventEndTimeStr( event ) );

  // Meeting Duration Row
  if ( !event->allDay() && event->hasEndDate() && event->dtEnd().isValid() ) {
    tmp.clear();
    QTime sDuration( 0, 0, 0 ), t;
    int secs = event->dtStart().secsTo( event->dtEnd() );
    t = sDuration.addSecs( secs );
    if ( t.hour() > 0 ) {
      tmp += i18np( "1 hour ", "%1 hours ", t.hour() );
    }
    if ( t.minute() > 0 ) {
      tmp += i18np( "1 minute ", "%1 minutes ", t.minute() );
    }

    html += invitationRow( i18n( "Duration:" ), tmp );
  }

  html += "</table>\n";
  html += invitationsDetailsIncidence( event );
  html += "</div>\n";

  return html;
}

static QString invitationDetailsTodo( Todo *todo )
{
  // To-do details are formatted into an HTML table
  if ( !todo ) {
    return QString();
  }

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( ! todo->summary().isEmpty() ) {
    sSummary = todo->richSummary();
  }
  if ( ! todo->description().isEmpty() ) {
    sDescr = todo->description();
  }
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Summary:" ), sSummary );
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";
  html += invitationsDetailsIncidence( todo );

  return html;
}

static QString invitationDetailsJournal( Journal *journal )
{
  if ( !journal ) {
    return QString();
  }

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( ! journal->summary().isEmpty() ) {
    sSummary = journal->richSummary();
  }
  if ( ! journal->description().isEmpty() ) {
    sDescr = journal->richDescription();
  }
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Summary:" ), sSummary );
  html += invitationRow( i18n( "Date:" ),
                         journal->dtStartDateStr( false, journal->dtStart().timeSpec() ) );
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";
  html += invitationsDetailsIncidence( journal );

  return html;
}

static QString invitationDetailsFreeBusy( FreeBusy *fb )
{
  if ( !fb ) {
    return QString();
  }

  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Person:" ), fb->organizer().fullName() );
  html += invitationRow( i18n( "Start date:" ),
                         fb->dtStartDateStr( true, fb->dtStart().timeSpec() ) );
  html += invitationRow( i18n( "End date:" ),
                         KGlobal::locale()->formatDate( fb->dtEnd().date(), KLocale::ShortDate ) );
  html += "<tr><td colspan=2><hr></td></tr>\n";
  html += "<tr><td colspan=2>Busy periods given in this free/busy object:</td></tr>\n";

  QList<Period> periods = fb->busyPeriods();
  QList<Period>::iterator it;
  for ( it = periods.begin(); it != periods.end(); ++it ) {
    Period per = *it;
    if ( per.hasDuration() ) {
      int dur = per.duration().asSeconds();
      QString cont;
      if ( dur >= 3600 ) {
        cont += i18ncp( "hours part of duration", "1 hour ", "%1 hours ", dur / 3600 );
        dur %= 3600;
      }
      if ( dur >= 60 ) {
        cont += i18ncp( "minutes part of duration", "1 minute", "%1 minutes ", dur / 60 );
        dur %= 60;
      }
      if ( dur > 0 ) {
        cont += i18ncp( "seconds part of duration", "1 second", "%1 seconds", dur );
      }
      html += invitationRow(
        QString(), i18nc( "startDate for duration", "%1 for %2",
                          KGlobal::locale()->formatDateTime(
                            per.start().dateTime(), KLocale::LongDate ), cont ) );
    } else {
      QString cont;
      if ( per.start().date() == per.end().date() ) {
        cont = i18nc( "date, fromTime - toTime ", "%1, %2 - %3",
                      KGlobal::locale()->formatDate( per.start().date() ),
                      KGlobal::locale()->formatTime( per.start().time() ),
                      KGlobal::locale()->formatTime( per.end().time() ) );
      } else {
        cont = i18nc( "fromDateTime - toDateTime", "%1 - %2",
                      KGlobal::locale()->formatDateTime(
                        per.start().dateTime(), KLocale::LongDate ),
                      KGlobal::locale()->formatDateTime(
                        per.end().dateTime(), KLocale::LongDate ) );
      }

      html += invitationRow( QString(), cont );
    }
  }

  html += "</table>\n";
  return html;
}

static QString invitationHeaderEvent( Event *event, ScheduleMessage *msg )
{
  if ( !msg || !event ) {
    return QString();
  }

  switch ( msg->method() ) {
  case iTIPPublish:
    return i18n( "This event has been published" );
  case iTIPRequest:
    if ( event->revision() > 0 ) {
      return i18n( "<h3>This meeting has been updated</h3>" );
    } else {
      return i18n( "You have been invited to this meeting" );
    }
  case iTIPRefresh:
    return i18n( "This invitation was refreshed" );
  case iTIPCancel:
    return i18n( "This meeting has been canceled" );
  case iTIPAdd:
    return i18n( "Addition to the meeting invitation" );
  case iTIPReply:
  {
    Attendee::List attendees = event->attendees();
    if( attendees.count() == 0 ) {
      kDebug() << "No attendees in the iCal reply!";
      return QString();
    }
    if ( attendees.count() != 1 ) {
      kDebug() << "Warning: attendeecount in the reply should be 1"
               << "but is" << attendees.count();
    }
    Attendee *attendee = *attendees.begin();
    QString attendeeName = attendee->name();
    if ( attendeeName.isEmpty() ) {
      attendeeName = attendee->email();
    }
    if ( attendeeName.isEmpty() ) {
      attendeeName = i18n( "Sender" );
    }

    QString delegatorName, dummy;
    KPIMUtils::extractEmailAddressAndName( attendee->delegator(), dummy, delegatorName );
    if ( delegatorName.isEmpty() ) {
      delegatorName = attendee->delegator();
    }

    switch( attendee->status() ) {
    case Attendee::NeedsAction:
      return i18n( "%1 indicates this invitation still needs some action", attendeeName );
    case Attendee::Accepted:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 accepts this meeting invitation", attendeeName );
      }
      return i18n( "%1 accepts this meeting invitation on behalf of %2",
                   attendeeName, delegatorName );
    case Attendee::Tentative:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 tentatively accepts this meeting invitation", attendeeName );
      }
      return i18n( "%1 tentatively accepts this meeting invitation on behalf of %2",
                   attendeeName, delegatorName );
    case Attendee::Declined:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 declines this meeting invitation", attendeeName );
      }
      return i18n( "%1 declines this meeting invitation on behalf of %2",
                   attendeeName, delegatorName );
    case Attendee::Delegated:
    {
      QString delegate, dummy;
      KPIMUtils::extractEmailAddressAndName( attendee->delegate(), dummy, delegate );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "%1 has delegated this meeting invitation to %2", attendeeName, delegate );
      }
      return i18n( "%1 has delegated this meeting invitation", attendeeName );
    }
    case Attendee::Completed:
      return i18n( "This meeting invitation is now completed" );
    case Attendee::InProcess:
      return i18n( "%1 is still processing the invitation", attendeeName );
    default:
      return i18n( "Unknown response to this meeting invitation" );
    }
    break;
  }
  case iTIPCounter:
    return i18n( "Sender makes this counter proposal" );
  case iTIPDeclineCounter:
    return i18n( "Sender declines the counter proposal" );
  case iTIPNoMethod:
    return i18n( "Error: iMIP message with unknown method: '%1'", msg->method() );
  }
  return QString();
}

static QString invitationHeaderTodo( Todo *todo, ScheduleMessage *msg )
{
  if ( !msg || !todo ) {
    return QString();
  }

  switch ( msg->method() ) {
  case iTIPPublish:
    return i18n( "This to-do has been published" );
  case iTIPRequest:
    if ( todo->revision() > 0 ) {
      return i18n( "This to-do has been updated" );
    } else {
      return i18n( "You have been assigned this to-do" );
    }
  case iTIPRefresh:
    return i18n( "This to-do was refreshed" );
  case iTIPCancel:
    return i18n( "This to-do was canceled" );
  case iTIPAdd:
    return i18n( "Addition to the to-do" );
  case iTIPReply:
  {
    Attendee::List attendees = todo->attendees();
    if ( attendees.count() == 0 ) {
      kDebug() << "No attendees in the iCal reply!";
      return QString();
    }
    if ( attendees.count() != 1 ) {
      kDebug() << "Warning: attendeecount in the reply should be 1"
               << "but is" << attendees.count();
    }
    Attendee *attendee = *attendees.begin();
    switch( attendee->status() ) {
    case Attendee::NeedsAction:
      return i18n( "Sender indicates this to-do assignment still needs some action" );
    case Attendee::Accepted:
      return i18n( "Sender accepts this to-do" );
    case Attendee::Tentative:
      return i18n( "Sender tentatively accepts this to-do" );
    case Attendee::Declined:
      return i18n( "Sender declines this to-do" );
    case Attendee::Delegated:
    {
      QString delegate, dummy;
      KPIMUtils::extractEmailAddressAndName( attendee->delegate(), dummy, delegate );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "Sender has delegated this request for the to-do to %1", delegate );
      }
      return i18n( "Sender has delegated this request for the to-do " );
    }
    case Attendee::Completed:
      return i18n( "The request for this to-do is now completed" );
    case Attendee::InProcess:
      return i18n( "Sender is still processing the invitation" );
    default:
      return i18n( "Unknown response to this to-do" );
    }
    break;
  }
  case iTIPCounter:
    return i18n( "Sender makes this counter proposal" );
  case iTIPDeclineCounter:
    return i18n( "Sender declines the counter proposal" );
  case iTIPNoMethod:
    return i18n( "Error: iMIP message with unknown method: '%1'", msg->method() );
  }
  return QString();
}

static QString invitationHeaderJournal( Journal *journal, ScheduleMessage *msg )
{
  // TODO: Several of the methods are not allowed for journals, so remove them.
  if ( !msg || !journal ) {
    return QString();
  }

  switch ( msg->method() ) {
  case iTIPPublish:
    return i18n( "This journal has been published" );
  case iTIPRequest:
    return i18n( "You have been assigned this journal" );
  case iTIPRefresh:
    return i18n( "This journal was refreshed" );
  case iTIPCancel:
    return i18n( "This journal was canceled" );
  case iTIPAdd:
    return i18n( "Addition to the journal" );
  case iTIPReply:
  {
    Attendee::List attendees = journal->attendees();
    if ( attendees.count() == 0 ) {
      kDebug() << "No attendees in the iCal reply!";
      return QString();
    }

    if( attendees.count() != 1 ) {
      kDebug() << "Warning: attendeecount in the reply should be 1"
               << "but is" << attendees.count();
    }

    Attendee *attendee = *attendees.begin();
    switch( attendee->status() ) {
    case Attendee::NeedsAction:
      return i18n( "Sender indicates this journal assignment still needs some action" );
    case Attendee::Accepted:
      return i18n( "Sender accepts this journal" );
    case Attendee::Tentative:
      return i18n( "Sender tentatively accepts this journal" );
    case Attendee::Declined:
      return i18n( "Sender declines this journal" );
    case Attendee::Delegated:
      return i18n( "Sender has delegated this request for the journal" );
    case Attendee::Completed:
      return i18n( "The request for this journal is now completed" );
    case Attendee::InProcess:
      return i18n( "Sender is still processing the invitation" );
    default:
      return i18n( "Unknown response to this journal" );
    }
    break;
  }
  case iTIPCounter:
    return i18n( "Sender makes this counter proposal" );
  case iTIPDeclineCounter:
    return i18n( "Sender declines the counter proposal" );
  case iTIPNoMethod:
    return i18n( "Error: iMIP message with unknown method: '%1'", msg->method() );
  }
  return QString();
}

static QString invitationHeaderFreeBusy( FreeBusy *fb, ScheduleMessage *msg )
{
  if ( !msg || !fb ) {
    return QString();
  }

  switch ( msg->method() ) {
  case iTIPPublish:
    return i18n( "This free/busy list has been published" );
  case iTIPRequest:
    return i18n( "The free/busy list has been requested" );
  case iTIPRefresh:
    return i18n( "This free/busy list was refreshed" );
  case iTIPCancel:
    return i18n( "This free/busy list was canceled" );
  case iTIPAdd:
    return i18n( "Addition to the free/busy list" );
  case iTIPNoMethod:
  default:
    return i18n( "Error: Free/Busy iMIP message with unknown method: '%1'", msg->method() );
  }
}

//@cond PRIVATE
class KCal::IncidenceFormatter::ScheduleMessageVisitor : public IncidenceBase::Visitor
{
  public:
    ScheduleMessageVisitor() : mMessage(0) { mResult = ""; }
    bool act( IncidenceBase *incidence, ScheduleMessage *msg )
    {
      mMessage = msg;
      return incidence->accept( *this );
    }
    QString result() const { return mResult; }

  protected:
    QString mResult;
    ScheduleMessage *mMessage;
};

class KCal::IncidenceFormatter::InvitationHeaderVisitor :
      public IncidenceFormatter::ScheduleMessageVisitor
{
  protected:
    bool visit( Event *event )
    {
      mResult = invitationHeaderEvent( event, mMessage );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = invitationHeaderTodo( todo, mMessage );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = invitationHeaderJournal( journal, mMessage );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = invitationHeaderFreeBusy( fb, mMessage );
      return !mResult.isEmpty();
    }
};

class KCal::IncidenceFormatter::InvitationBodyVisitor
  : public IncidenceFormatter::ScheduleMessageVisitor
{
  protected:
    bool visit( Event *event )
    {
      mResult = invitationDetailsEvent( event );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = invitationDetailsTodo( todo );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = invitationDetailsJournal( journal );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = invitationDetailsFreeBusy( fb );
      return !mResult.isEmpty();
    }
};
//@endcond

QString InvitationFormatterHelper::generateLinkURL( const QString &id )
{
  return id;
}

class IncidenceFormatter::IncidenceCompareVisitor :
  public IncidenceBase::Visitor
{
  public:
    IncidenceCompareVisitor() : mExistingIncidence( 0 ) {}
    bool act( IncidenceBase *incidence, Incidence *existingIncidence )
    {
      Incidence *inc = dynamic_cast<Incidence *>( incidence );
      if ( inc && inc->revision() <= existingIncidence->revision() ) {
        return false;
      }
      mExistingIncidence = existingIncidence;
      return incidence->accept( *this );
    }

    QString result() const
    {
      if ( mChanges.isEmpty() ) {
        return QString();
      }
      QString html = "<div align=\"left\"><ul><li>";
      html += mChanges.join( "</li><li>" );
      html += "</li><ul></div>";
      return html;
    }

  protected:
    bool visit( Event *event )
    {
      compareEvents( event, dynamic_cast<Event*>( mExistingIncidence ) );
      compareIncidences( event, mExistingIncidence );
      return !mChanges.isEmpty();
    }
    bool visit( Todo *todo )
    {
      compareIncidences( todo, mExistingIncidence );
      return !mChanges.isEmpty();
    }
    bool visit( Journal *journal )
    {
      compareIncidences( journal, mExistingIncidence );
      return !mChanges.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      Q_UNUSED( fb );
      return !mChanges.isEmpty();
    }

  private:
    void compareEvents( Event *newEvent, Event *oldEvent )
    {
      if ( !oldEvent || !newEvent ) {
        return;
      }
      if ( oldEvent->dtStart() != newEvent->dtStart() ||
           oldEvent->allDay() != newEvent->allDay() ) {
        mChanges += i18n( "The begin of the meeting has been changed from %1 to %2",
                          eventStartTimeStr( oldEvent ), eventStartTimeStr( newEvent ) );
      }
      if ( oldEvent->dtEnd() != newEvent->dtEnd() ||
           oldEvent->allDay() != newEvent->allDay() ) {
        mChanges += i18n( "The end of the meeting has been changed from %1 to %2",
                          eventEndTimeStr( oldEvent ), eventEndTimeStr( newEvent ) );
      }
    }

    void compareIncidences( Incidence *newInc, Incidence *oldInc )
    {
      if ( !oldInc || !newInc ) {
        return;
      }

      if ( oldInc->summary() != newInc->summary() ) {
        mChanges += i18n( "The summary has been changed to: \"%1\"",
                          newInc->richSummary() );
      }

      if ( oldInc->location() != newInc->location() ) {
        mChanges += i18n( "The location has been changed to: \"%1\"",
                          newInc->richLocation() );
      }

      if ( oldInc->description() != newInc->description() ) {
        mChanges += i18n( "The description has been changed to: \"%1\"",
                          newInc->richDescription() );
      }

      Attendee::List oldAttendees = oldInc->attendees();
      Attendee::List newAttendees = newInc->attendees();
      for ( Attendee::List::ConstIterator it = newAttendees.constBegin();
            it != newAttendees.constEnd(); ++it ) {
        Attendee *oldAtt = oldInc->attendeeByMail( (*it)->email() );
        if ( !oldAtt ) {
          mChanges += i18n( "Attendee %1 has been added", (*it)->fullName() );
        } else {
          if ( oldAtt->status() != (*it)->status() ) {
            mChanges += i18n( "The status of attendee %1 has been changed to: %2",
                              (*it)->fullName(), (*it)->statusStr() );
          }
        }
      }

      for ( Attendee::List::ConstIterator it = oldAttendees.constBegin();
            it != oldAttendees.constEnd(); ++it ) {
        Attendee *newAtt = newInc->attendeeByMail( (*it)->email() );
        if ( !newAtt ) {
          mChanges += i18n( "Attendee %1 has been removed", (*it)->fullName() );
        }
      }
    }

  private:
    Incidence *mExistingIncidence;
    QStringList mChanges;
};

QString InvitationFormatterHelper::makeLink( const QString &id, const QString &text )
{
  QString res( "<a href=\"%1\"><b>%2</b></a>" );
  return res.arg( generateLinkURL( id ) ).arg( text );
  return res;
}

Calendar *InvitationFormatterHelper::calendar() const
{
  return 0;
}

// Check if the given incidence is likely one that we own instead one from
// a shared calendar (Kolab-specific)
static bool incidenceOwnedByMe( Calendar *calendar, Incidence *incidence )
{
  CalendarResources* cal = dynamic_cast<CalendarResources*>( calendar );
  if ( !cal || !incidence ) {
    return true;
  }

  ResourceCalendar *res = cal->resource( incidence );
  if ( !res ) {
    return true;
  }

  const QString subRes = res->subresourceIdentifier( incidence );
  if ( !subRes.contains( "/.INBOX.directory/" ) ) {
    return false;
  }
  return true;
}

QString IncidenceFormatter::formatICalInvitation( QString invitation, Calendar *mCalendar,
    InvitationFormatterHelper *helper )
{
  if ( invitation.isEmpty() ) {
    return QString();
  }

  ICalFormat format;
  // parseScheduleMessage takes the tz from the calendar,
  // no need to set it manually here for the format!
  ScheduleMessage *msg = format.parseScheduleMessage( mCalendar, invitation );

  if( !msg ) {
    kDebug() << "Failed to parse the scheduling message";
    Q_ASSERT( format.exception() );
    kDebug() << format.exception()->message();
    return QString();
  }

  IncidenceBase *incBase = msg->event();

  Incidence *existingIncidence = 0;
  if ( helper->calendar() ) {
    existingIncidence = helper->calendar()->incidence( incBase->uid() );
    if ( !incidenceOwnedByMe( helper->calendar(), existingIncidence ) ) {
      existingIncidence = 0;
    }
    if ( !existingIncidence ) {
      const Incidence::List list = helper->calendar()->incidences();
      for ( Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it ) {
        if ( (*it)->schedulingID() == incBase->uid() &&
             incidenceOwnedByMe( helper->calendar(), *it ) ) {
          existingIncidence = *it;
          break;
        }
      }
    }
  }

  // First make the text of the message
  QString html;

  QString tableStyle = QString::fromLatin1(
    "style=\"border: solid 1px; margin: 0em;\"" );
  QString tableHead = QString::fromLatin1(
    "<div align=\"center\">"
    "<table width=\"80%\" cellpadding=\"1\" cellspacing=\"0\" %1>"
    "<tr><td>" ).arg( tableStyle );

  html += tableHead;
  InvitationHeaderVisitor headerVisitor;
  // The InvitationHeaderVisitor returns false if the incidence is somehow invalid, or not handled
  if ( !headerVisitor.act( incBase, msg ) ) {
    return QString();
  }
  html += "<h3>" + headerVisitor.result() + "</h3>";

  InvitationBodyVisitor bodyVisitor;
  if ( !bodyVisitor.act( incBase, msg ) ) {
    return QString();
  }
  html += bodyVisitor.result();

  if ( msg->method() == iTIPRequest ) { // ### Scheduler::Publish/Refresh/Add as well?
    IncidenceCompareVisitor compareVisitor;
    if ( compareVisitor.act( incBase, existingIncidence ) ) {
      html +=
        i18n( "<p align=\"left\">The following changes have been made by the organizer:</p>" );
      html += compareVisitor.result();
    }
  }

  html += "<br/>";
  html += "<table border=\"0\" cellspacing=\"0\"><tr><td>&nbsp;</td></tr><tr>";

#if 0
  // TODO: implement this
  html += helper->makeLinkURL( "accept", i18n( "[Enter this into my calendar]" ) );
  html += "</td><td> &nbsp; </td><td>";
#endif

  // Add groupware links

  Incidence *incidence = dynamic_cast<Incidence*>( incBase );
  switch ( msg->method() ) {
  case iTIPPublish:
  case iTIPRequest:
  case iTIPRefresh:
  case iTIPAdd:
  {
    if ( incidence && incidence->revision() > 0 && ( existingIncidence || !helper->calendar() ) ) {
      if ( incBase->type() == "Todo" ) {
        html += "<td colspan=\"13\">";
        html += helper->makeLink( "reply", i18n( "[Enter this into my to-do list]" ) );
      } else {
        html += "<td colspan=\"9\">";
        html += helper->makeLink( "reply", i18n( "[Enter this into my calendar]" ) );
      }
      html += "</td></tr><tr>";
    }
    html += "<td>";

    if ( !existingIncidence ) {
      // Accept
      html += helper->makeLink( "accept", i18nc( "accept to-do request", "[Accept]" ) );
      html += "</td><td> &nbsp; </td><td>";
      html += helper->makeLink( "accept_conditionally",
                              i18nc( "Accept conditionally", "[Accept cond.]" ) );
      html += "</td><td> &nbsp; </td><td>";
      // counter proposal
      html += helper->makeLink( "counter", i18n( "[Counter proposal]" ) );
      html += "</td><td> &nbsp; </td><td>";
      // Decline
      html += helper->makeLink( "decline", i18nc( "decline to-do request", "[Decline]" ) );
      html += "</td><td> &nbsp; </td><td>";

      // Delegate
      html += helper->makeLink( "delegate", i18nc( "delegate to-do to another", "[Delegate]" ) );
      html += "</td><td> &nbsp; </td><td>";

      // Forward
      html += helper->makeLink( "forward", i18nc( "forward request to another", "[Forward]" ) );

      if ( incBase->type() == "Event" ) {
        html += "</b></a></td><td> &nbsp; </td><td>";
        html += helper->makeLink( "check_calendar", i18n("[Check my calendar]" ) );
      }
    }
    break;
  }

  case iTIPCancel:
    // Cancel event from my calendar
    html += helper->makeLink( "cancel", i18n( "[Remove this from my calendar]" ) );
    break;

  case iTIPReply:
    // Enter this into my calendar
    if ( incBase->type() == "Todo" ) {
      html += helper->makeLink( "reply", i18n( "[Enter this into my to-do list]" ) );
    } else {
      html += helper->makeLink( "reply", i18n( "[Enter this into my calendar]" ) );
    }
    break;

  case iTIPCounter:
  case iTIPDeclineCounter:
  case iTIPNoMethod:
    break;
  }

  html += "</td></tr></table>";

  html += "</td></tr></table><br></div>";

  return html;
}

/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

//@cond PRIVATE
class KCal::IncidenceFormatter::ToolTipVisitor : public IncidenceBase::Visitor
{
  public:
    ToolTipVisitor() : mRichText( true ), mResult( "" ) {}

    bool act( IncidenceBase *incidence, bool richText=true )
    {
      mRichText = richText;
      mResult = "";
      return incidence ? incidence->accept( *this ) : false;
    }
    QString result() const { return mResult; }

  protected:
    bool visit( Event *event );
    bool visit( Todo *todo );
    bool visit( Journal *journal );
    bool visit( FreeBusy *fb );

    QString dateRangeText( Event *event );
    QString dateRangeText( Todo *todo );
    QString dateRangeText( Journal *journal );
    QString dateRangeText( FreeBusy *fb );

    QString generateToolTip( Incidence *incidence, QString dtRangeText );

  protected:
    bool mRichText;
    QString mResult;
};

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Event *event )
{
  //FIXME: support mRichText==false
  //TODO: the &nbsp; in the strings are obsolete, they should be removed
  //      (couldn't do that because of the string freeze)
  QString ret;
  QString tmp;
  if ( event->isMultiDay() ) {

    tmp = event->dtStartStr( true, event->dtStart().timeSpec() );
    ret += "<br>" + i18nc( "Event start", "<i>From:</i>&nbsp;%1", tmp );

    tmp = event->dtEndStr( true, event->dtEnd().timeSpec() );
    ret += "<br>" + i18nc( "Event end","<i>To:</i>&nbsp;%1", tmp );

  } else {

    ret += "<br>" +
           i18n( "<i>Date:</i>&nbsp;%1",
                 event->dtStartDateStr(
                   true, event->dtStart().timeSpec() ) );
    if ( !event->allDay() ) {
      if ( event->dtStartTimeStr( true, event->dtStart().timeSpec() ) ==
           event->dtEndTimeStr( true, event->dtEnd().timeSpec() ) ) {
        // to prevent 'Time: 17:00 - 17:00'
        tmp = "<br>" +
              // TODO: the comment is no longer true, &nbsp; is not needed anymore, I leave
              // because of the string freeze
              i18nc( "time for event, &nbsp; to prevent ugly line breaks", "<i>Time:</i>&nbsp;%1",
                     event->dtStartTimeStr(
                       true, event->dtStart().timeSpec() ) );
      } else {
        tmp = "<br>" +
              // TODO: the comment is no longer true, &nbsp; is not needed anymore, I leave
              // because of the string freeze
              i18nc( "time range for event, &nbsp; to prevent ugly line breaks",
                     "<i>Time:</i>&nbsp;%1&nbsp;-&nbsp;%2",
                     event->dtStartTimeStr(
                       true, event->dtStart().timeSpec() ),
                     event->dtEndTimeStr(
                       true, event->dtEnd().timeSpec() ) );
      }
      ret += tmp;
    }
  }
  return ret.replace( " ", "&nbsp;" );
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Todo *todo )
{
  //FIXME: support mRichText==false
  //TODO: the &nbsp; in the strings are obsolete, they should be removed
  //      (couldn't do that because of the string freeze)
  QString ret;
  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    // No need to add <i> here. This is separated issue and each line
    // is very visible on its own. On the other hand... Yes, I like it
    // italics here :)
    ret += "<br>" + i18n( "<i>Start:</i>&nbsp;%1",
                            todo->dtStartStr(
                            true, false, todo->dtStart().timeSpec() ) ) ;
  }
  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    ret += "<br>" + i18n( "<i>Due:</i>&nbsp;%1",
                            todo->dtDueStr(
                            true, todo->dtDue().timeSpec() ) );
  }
  if ( todo->isCompleted() ) {
    ret += "<br>" +
           i18n( "<i>Completed:</i>&nbsp;%1", todo->completedStr() );
  } else {
    ret += "<br>" +
           i18nc( "percent complete", "%1 % completed", todo->percentComplete() );
  }

  return ret.replace( " ", "&nbsp;" );
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Journal *journal )
{
  //FIXME: support mRichText==false
  //TODO: the &nbsp; in the strings are obsolete, they should be removed
  //      (couldn't do that because of the string freeze)
  QString ret;
  if ( journal->dtStart().isValid() ) {
    ret += "<br>" +
           i18n( "<i>Date:</i>&nbsp;%1",
                 journal->dtStartDateStr( false, journal->dtStart().timeSpec() ) );
  }
  return ret.replace( " ", "&nbsp;" );
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( FreeBusy *fb )
{
  //FIXME: support mRichText==false
  //TODO: the &nbsp; in the strings are obsolete, they should be removed
  //      (couldn't do that because of the string freeze)
  QString ret;
  ret = "<br>" +
        i18n( "<i>Period start:</i>&nbsp;%1",
              KGlobal::locale()->formatDateTime( fb->dtStart().dateTime() ) );
  ret += "<br>" +
         i18n( "<i>Period start:</i>&nbsp;%1",
               KGlobal::locale()->formatDateTime( fb->dtEnd().dateTime() ) );
  return ret.replace( " ", "&nbsp;" );
}

bool IncidenceFormatter::ToolTipVisitor::visit( Event *event )
{
  mResult = generateToolTip( event, dateRangeText( event ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( Todo *todo )
{
  mResult = generateToolTip( todo, dateRangeText( todo ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( Journal *journal )
{
  mResult = generateToolTip( journal, dateRangeText( journal ) );
  return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit( FreeBusy *fb )
{
  //FIXME: support mRichText==false
  mResult = "<qt><b>" + i18n( "Free/Busy information for %1", fb->organizer().fullName() ) + "</b>";
  mResult += dateRangeText( fb );
  mResult += "</qt>";
  return !mResult.isEmpty();
}

QString IncidenceFormatter::ToolTipVisitor::generateToolTip( Incidence *incidence,
                                                             QString dtRangeText )
{
  //FIXME: support mRichText==false

  if ( !incidence ) {
    return QString();
  }

  QString tmp = "<qt><b>"+ incidence->richSummary() + "</b>";

  tmp += dtRangeText;

  if ( !incidence->location().isEmpty() ) {
    // Put Location: in italics
    tmp += "<br>" +
           i18n( "<i>Location:</i>&nbsp;%1", incidence->richLocation() );
  }

  if ( !incidence->description().isEmpty() ) {
    QString desc( incidence->description() );
    if ( !incidence->descriptionIsRich() ) {
      if ( desc.length() > 120 ) {
        desc = desc.left( 120 ) + "...";
      }
      desc = Qt::escape( desc ).replace( "\n", "<br>" );
    } else {
      // TODO: truncate the description when it's rich text
    }
    tmp += "<br>----------<br>" + i18n( "<i>Description:</i>" ) + "<br>" + desc;
  }
  tmp += "</qt>";
  return tmp;
}
//@endcond

QString IncidenceFormatter::toolTipString( IncidenceBase *incidence, bool richText )
{
  ToolTipVisitor v;
  if ( v.act( incidence, richText ) ) {
    return v.result();
  } else {
    return QString();
  }
}

/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

static QString mailBodyIncidence( Incidence *incidence )
{
  QString body;
  if ( !incidence->summary().isEmpty() ) {
    body += i18n( "Summary: %1\n", incidence->richSummary() );
  }
  if ( !incidence->organizer().isEmpty() ) {
    body += i18n( "Organizer: %1\n", incidence->organizer().fullName() );
  }
  if ( !incidence->location().isEmpty() ) {
    body += i18n( "Location: %1\n", incidence->richLocation() );
  }
  return body;
}

//@cond PRIVATE
class KCal::IncidenceFormatter::MailBodyVisitor : public IncidenceBase::Visitor
{
  public:
    MailBodyVisitor() : mResult( "" ) {}

    bool act( IncidenceBase *incidence )
    {
      mResult = "";
      return incidence ? incidence->accept( *this ) : false;
    }
    QString result() const
    {
      return mResult;
    }

  protected:
    bool visit( Event *event );
    bool visit( Todo *todo );
    bool visit( Journal *journal );
    bool visit( FreeBusy * )
    {
      mResult = i18n( "This is a Free Busy Object" );
      return !mResult.isEmpty();
    }
  protected:
    QString mResult;
};

bool IncidenceFormatter::MailBodyVisitor::visit( Event *event )
{
  QString recurrence[]= {
    i18nc( "no recurrence", "None" ),
    i18nc( "event recurs by minutes", "Minutely" ),
    i18nc( "event recurs by hours", "Hourly" ),
    i18nc( "event recurs by days", "Daily" ),
    i18nc( "event recurs by weeks", "Weekly" ),
    i18nc( "event recurs same position (e.g. first monday) each month", "Monthly Same Position" ),
    i18nc( "event recurs same day each month", "Monthly Same Day" ),
    i18nc( "event recurs same month each year", "Yearly Same Month" ),
    i18nc( "event recurs same day each year", "Yearly Same Day" ),
    i18nc( "event recurs same position (e.g. first monday) each year", "Yearly Same Position" )
  };

  mResult = mailBodyIncidence( event );
  mResult += i18n( "Start Date: %1\n",
                   event->dtStartDateStr( true, event->dtStart().timeSpec() ) );
  if ( !event->allDay() ) {
    mResult += i18n( "Start Time: %1\n",
                     event->dtStartTimeStr( true, event->dtStart().timeSpec() ) );
  }
  if ( event->dtStart() != event->dtEnd() ) {
    mResult += i18n( "End Date: %1\n",
                     event->dtEndDateStr( true, event->dtStart().timeSpec() ) );
  }
  if ( !event->allDay() ) {
    mResult += i18n( "End Time: %1\n",
                     event->dtEndTimeStr( true, event->dtStart().timeSpec() ) );
  }
  if ( event->recurs() ) {
    Recurrence *recur = event->recurrence();
    // TODO: Merge these two to one of the form "Recurs every 3 days"
    mResult += i18n( "Recurs: %1\n", recurrence[ recur->recurrenceType() ] );
    mResult += i18n( "Frequency: %1\n", event->recurrence()->frequency() );

    if ( recur->duration() > 0 ) {
      mResult += i18np( "Repeats once", "Repeats %1 times", recur->duration() );
      mResult += '\n';
    } else {
      if ( recur->duration() != -1 ) {
// TODO_Recurrence: What to do with all-day
        QString endstr;
        if ( event->allDay() ) {
          endstr = KGlobal::locale()->formatDate( recur->endDate() );
        } else {
          endstr = KGlobal::locale()->formatDateTime( recur->endDateTime().dateTime() );
        }
        mResult += i18n( "Repeat until: %1\n", endstr );
      } else {
        mResult += i18n( "Repeats forever\n" );
      }
    }
  }

  QString details = event->richDescription();
  if ( !details.isEmpty() ) {
    mResult += i18n( "Details:\n%1\n", details );
  }
  return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit( Todo *todo )
{
  mResult = mailBodyIncidence( todo );

  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    mResult += i18n( "Start Date: %1\n",
                     todo->dtStartDateStr( true, false, todo->dtStart().timeSpec() ) );
    if ( !todo->allDay() ) {
      mResult += i18n( "Start Time: %1\n",
                       todo->dtStartTimeStr( true, false, todo->dtStart().timeSpec() ) );
    }
  }
  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    mResult += i18n( "Due Date: %1\n",
                     todo->dtDueDateStr( true, todo->dtDue().timeSpec() ) );
    if ( !todo->allDay() ) {
      mResult += i18n( "Due Time: %1\n",
                       todo->dtDueTimeStr( true, todo->dtDue().timeSpec() ) );
    }
  }
  QString details = todo->richDescription();
  if ( !details.isEmpty() ) {
    mResult += i18n( "Details:\n%1\n", details );
  }
  return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit( Journal *journal )
{
  mResult = mailBodyIncidence( journal );
  mResult += i18n( "Date: %1\n", journal->dtStartDateStr( true, journal->dtStart().timeSpec() ) );
  if ( !journal->allDay() ) {
    mResult += i18n( "Time: %1\n", journal->dtStartTimeStr( true, journal->dtStart().timeSpec() ) );
  }
  if ( !journal->description().isEmpty() ) {
    mResult += i18n( "Text of the journal:\n%1\n", journal->richDescription() );
  }
  return !mResult.isEmpty();
}
//@endcond

QString IncidenceFormatter::mailBodyString( IncidenceBase *incidence )
{
  if ( !incidence ) {
    return QString();
  }

  MailBodyVisitor v;
  if ( v.act( incidence ) ) {
    return v.result();
  }
  return QString();
}

static QString recurEnd( Incidence *incidence )
{
  QString endstr;
  if ( incidence->allDay() ) {
    endstr = KGlobal::locale()->formatDate( incidence->recurrence()->endDate() );
  } else {
    endstr = KGlobal::locale()->formatDateTime( incidence->recurrence()->endDateTime() );
  }
  return endstr;
}

QString IncidenceFormatter::recurrenceString( Incidence *incidence )
{
  if ( !incidence->recurs() ) {
    return i18n( "No recurrence" );
  }

  Recurrence *recur = incidence->recurrence();
  switch ( recur->recurrenceType() ) {
  case Recurrence::rNone:
    return i18n( "No recurrence" );
  case Recurrence::rMinutely:
    if ( recur->duration() != -1 ) {
      return i18np( "Recurs every minute until %2",
                    "Recurs every %1 minutes until %2",
                    recur->frequency(), recurEnd( incidence ) );
    }
    return i18np( "Recurs every minute",
                  "Recurs every %1 minutes", recur->frequency() );
  case Recurrence::rHourly:
    if ( recur->duration() != -1 ) {
      return i18np( "Recurs hourly until %2",
                    "Recurs every %1 hours until %2",
                    recur->frequency(), recurEnd( incidence ) );
    }
    return i18np( "Recurs hourly", "Recurs every %1 hours", recur->frequency() );
  case Recurrence::rDaily:
    if ( recur->duration() != -1 ) {
      return i18np( "Recurs daily until %2",
                    "Recurs every %1 days until %2",
                    recur->frequency(), recurEnd( incidence ) );
    }
    return i18np( "Recurs daily", "Recurs every %1 days", recur->frequency() );
  case Recurrence::rWeekly:
    if ( recur->duration() != -1 ) {
      return i18np( "Recurs weekly until %2",
                    "Recurs every %1 weeks until %2",
                    recur->frequency(), recurEnd( incidence ) );
    }
    return i18np( "Recurs weekly", "Recurs every %1 weeks", recur->frequency() );
  case Recurrence::rMonthlyPos:
  case Recurrence::rMonthlyDay:
    if ( recur->duration() != -1 ) {
      return i18n( "Recurs monthly until %1", recurEnd( incidence ) );
    }
    return i18n( "Recurs monthly" );
  case Recurrence::rYearlyMonth:
  case Recurrence::rYearlyDay:
  case Recurrence::rYearlyPos:
    if ( recur->duration() != -1 ) {
      return i18n( "Recurs yearly until %1", recurEnd( incidence ) );
    }
    return i18n( "Recurs yearly" );
  default:
    return i18n( "Incidence recurs" );
  }
}
