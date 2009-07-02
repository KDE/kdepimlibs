/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
  This file is part of the API for handling calendar data and provides
  static functions for formatting Incidences for various purposes.

  @brief
  Provides methods to format Incidences in various ways for display purposes.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
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
#include <kemailsettings.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <ksystemtimezone.h>

#include <QtCore/QBuffer>
#include <QtCore/QList>
#include <QtGui/QTextDocument>
#include <QtGui/QApplication>

#include <time.h>

using namespace KCal;

/*******************************************************************
 *  Helper functions for the extensive display (event viewer)
 *******************************************************************/

//@cond PRIVATE
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
      for ( int i = 0; i <= numLineBreaks; ++i ) {
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
    KUrl mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( email );
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
    for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
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
    for ( it = as.constBegin(); it != as.constEnd(); ++it ) {
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
  KIconLoader *iconLoader = KIconLoader::global();
  tmpStr += "<td>";

  // TODO: KDE5. Make the function QString Incidence::getPixmap() so we don't
  // need downcasting.

  if ( incidence->type() == "Todo" ) {
    tmpStr += "<img src=\"";
    Todo *todo = static_cast<Todo *>( incidence );
    if ( !todo->isCompleted() ) {
      tmpStr += iconLoader->iconPath( "view-calendar-tasks", KIconLoader::Small );
    } else {
      tmpStr += iconLoader->iconPath( "task-complete", KIconLoader::Small );
    }
    tmpStr += "\">";
  }

  if ( incidence->type() == "Event" ) {
    tmpStr += "<img src=\"" +
              iconLoader->iconPath( "view-calendar-day", KIconLoader::Small ) +
              "\">";
  }

  if ( incidence->type() == "Journal" ) {
    tmpStr += "<img src=\"" +
              iconLoader->iconPath( "view-pim-journal", KIconLoader::Small ) +
              "\">";
  }

  if ( incidence->isAlarmEnabled() ) {
    tmpStr += "<img src=\"" +
              iconLoader->iconPath( "preferences-desktop-notification-bell", KIconLoader::Small ) +
              "\">";
  }
  if ( incidence->recurs() ) {
    tmpStr += "<img src=\"" +
              iconLoader->iconPath( "edit-redo", KIconLoader::Small ) +
              "\">";
  }
  if ( incidence->isReadOnly() ) {
    tmpStr += "<img src=\"" +
              iconLoader->iconPath( "object-locked", KIconLoader::Small ) +
              "\">";
  }
  tmpStr += "</td>";

  tmpStr += "<td>" +
            eventViewerAddTag( "h2", incidence->richSummary() ) +
            "</td>";
  tmpStr += "</tr></table>";

  return tmpStr;
}

static QString eventViewerFormatEvent( Event *event, KDateTime::Spec spec )
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
      tmpStr += "<td>" +
                i18nc( "<beginTime> - <endTime>","%1 - %2",
                       IncidenceFormatter::dateToString( event->dtStart(), true, spec ),
                       IncidenceFormatter::dateToString( event->dtEnd(), true, spec ) ) +
                "</td>";
    } else {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Date" ) + "</b></td>";
      tmpStr += "<td>" +
                i18nc( "date as string","%1",
                       IncidenceFormatter::dateToString( event->dtStart(), true, spec ) ) +
                "</td>";
    }
  } else {
    if ( event->isMultiDay() ) {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      tmpStr += "<td>" +
                i18nc( "<beginTime> - <endTime>","%1 - %2",
                       IncidenceFormatter::dateToString( event->dtStart(), true, spec ),
                       IncidenceFormatter::dateToString( event->dtEnd(), true, spec ) ) +
                "</td>";
    } else {
      tmpStr += "<td align=\"right\"><b>" + i18n( "Time" ) + "</b></td>";
      if ( event->hasEndDate() && event->dtStart() != event->dtEnd() ) {
        tmpStr += "<td>" +
                  i18nc( "<beginTime> - <endTime>","%1 - %2",
                         IncidenceFormatter::timeToString( event->dtStart(), true, spec ),
                         IncidenceFormatter::timeToString( event->dtEnd(), true, spec ) ) +
                  "</td>";
      } else {
        tmpStr += "<td>" +
                  IncidenceFormatter::timeToString( event->dtStart(), true, spec ) +
                  "</td>";
      }
      tmpStr += "</tr><tr>";
      tmpStr += "<td align=\"right\"><b>" + i18n( "Date" ) + "</b></td>";
      tmpStr += "<td>" +
                i18nc( "date as string","%1",
                       IncidenceFormatter::dateToString( event->dtStart(), true, spec ) ) +
                "</td>";
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
              ( dt.isValid() ?
                KGlobal::locale()->formatDateTime( dt.dateTime(), KLocale::ShortDate ) :
                i18nc( "no date", "none" ) ) +
              "</td>";
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
  KDateTime kdt = event->created().toTimeSpec( spec );
  tmpStr += "</table>";
  tmpStr += "<p><em>" +
            i18n( "Creation date: %1", KGlobal::locale()->formatDateTime(
                    kdt.dateTime(),
                    KLocale::ShortDate ) ) + "</em>";
  return tmpStr;
}

static QString eventViewerFormatTodo( Todo *todo, KDateTime::Spec spec )
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
    tmpStr += i18n( "<b>Due on:</b> %1",
                    IncidenceFormatter::dateTimeToString( todo->dtDue(),
                                                          todo->allDay(),
                                                          true, spec ) );
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

  KDateTime kdt = todo->created().toTimeSpec( spec );
  tmpStr += "<p><em>" +
            i18n( "Creation date: %1",
                  KGlobal::locale()->formatDateTime( kdt.dateTime(), KLocale::ShortDate ) ) +
            "</em>";
  return tmpStr;
}

static QString eventViewerFormatJournal( Journal *journal, KDateTime::Spec spec )
{
  if ( !journal ) {
    return QString();
  }

  QString tmpStr;
  if ( !journal->summary().isEmpty() ) {
    tmpStr+= eventViewerAddTag( "h2", journal->richSummary() );
  }
  tmpStr += eventViewerAddTag(
    "h3", i18n( "Journal for %1", IncidenceFormatter::dateToString( journal->dtStart(), false,
                                                                    spec ) ) );
  if ( !journal->description().isEmpty() ) {
    tmpStr += eventViewerAddTag( "p", journal->richDescription() );
  }
  return tmpStr;
}

static QString eventViewerFormatFreeBusy( FreeBusy *fb, KDateTime::Spec spec )
{
  Q_UNUSED( spec );

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
//@endcond

//@cond PRIVATE
class KCal::IncidenceFormatter::EventViewerVisitor
  : public IncidenceBase::Visitor
{
  public:
    EventViewerVisitor()
      : mSpec( KDateTime::Spec() ), mResult( "" ) {}

    bool act( IncidenceBase *incidence, KDateTime::Spec spec=KDateTime::Spec() )
    {
      mSpec = spec;
      mResult = "";
      return incidence->accept( *this );
    }
    QString result() const { return mResult; }

  protected:
    bool visit( Event *event )
    {
      mResult = eventViewerFormatEvent( event, mSpec );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = eventViewerFormatTodo( todo, mSpec );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = eventViewerFormatJournal( journal, mSpec );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = eventViewerFormatFreeBusy( fb, mSpec );
      return !mResult.isEmpty();
    }

  protected:
    KDateTime::Spec mSpec;
    QString mResult;
};
//@endcond

QString IncidenceFormatter::extensiveDisplayString( IncidenceBase *incidence )
{
  return extensiveDisplayStr( incidence, KDateTime::Spec() );
}

QString IncidenceFormatter::extensiveDisplayStr( IncidenceBase *incidence, KDateTime::Spec spec )
{
  if ( !incidence ) {
    return QString();
  }

  EventViewerVisitor v;
  if ( v.act( incidence, spec ) ) {
    return v.result();
  } else {
    return QString();
  }
}

/*******************************************************************
 *  Helper functions for the body part formatter of kmail
 *******************************************************************/

//@cond PRIVATE
static QString string2HTML( const QString &str )
{
  return Qt::escape( str );
}

static QString cleanHtml( const QString &html )
{
  QRegExp rx( "<body[^>]*>(.*)</body>", Qt::CaseInsensitive );
  rx.indexIn( html );
  QString body = rx.cap( 1 );

  return Qt::escape( body.remove( QRegExp( "<[^>]*>" ) ).trimmed() );
}

static QString eventStartTimeStr( Event *event )
{
  QString tmp;
  if ( !event->allDay() ) {
    tmp =  i18nc( "%1: Start Date, %2: Start Time", "%1 %2",
                  IncidenceFormatter::dateToString( event->dtStart(), true,
                                                    KSystemTimeZones::local() ),
                  IncidenceFormatter::timeToString( event->dtStart(), true,
                                                    KSystemTimeZones::local() ) );
  } else {
    tmp = i18nc( "%1: Start Date", "%1 (all day)",
                 IncidenceFormatter::dateToString( event->dtStart(), true,
                                                   KSystemTimeZones::local() ) );
  }
  return tmp;
}

static QString eventEndTimeStr( Event *event )
{
  QString tmp;
  if ( event->hasEndDate() && event->dtEnd().isValid() ) {
    if ( !event->allDay() ) {
      tmp =  i18nc( "%1: End Date, %2: End Time", "%1 %2",
                    IncidenceFormatter::dateToString( event->dtEnd(), true,
                                                      KSystemTimeZones::local() ),
                    IncidenceFormatter::timeToString( event->dtEnd(), true,
                                                      KSystemTimeZones::local() ) );
    } else {
      tmp = i18nc( "%1: End Date", "%1 (all day)",
                   IncidenceFormatter::dateToString( event->dtEnd(), true,
                                                     KSystemTimeZones::local() ) );
    }
  }
  return tmp;
}

static QString invitationRow( const QString &cell1, const QString &cell2 )
{
  return "<tr><td>" + cell1 + "</td><td>" + cell2 + "</td></tr>\n";
}

static bool iamOrganizer( Incidence *incidence )
{
  // Check if I'm the organizer for this incidence

  if ( !incidence ) {
    return false;
  }

  bool iam = false;
  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for ( QStringList::Iterator it=profiles.begin(); it != profiles.end(); ++it ) {
    settings.setProfile( *it );
    if ( settings.getSetting( KEMailSettings::EmailAddress ) == incidence->organizer().email() ) {
      iam = true;
      break;
    }
  }
  return iam;
}

static bool iamAttendee( Attendee *attendee )
{
  // Check if I'm this attendee

  bool iam = false;
  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for ( QStringList::Iterator it=profiles.begin(); it != profiles.end(); ++it ) {
    settings.setProfile( *it );
    if ( settings.getSetting( KEMailSettings::EmailAddress ) == attendee->email() ) {
      iam = true;
      break;
    }
  }
  return iam;
}

static Attendee *findMyAttendee( Incidence *incidence )
{
  // Return the attendee for the incidence that is probably me

  Attendee *attendee = 0;
  if ( !incidence ) {
    return attendee;
  }

  KEMailSettings settings;
  QStringList profiles = settings.profiles();
  for ( QStringList::Iterator it=profiles.begin(); it != profiles.end(); ++it ) {
    settings.setProfile( *it );

    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it2;
    for ( it2 = attendees.constBegin(); it2 != attendees.constEnd(); ++it2 ) {
      Attendee *a = *it2;
      if ( settings.getSetting( KEMailSettings::EmailAddress ) == a->email() ) {
        attendee = a;
        break;
      }
    }
  }
  return attendee;
}

static Attendee *findAttendee( Incidence *incidence, const QString &email )
{
  // Search for an attendee by email address

  Attendee *attendee = 0;
  if ( !incidence ) {
    return attendee;
  }

  Attendee::List attendees = incidence->attendees();
  Attendee::List::ConstIterator it;
  for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
    Attendee *a = *it;
    if ( email == a->email() ) {
      attendee = a;
      break;
    }
  }
  return attendee;
}

static bool rsvpRequested( Incidence *incidence )
{
  //use a heuristic to determine if a response is requested.

  bool rsvp = true; // better send superfluously than not at all
  Attendee::List attendees = incidence->attendees();
  Attendee::List::ConstIterator it;
  for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
    if ( it == attendees.constBegin() ) {
      rsvp = (*it)->RSVP(); // use what the first one has
    } else {
      if ( (*it)->RSVP() != rsvp ) {
        rsvp = true; // they differ, default
        break;
      }
    }
  }
  return rsvp;
}

static QString rsvpRequestedStr( bool rsvpRequested )
{
  if ( rsvpRequested ) {
    return i18n( "Your response is requested" );
  } else {
    return i18n( "A response is not necessary" );
  }
}

static QString invitationPerson( const QString &email, QString name, QString uid )
{
  // Make the search, if there is an email address to search on,
  // and either name or uid is missing
  if ( !email.isEmpty() && ( name.isEmpty() || uid.isEmpty() ) ) {
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList = add_book->findByEmail( email );
    if ( !addressList.isEmpty() ) {
      KABC::Addressee o = addressList.first();
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
  }

  // Show the attendee
  QString tmpString;
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
  if ( !email.isEmpty() ) {
    KCal::Person person( name, email );
    KUrl mailto;
    mailto.setProtocol( "mailto" );
    mailto.setPath( person.fullName() );
    const QString iconPath =
      KIconLoader::global()->iconPath( "mail-message-new", KIconLoader::Small );
    tmpString += eventViewerAddLink( mailto.url(), "<img src=\"" + iconPath + "\">" );
  }
  tmpString += '\n';

  return tmpString;
}

static QString invitationsDetailsIncidence( Incidence *incidence, bool noHtmlMode )
{
  // if description and comment -> use both
  // if description, but no comment -> use the desc as the comment (and no desc)
  // if comment, but no description -> use the comment and no description

  QString html;
  QString descr;
  QStringList comments;

  if ( incidence->comments().isEmpty() ) {
    if ( !incidence->description().isEmpty() ) {
      // use description as comments
      if ( !incidence->descriptionIsRich() ) {
        comments << string2HTML( incidence->description() );
      } else {
        comments << incidence->richDescription();
        if ( noHtmlMode ) {
          comments[0] = cleanHtml( comments[0] );
        }
        comments[0] = eventViewerAddTag( "p", comments[0] );
      }
    }
    //else desc and comments are empty
  } else {
    // non-empty comments
    for ( int i=0; i < incidence->comments().count(); ++i ) {
      comments[i] = string2HTML( incidence->comments()[i] );
    }
    if ( !incidence->description().isEmpty() ) {
      // use description too
      if ( !incidence->descriptionIsRich() ) {
        descr = string2HTML( incidence->description() );
      } else {
        descr = incidence->richDescription();
        if ( noHtmlMode ) {
          descr = cleanHtml( descr );
        }
        descr = eventViewerAddTag( "p", descr );
      }
    }
  }

  if( !descr.isEmpty() ) {
    html += "<p>";
    html += "<table border=\"0\" style=\"margin-top:4px;\">";
    html += "<tr><td><center>" +
            eventViewerAddTag( "u", i18n( "Description:" ) ) +
            "</center></td></tr>";
    html += "<tr><td>" + descr + "</td></tr>";
    html += "</table>";
  }

  if ( !comments.isEmpty() ) {
    html += "<p>";
    html += "<table border=\"0\" style=\"margin-top:4px;\">";
    html += "<tr><td><center>" +
            eventViewerAddTag( "u", i18n( "Comments:" ) ) +
            "</center></td></tr>";
    html += "<tr><td>";
    if ( comments.count() > 1 ) {
      html += "<ul>";
      for ( int i=0; i < comments.count(); ++i ) {
        html += "<li>" + comments[i] + "</li>";
      }
      html += "</ul>";
    } else {
      html += comments[0];
    }
    html += "</td></tr>";
    html += "</table>";
  }
  return html;
}

static QString invitationDetailsEvent( Event *event, bool noHtmlMode, KDateTime::Spec spec )
{
  // Invitation details are formatted into an HTML table
  if ( !event ) {
    return QString();
  }

  QString sSummary = i18n( "Summary unspecified" );
  if ( !event->summary().isEmpty() ) {
    if ( !event->summaryIsRich() ) {
      sSummary = string2HTML( event->summary() );
    } else {
      sSummary = event->richSummary();
      if ( noHtmlMode ) {
        sSummary = cleanHtml( sSummary );
      }
    }
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( !event->location().isEmpty() ) {
    if ( !event->locationIsRich() ) {
      sLocation = string2HTML( event->location() );
    } else {
      sLocation = event->richLocation();
      if ( noHtmlMode ) {
        sLocation = cleanHtml( sLocation );
      }
    }
  }

  QString dir = ( QApplication::isRightToLeft() ? "rtl" : "ltr" );
  QString html = QString( "<div dir=\"%1\">\n" ).arg( dir );
  html += "<table cellspacing=\"4\" style=\"border-width:4px; border-style:groove\">";

  // Invitation summary & location rows
  html += invitationRow( i18n( "What:" ), sSummary );
  html += invitationRow( i18n( "Where:" ), sLocation );

  // If a 1 day event
  if ( event->dtStart().date() == event->dtEnd().date() ) {
    html += invitationRow( i18n( "Date:" ),
                           IncidenceFormatter::dateToString( event->dtStart(), false, spec ) );
    if ( !event->allDay() ) {
      html += invitationRow( i18n( "Time:" ),
                             IncidenceFormatter::timeToString( event->dtStart(), false, spec ) +
                             " - " +
                             IncidenceFormatter::timeToString( event->dtEnd(), false, spec ) );
    }
  } else {
    html += invitationRow( i18nc( "starting date", "From:" ),
                           IncidenceFormatter::dateToString( event->dtStart(), false, spec ) );
    if ( !event->allDay() ) {
      html += invitationRow( i18nc( "starting time", "At:" ),
                             IncidenceFormatter::timeToString( event->dtStart(), false, spec ) );
    }
    if ( event->hasEndDate() ) {
      html += invitationRow( i18nc( "ending date", "To:" ),
                             IncidenceFormatter::dateToString( event->dtEnd(), false, spec ) );
      if ( !event->allDay() ) {
        html += invitationRow( i18nc( "ending time", "At:" ),
                               IncidenceFormatter::timeToString( event->dtEnd(), false, spec ) );
      }
    } else {
      html += invitationRow( i18nc( "ending date", "To:" ),
                             i18n( "no end date specified" ) );
    }
  }

  // Invitation Duration Row
  if ( !event->allDay() && event->hasEndDate() && event->dtEnd().isValid() ) {
    QString tmp;
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

  if ( event->recurs() ) {
    html += invitationRow( i18n( "Recurrence:" ), IncidenceFormatter::recurrenceString( event ) );
  }

  html += "</table></div>\n";
  html += invitationsDetailsIncidence( event, noHtmlMode );

  return html;
}

static QString invitationDetailsTodo( Todo *todo, bool noHtmlMode, KDateTime::Spec spec )
{
  // To-do details are formatted into an HTML table
  if ( !todo ) {
    return QString();
  }

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( ! todo->summary().isEmpty() ) {
    sSummary = todo->richSummary();
    if ( noHtmlMode ) {
      sSummary = cleanHtml( sSummary );
    }
  }
  if ( ! todo->description().isEmpty() ) {
    sDescr = todo->description();
    if ( noHtmlMode ) {
      sDescr = cleanHtml( sDescr );
    }
  }
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Summary:" ), sSummary );
  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    html += invitationRow( i18n( "Start Date:" ),
                           IncidenceFormatter::dateToString( todo->dtStart(),
                                                             false,
                                                             spec ) );
  } else {
    html += invitationRow( i18n( "Start Date:" ),
                           i18nc( "no to-do start date", "None" ) );
  }
  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    html += invitationRow( i18n( "Due Date:" ),
                           IncidenceFormatter::dateToString( todo->dtDue(),
                                                             false,
                                                             spec ) );
  } else {
    html += invitationRow( i18n( "Due Date:" ),
                           i18nc( "no to-do due date", "None" ) );
  }
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";
  html += invitationsDetailsIncidence( todo, noHtmlMode );

  return html;
}

static QString invitationDetailsJournal( Journal *journal, bool noHtmlMode, KDateTime::Spec spec )
{
  if ( !journal ) {
    return QString();
  }

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( ! journal->summary().isEmpty() ) {
    sSummary = journal->richSummary();
    if ( noHtmlMode ) {
      sSummary = cleanHtml( sSummary );
    }
  }
  if ( ! journal->description().isEmpty() ) {
    sDescr = journal->richDescription();
    if ( noHtmlMode ) {
      sDescr = cleanHtml( sDescr );
    }
  }
  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Summary:" ), sSummary );
  html += invitationRow( i18n( "Date:" ),
                         IncidenceFormatter::dateToString( journal->dtStart(),
                                                           false,
                                                           spec ) );
  html += invitationRow( i18n( "Description:" ), sDescr );
  html += "</table>\n";
  html += invitationsDetailsIncidence( journal, noHtmlMode );

  return html;
}

static QString invitationDetailsFreeBusy( FreeBusy *fb, bool noHtmlMode, KDateTime::Spec spec )
{
  Q_UNUSED( noHtmlMode );

  if ( !fb ) {
    return QString();
  }

  QString html( "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n" );
  html += invitationRow( i18n( "Person:" ), fb->organizer().fullName() );
  html += invitationRow( i18n( "Start date:" ),
                         IncidenceFormatter::dateToString( fb->dtStart(),
                                                           true, spec ) );
  html += invitationRow( i18n( "End date:" ),
                         IncidenceFormatter::dateToString( fb->dtEnd(),
                                                           true, spec ) );

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
      return i18n( "This invitation has been updated" );
    }
    if ( iamOrganizer( event ) ) {
      return i18n( "I sent this invitation" );
    } else {
      if ( !event->organizer().fullName().isEmpty() ) {
        return i18n( "You received an invitation from %1",
                     event->organizer().fullName() );
      } else {
        return i18n( "You received an invitation" );
      }
    }
  case iTIPRefresh:
    return i18n( "This invitation was refreshed" );
  case iTIPCancel:
    return i18n( "This invitation has been canceled" );
  case iTIPAdd:
    return i18n( "Addition to the invitation" );
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
        return i18n( "%1 accepts this invitation", attendeeName );
      } else {
        return i18n( "%1 accepts this invitation on behalf of %2",
                     attendeeName, delegatorName );
      }
    case Attendee::Tentative:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 tentatively accepts this invitation", attendeeName );
      } else {
        return i18n( "%1 tentatively accepts this invitation on behalf of %2",
                     attendeeName, delegatorName );
      }
    case Attendee::Declined:
      if ( delegatorName.isEmpty() ) {
        return i18n( "%1 declines this invitation", attendeeName );
      } else {
        return i18n( "%1 declines this invitation on behalf of %2",
                     attendeeName, delegatorName );
      }
    case Attendee::Delegated:
    {
      QString delegate, dummy;
      KPIMUtils::extractEmailAddressAndName( attendee->delegate(), dummy, delegate );
      if ( delegate.isEmpty() ) {
        delegate = attendee->delegate();
      }
      if ( !delegate.isEmpty() ) {
        return i18n( "%1 has delegated this invitation to %2", attendeeName, delegate );
      } else {
        return i18n( "%1 has delegated this invitation", attendeeName );
      }
    }
    case Attendee::Completed:
      return i18n( "This invitation is now completed" );
    case Attendee::InProcess:
      return i18n( "%1 is still processing the invitation", attendeeName );
    default:
      return i18n( "Unknown response to this invitation" );
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
      } else {
        return i18n( "Sender has delegated this request for the to-do " );
      }
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
      kDebug() << "Warning: attendeecount in the reply should be 1 "
               << "but is " << attendees.count();
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
//@endcond

static QString invitationAttendees( Incidence *incidence )
{
  QString tmpStr;
  if ( !incidence ) {
    return tmpStr;
  }

  tmpStr += i18n( "Invitation List" );

  int count=0;
  Attendee::List attendees = incidence->attendees();
  if ( !attendees.isEmpty() ) {

    Attendee::List::ConstIterator it;
    for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
      Attendee *a = *it;
      if ( !iamAttendee( a ) ) {
        count++;
        if ( count == 1 ) {
          tmpStr += "<table border=\"1\" cellpadding=\"1\" cellspacing=\"0\">";
        }
        tmpStr += "<tr>";
        tmpStr += "<td>";
        tmpStr += invitationPerson( a->email(), a->name(), QString() );
        if ( !a->delegator().isEmpty() ) {
          tmpStr += i18n( " (delegated by %1)", a->delegator() );
        }
        if ( !a->delegate().isEmpty() ) {
          tmpStr += i18n( " (delegated to %1)", a->delegate() );
        }
        tmpStr += "</td>";
        tmpStr += "<td>" + a->statusStr() + "</td>";
        tmpStr += "</tr>";
      }
    }
  }
  if ( count ) {
    tmpStr += "</table>";
  } else {
    tmpStr += "<i>" + i18nc( "no attendees", "None" ) + "</i>";
  }

  return tmpStr;
}

//@cond PRIVATE
class KCal::IncidenceFormatter::ScheduleMessageVisitor
  : public IncidenceBase::Visitor
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
  public:
    InvitationBodyVisitor( bool noHtmlMode, KDateTime::Spec spec )
      : ScheduleMessageVisitor(), mNoHtmlMode( noHtmlMode ), mSpec( spec ) {}

  protected:
    bool visit( Event *event )
    {
      mResult = invitationDetailsEvent( event, mNoHtmlMode, mSpec );
      return !mResult.isEmpty();
    }
    bool visit( Todo *todo )
    {
      mResult = invitationDetailsTodo( todo, mNoHtmlMode, mSpec );
      return !mResult.isEmpty();
    }
    bool visit( Journal *journal )
    {
      mResult = invitationDetailsJournal( journal, mNoHtmlMode, mSpec );
      return !mResult.isEmpty();
    }
    bool visit( FreeBusy *fb )
    {
      mResult = invitationDetailsFreeBusy( fb, mNoHtmlMode, mSpec );
      return !mResult.isEmpty();
    }

  private:
    bool mNoHtmlMode;
    KDateTime::Spec mSpec;
};
//@endcond

QString InvitationFormatterHelper::generateLinkURL( const QString &id )
{
  return id;
}

//@cond PRIVATE
class IncidenceFormatter::IncidenceCompareVisitor
  : public IncidenceBase::Visitor
{
  public:
    IncidenceCompareVisitor() : mExistingIncidence( 0 ) {}
    bool act( IncidenceBase *incidence, Incidence *existingIncidence )
    {
      if ( !existingIncidence ) {
        return false;
      }
      Incidence *inc = dynamic_cast<Incidence *>( incidence );
      if ( !inc || !existingIncidence || inc->revision() <= existingIncidence->revision() ) {
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
        mChanges += i18n( "The invitation starting time has been changed from %1 to %2",
                          eventStartTimeStr( oldEvent ), eventStartTimeStr( newEvent ) );
      }
      if ( oldEvent->dtEnd() != newEvent->dtEnd() ||
           oldEvent->allDay() != newEvent->allDay() ) {
        mChanges += i18n( "The invitation ending time has been changed from %1 to %2",
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
//@endcond

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

static QString formatICalInvitationHelper( QString invitation,
                                           Calendar *mCalendar,
                                           InvitationFormatterHelper *helper,
                                           bool noHtmlMode,
                                           KDateTime::Spec spec )
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
  incBase->shiftTimes( mCalendar->timeSpec(), KDateTime::Spec::LocalZone() );

  // Determine if this incidence is in my calendar
  Incidence *existingIncidence = 0;
  if ( incBase && helper->calendar() ) {
    existingIncidence = helper->calendar()->incidence( incBase->uid() );
    if ( !existingIncidence ) {
      const Incidence::List list = helper->calendar()->incidences();
      for ( Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it ) {
        if ( (*it)->schedulingID() == incBase->uid() ) {
          existingIncidence = *it;
          break;
        }
      }
    }
  }

  // First make the text of the message
  QString html;
  html += "<div align=\"center\" style=\"border:solid 1px;\">";

  IncidenceFormatter::InvitationHeaderVisitor headerVisitor;
  // The InvitationHeaderVisitor returns false if the incidence is somehow invalid, or not handled
  if ( !headerVisitor.act( incBase, msg ) ) {
    return QString();
  }
  html += eventViewerAddTag( "h3", headerVisitor.result() );

  IncidenceFormatter::InvitationBodyVisitor bodyVisitor( noHtmlMode, spec );
  if ( !bodyVisitor.act( incBase, msg ) ) {
    return QString();
  }
  html += bodyVisitor.result();

  if ( msg->method() == iTIPRequest ) { // ### Scheduler::Publish/Refresh/Add as well?
    IncidenceFormatter::IncidenceCompareVisitor compareVisitor;
    if ( compareVisitor.act( incBase, existingIncidence ) ) {
      html +=
        i18n( "<p align=\"left\">The following changes have been made by the organizer:</p>" );
      html += compareVisitor.result();
    }
  }

  Incidence *inc = dynamic_cast<Incidence*>( incBase );

  // determine if I am the organizer for this invitation
  bool myInc = iamOrganizer( inc );

  // determine if the invitation response has already been recorded
  bool rsvpRec = false;
  Attendee *ea = 0;
  if ( !myInc ) {
    if ( existingIncidence ) {
      ea = findMyAttendee( existingIncidence );
    }
    if ( ea && ( ea->status() == Attendee::Accepted || ea->status() == Attendee::Declined ) ) {
      rsvpRec = true;
    }
  }

  // Print if RSVP needed, not-needed, or response already recorded
  bool rsvpReq = rsvpRequested( inc );
  if ( !myInc ) {
    html += "<br/>";
    html += "<i><u>";
    if ( rsvpRec && ( inc && inc->revision() == 0 ) ) {
      html += i18n( "Your response has already been recorded [%1]", ea->statusStr() );
      rsvpReq = false;
    } else {
      html += rsvpRequestedStr( rsvpReq );
    }
    html += "</u></i><br>";
  }

  // Add groupware links

  html += "<p>";
  html += "<table border=\"0\" align=\"center\" cellspacing=\"4\"><tr>";

  const QString tdOpen = "<td style=\"border-width:2px;border-style:outset\">";
  const QString tdClose = "</td>";
  switch ( msg->method() ) {
    case iTIPPublish:
    case iTIPRequest:
    case iTIPRefresh:
    case iTIPAdd:
    {
      if ( inc && inc->revision() > 0 && existingIncidence ) {
        if ( inc->type() == "Todo" ) {
          html += helper->makeLink( "reply", i18n( "[Record invitation into my to-do list]" ) );
        } else {
          html += helper->makeLink( "reply", i18n( "[Record invitation into my calendar]" ) );
        }
      }

      if ( !myInc ) {
        if ( rsvpReq ) {
          // Accept
          html += tdOpen;
          html += helper->makeLink( "accept",
                                    i18nc( "accept invitation",
                                           "Accept" ) );
          html += tdClose;

          // Accept conditionally
          html += tdOpen;
          html += helper->makeLink( "accept_conditionally",
                                    i18nc( "Accept invitation conditionally",
                                           "Accept cond." ) );
          html += tdClose;
        }

        if ( rsvpReq ) {
          // Counter proposal
          html += tdOpen;
          html += helper->makeLink( "counter",
                                    i18nc( "invitation counter proposal",
                                           "Counter proposal" ) );
          html += tdClose;
        }

        if ( rsvpReq ) {
          // Decline
          html += tdOpen;
          html += helper->makeLink( "decline",
                                    i18nc( "decline invitation",
                                           "Decline" ) );
          html += tdClose;
        }

        if ( !rsvpRec || ( inc && inc->revision() > 0 ) ) {
          // Delegate
          html += tdOpen;
          html += helper->makeLink( "delegate",
                                    i18nc( "delegate inviation to another",
                                           "Delegate" ) );
          html += tdClose;

          // Forward
          html += tdOpen;
          html += helper->makeLink( "forward",
                                    i18nc( "forward request to another",
                                           "Forward" ) );
          html += tdClose;

          // Check calendar
          if ( incBase->type() == "Event" ) {
            html += tdOpen;
            html += helper->makeLink( "check_calendar",
                                      i18nc( "look for scheduling conflicts",
                                             "Check my calendar" ) );
            html += tdClose;
          }
        }
      }
      break;
    }

    case iTIPCancel:
      // Remove invitation
      if ( existingIncidence ) {
        html += tdOpen;
        if ( inc->type() == "Todo" ) {
          html += helper->makeLink( "cancel",
                                    i18n( "Remove invitation from my task list" ) );
        } else {
          html += helper->makeLink( "cancel",
                                    i18n( "Remove invitation from my calendar" ) );
        }
        html += tdClose;
      }
      break;

    case iTIPReply:
    {
      // Record invitation response
      Attendee *a = 0;
      Attendee *ea = 0;
      if ( inc ) {
        a = inc->attendees().first();
        if ( a && helper->calendar() ) {
          ea = findAttendee( existingIncidence, a->email() );
        }
      }
      if ( ea && ( ea->status() != Attendee::NeedsAction ) && ( ea->status() == a->status() ) ) {
        html += tdOpen;
        html += eventViewerAddTag( "i", i18n( "The response has already been recorded" ) );
        html += tdClose;
      } else {
        if ( inc->type() == "Todo" ) {
          html += helper->makeLink( "reply", i18n( "[Record response into my to-do list]" ) );
        } else {
          html += helper->makeLink( "reply", i18n( "[Record response into my calendar]" ) );
        }
      }
      break;
    }

    case iTIPCounter:
      // Counter proposal
      html += tdOpen;
      html += helper->makeLink( "accept_counter", i18n( "Accept" ) );
      html += tdClose;

      html += tdOpen;
      html += helper->makeLink( "decline_counter", i18n( "Decline" ) );
      html += tdClose;

      html += tdOpen;
      html += helper->makeLink( "check_calendar", i18n( "Check my calendar" ) );
      html += tdClose;
      break;

    case iTIPDeclineCounter:
    case iTIPNoMethod:
      break;
  }

  // close the groupware table
  html += "</tr></table>";

  // Add the attendee list if I am the organizer
  if ( myInc && helper->calendar() ) {
    html += invitationAttendees( helper->calendar()->incidence( inc->uid() ) );
  }

  // close the top-level
  html += "</div>";
  return html;
}
//@endcond

QString IncidenceFormatter::formatICalInvitation( QString invitation,
                                                  Calendar *mCalendar,
                                                  InvitationFormatterHelper *helper )
{
  return formatICalInvitationHelper( invitation, mCalendar, helper, false,
                                     KSystemTimeZones::local() );
}

QString IncidenceFormatter::formatICalInvitationNoHtml( QString invitation,
                                                        Calendar *mCalendar,
                                                        InvitationFormatterHelper *helper )
{
  return formatICalInvitationHelper( invitation, mCalendar, helper, true,
                                     KSystemTimeZones::local() );
}

/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

//@cond PRIVATE
class KCal::IncidenceFormatter::ToolTipVisitor
  : public IncidenceBase::Visitor
{
  public:
    ToolTipVisitor()
      : mRichText( true ), mSpec( KDateTime::Spec() ), mResult( "" ) {}

    bool act( IncidenceBase *incidence, bool richText=true, KDateTime::Spec spec=KDateTime::Spec() )
    {
      mRichText = richText;
      mSpec = spec;
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
    KDateTime::Spec mSpec;
    QString mResult;
};

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Event *event )
{
  //FIXME: support mRichText==false
  QString ret;
  QString tmp;
  if ( event->isMultiDay() ) {

    tmp = IncidenceFormatter::dateToString( event->dtStart(), true, mSpec );
    ret += "<br>" + i18nc( "Event start", "<i>From:</i> %1", tmp );

    tmp = IncidenceFormatter::dateToString( event->dtEnd(), true, mSpec );
    ret += "<br>" + i18nc( "Event end","<i>To:</i> %1", tmp );

  } else {

    ret += "<br>" +
           i18n( "<i>Date:</i> %1",
                 IncidenceFormatter::dateToString( event->dtStart(), true, mSpec ) );
    if ( !event->allDay() ) {
      const QString dtStartTime = IncidenceFormatter::timeToString( event->dtStart(), true, mSpec );
      const QString dtEndTime = IncidenceFormatter::timeToString( event->dtEnd(), true, mSpec );
      if ( dtStartTime == dtEndTime ) {
        // to prevent 'Time: 17:00 - 17:00'
        tmp = "<br>" +
              i18nc( "time for event", "<i>Time:</i> %1",
                     dtStartTime );
      } else {
        tmp = "<br>" +
              i18nc( "time range for event",
                     "<i>Time:</i> %1 - %2",
                     dtStartTime, dtEndTime );
      }
      ret += tmp;
    }
  }
  return ret.replace( ' ', "&nbsp;" );
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Todo *todo )
{
  //FIXME: support mRichText==false
  QString ret;
  if ( todo->hasStartDate() && todo->dtStart().isValid() ) {
    // No need to add <i> here. This is separated issue and each line
    // is very visible on its own. On the other hand... Yes, I like it
    // italics here :)
    ret += "<br>" + i18n( "<i>Start:</i> %1",
                          IncidenceFormatter::dateToString( todo->dtStart( false ), true, mSpec ) );
  }
  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    ret += "<br>" + i18n( "<i>Due:</i> %1",
                          IncidenceFormatter::dateTimeToString( todo->dtDue(),
                                                                todo->allDay(),
                                                                true, mSpec ) );
  }
  if ( todo->isCompleted() ) {
    ret += "<br>" +
           i18n( "<i>Completed:</i> %1", todo->completedStr() );
  } else {
    ret += "<br>" +
           i18nc( "percent complete", "%1 % completed", todo->percentComplete() );
  }

  return ret.replace( ' ', "&nbsp;" );
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( Journal *journal )
{
  //FIXME: support mRichText==false
  QString ret;
  if ( journal->dtStart().isValid() ) {
    ret += "<br>" +
           i18n( "<i>Date:</i> %1",
                 IncidenceFormatter::dateToString( journal->dtStart(), false, mSpec ) );
  }
  return ret.replace( ' ', "&nbsp;" );
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText( FreeBusy *fb )
{
  //FIXME: support mRichText==false
  QString ret;
  ret = "<br>" +
        i18n( "<i>Period start:</i> %1",
              KGlobal::locale()->formatDateTime( fb->dtStart().dateTime() ) );
  ret += "<br>" +
         i18n( "<i>Period start:</i> %1",
               KGlobal::locale()->formatDateTime( fb->dtEnd().dateTime() ) );
  return ret.replace( ' ', "&nbsp;" );
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
           i18n( "<i>Location:</i> %1", incidence->richLocation() );
  }

  if ( !incidence->description().isEmpty() ) {
    QString desc( incidence->description() );
    if ( !incidence->descriptionIsRich() ) {
      if ( desc.length() > 120 ) {
        desc = desc.left( 120 ) + "...";
      }
      desc = Qt::escape( desc ).replace( '\n', "<br>" );
    } else {
      // TODO: truncate the description when it's rich text
    }
    tmp += "<br>----------<br>" + i18n( "<i>Description:</i>" ) + "<br>" + desc;
  }
  tmp += "</qt>";
  return tmp;
}
//@endcond

QString IncidenceFormatter::toolTipString( IncidenceBase *incidence,
                                           bool richText )
{
  return toolTipStr( incidence, richText, KDateTime::Spec() );
}

QString IncidenceFormatter::toolTipStr( IncidenceBase *incidence,
                                        bool richText, KDateTime::Spec spec )
{
  ToolTipVisitor v;
  if ( v.act( incidence, richText, spec ) ) {
    return v.result();
  } else {
    return QString();
  }
}

/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

//@cond PRIVATE
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
//@endcond

//@cond PRIVATE
class KCal::IncidenceFormatter::MailBodyVisitor
  : public IncidenceBase::Visitor
{
  public:
    MailBodyVisitor()
      : mSpec( KDateTime::Spec() ), mResult( "" ) {}

    bool act( IncidenceBase *incidence, KDateTime::Spec spec=KDateTime::Spec() )
    {
      mSpec = spec;
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
    KDateTime::Spec mSpec;
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
  mResult += i18n( "Start Date: %1\n", IncidenceFormatter::dateToString( event->dtStart(), true,
                                                                         mSpec ) );
  if ( !event->allDay() ) {
    mResult += i18n( "Start Time: %1\n", IncidenceFormatter::timeToString( event->dtStart(),
                                                                           true, mSpec ) );
  }
  if ( event->dtStart() != event->dtEnd() ) {
    mResult += i18n( "End Date: %1\n", IncidenceFormatter::dateToString( event->dtEnd(), true,
                                                                         mSpec ) );
  }
  if ( !event->allDay() ) {
    mResult += i18n( "End Time: %1\n", IncidenceFormatter::timeToString( event->dtEnd(), true,
                                                                         mSpec ) );
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
                     IncidenceFormatter::dateToString( todo->dtStart(false), true, mSpec ) );
    if ( !todo->allDay() ) {
      mResult += i18n( "Start Time: %1\n",
                       IncidenceFormatter::timeToString( todo->dtStart(false), true, mSpec ) );
    }
  }
  if ( todo->hasDueDate() && todo->dtDue().isValid() ) {
    mResult += i18n( "Due Date: %1\n",
                     IncidenceFormatter::dateToString( todo->dtDue(), true, mSpec ) );
    if ( !todo->allDay() ) {
      mResult += i18n( "Due Time: %1\n",
                       IncidenceFormatter::timeToString( todo->dtDue(), true, mSpec ) );
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
  mResult += i18n( "Date: %1\n",
                   IncidenceFormatter::dateToString( journal->dtStart(), true, mSpec ) );
  if ( !journal->allDay() ) {
    mResult += i18n( "Time: %1\n",
                     IncidenceFormatter::timeToString( journal->dtStart(), true, mSpec ) );
  }
  if ( !journal->description().isEmpty() ) {
    mResult += i18n( "Text of the journal:\n%1\n", journal->richDescription() );
  }
  return !mResult.isEmpty();
}
//@endcond

QString IncidenceFormatter::mailBodyString( IncidenceBase *incidence )
{
  return mailBodyStr( incidence, KDateTime::Spec() );
}

QString IncidenceFormatter::mailBodyStr( IncidenceBase *incidence,
                                         KDateTime::Spec spec )
{
  if ( !incidence ) {
    return QString();
  }

  MailBodyVisitor v;
  if ( v.act( incidence, spec ) ) {
    return v.result();
  }
  return QString();
}

//@cond PRIVATE
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
//@endcond

QString IncidenceFormatter::recurrenceString( Incidence *incidence )
{
  if ( !incidence->recurs() ) {
    return i18n( "No recurrence" );
  }
  QStringList dayList;
  dayList.append( i18n( "31st Last" ) );
  dayList.append( i18n( "30th Last" ) );
  dayList.append( i18n( "29th Last" ) );
  dayList.append( i18n( "28th Last" ) );
  dayList.append( i18n( "27th Last" ) );
  dayList.append( i18n( "26th Last" ) );
  dayList.append( i18n( "25th Last" ) );
  dayList.append( i18n( "24th Last" ) );
  dayList.append( i18n( "23rd Last" ) );
  dayList.append( i18n( "22nd Last" ) );
  dayList.append( i18n( "21st Last" ) );
  dayList.append( i18n( "20th Last" ) );
  dayList.append( i18n( "19th Last" ) );
  dayList.append( i18n( "18th Last" ) );
  dayList.append( i18n( "17th Last" ) );
  dayList.append( i18n( "16th Last" ) );
  dayList.append( i18n( "15th Last" ) );
  dayList.append( i18n( "14th Last" ) );
  dayList.append( i18n( "13th Last" ) );
  dayList.append( i18n( "12th Last" ) );
  dayList.append( i18n( "11th Last" ) );
  dayList.append( i18n( "10th Last" ) );
  dayList.append( i18n( "9th Last" ) );
  dayList.append( i18n( "8th Last" ) );
  dayList.append( i18n( "7th Last" ) );
  dayList.append( i18n( "6th Last" ) );
  dayList.append( i18n( "5th Last" ) );
  dayList.append( i18n( "4th Last" ) );
  dayList.append( i18n( "3rd Last" ) );
  dayList.append( i18n( "2nd Last" ) );
  dayList.append( i18nc( "last day of the month", "Last" ) );
  dayList.append( i18nc( "unknown day of the month", "unknown" ) ); //#31 - zero offset from UI
  dayList.append( i18n( "1st" ) );
  dayList.append( i18n( "2nd" ) );
  dayList.append( i18n( "3rd" ) );
  dayList.append( i18n( "4th" ) );
  dayList.append( i18n( "5th" ) );
  dayList.append( i18n( "6th" ) );
  dayList.append( i18n( "7th" ) );
  dayList.append( i18n( "8th" ) );
  dayList.append( i18n( "9th" ) );
  dayList.append( i18n( "10th" ) );
  dayList.append( i18n( "11th" ) );
  dayList.append( i18n( "12th" ) );
  dayList.append( i18n( "13th" ) );
  dayList.append( i18n( "14th" ) );
  dayList.append( i18n( "15th" ) );
  dayList.append( i18n( "16th" ) );
  dayList.append( i18n( "17th" ) );
  dayList.append( i18n( "18th" ) );
  dayList.append( i18n( "19th" ) );
  dayList.append( i18n( "20th" ) );
  dayList.append( i18n( "21st" ) );
  dayList.append( i18n( "22nd" ) );
  dayList.append( i18n( "23rd" ) );
  dayList.append( i18n( "24th" ) );
  dayList.append( i18n( "25th" ) );
  dayList.append( i18n( "26th" ) );
  dayList.append( i18n( "27th" ) );
  dayList.append( i18n( "28th" ) );
  dayList.append( i18n( "29th" ) );
  dayList.append( i18n( "30th" ) );
  dayList.append( i18n( "31st" ) );
  int weekStart = KGlobal::locale()->weekStartDay();
  QString dayNames;
  QString txt;
  const KCalendarSystem *calSys = KGlobal::locale()->calendar();
  Recurrence *recur = incidence->recurrence();
  switch ( recur->recurrenceType() ) {
  case Recurrence::rNone:
    return i18n( "No recurrence" );
  case Recurrence::rMinutely:
    if ( recur->duration() != -1 ) {
      txt = i18np( "Recurs every minute until %2",
                   "Recurs every %1 minutes until %2",
                   recur->frequency(), recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18np( "Recurs every minute",
                  "Recurs every %1 minutes", recur->frequency() );
  case Recurrence::rHourly:
    if ( recur->duration() != -1 ) {
      txt = i18np( "Recurs hourly until %2",
                   "Recurs every %1 hours until %2",
                   recur->frequency(), recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18np( "Recurs hourly", "Recurs every %1 hours", recur->frequency() );
  case Recurrence::rDaily:
    if ( recur->duration() != -1 ) {
      txt = i18np( "Recurs daily until %2",
                   "Recurs every %1 days until %2",
                   recur->frequency(), recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18np( "Recurs daily", "Recurs every %1 days", recur->frequency() );
  case Recurrence::rWeekly:
  {
    bool addSpace = false;
    for ( int i = 0; i < 7; ++i ) {
      if ( recur->days().testBit( ( i + weekStart + 6 ) % 7 ) ) {
        if ( addSpace ) {
          dayNames.append( i18nc( "separator for list of days", ", " ) );
        }
        dayNames.append( calSys->weekDayName( ( ( i + weekStart + 6 ) % 7 ) + 1,
                                              KCalendarSystem::ShortDayName ) );
        addSpace = true;
      }
    }
    if ( dayNames.isEmpty() ) {
      dayNames = i18nc( "Recurs weekly on no days", "no days" );
    }
    if ( recur->duration() != -1 ) {
      txt = i18ncp( "Recurs weekly on [list of days] until end-date",
                    "Recurs weekly on %2 until %3",
                    "Recurs every <numid>%1</numid> weeks on %2 until %3",
                    recur->frequency(), dayNames, recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18ncp( "Recurs weekly on [list of days]",
                   "Recurs weekly on %2",
                   "Recurs every <numid>%1</numid> weeks on %2",
                   recur->frequency(), dayNames );
  }
  case Recurrence::rMonthlyPos:
  {
    KCal::RecurrenceRule::WDayPos rule = recur->monthPositions()[0];
    if ( recur->duration() != -1 ) {
      txt = i18ncp( "Recurs every N months on the [2nd|3rd|...]"
                    " weekdayname until end-date",
                    "Recurs every month on the %2 %3 until %4",
                    "Recurs every <numid>%1</numid> months on the %2 %3 until %4",
                    recur->frequency(),
                    dayList[rule.pos() + 31],
                    calSys->weekDayName( rule.day(),KCalendarSystem::LongDayName ),
                    recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18ncp( "Recurs every N months on the [2nd|3rd|...] weekdayname",
                   "Recurs every month on the %2 %3",
                   "Recurs every %1 months on the %2 %3",
                   recur->frequency(),
                   dayList[rule.pos() + 31],
                   calSys->weekDayName( rule.day(), KCalendarSystem::LongDayName ) );
  }
  case Recurrence::rMonthlyDay:
  {
    int days = recur->monthDays()[0];
    if ( recur->duration() != -1 ) {
        txt = i18ncp( "Recurs monthly on the [1st|2nd|...] day until end-date",
                      "Recurs monthly on the %2 day until %3",
                      "Recurs every %1 months on the %2 day until %3",
                      recur->frequency(),
                      dayList[days + 31],
                      recurEnd( incidence ) );
        if ( recur->duration() >  0 ) {
          txt += i18nc( "number of occurrences",
                        " (<numid>%1</numid> occurrences)",
                        recur->duration() );
        }
        return txt;
    }
    return i18ncp( "Recurs monthly on the [1st|2nd|...] day",
                   "Recurs monthly on the %2 day",
                   "Recurs every <numid>%1</numid> month on the %2 day",
                   recur->frequency(),
                   dayList[days + 31] );
  }
  case Recurrence::rYearlyMonth:
  {
    if ( recur->duration() != -1 ) {
      txt = i18ncp( "Recurs Every N years on month-name [1st|2nd|...]"
                    " until end-date",
                    "Recurs yearly on %2 %3 until %4",
                    "Recurs every %1 years on %2 %3 until %4",
                    recur->frequency(),
                    calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ),
                    dayList[ recur->yearDates()[0] + 31 ],
                    recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    if ( !recur->yearDates().isEmpty() ) {
      return i18ncp( "Recurs Every N years on month-name [1st|2nd|...]",
                     "Recurs yearly on %2 %3",
                     "Recurs every %1 years on %2 %3",
                     recur->frequency(),
                     calSys->monthName( recur->yearMonths()[0],
                                        recur->startDate().year() ),
                     dayList[ recur->yearDates()[0] + 31 ] );
    } else {
      if (!recur->yearMonths().isEmpty() ) {
        return i18nc( "Recurs Every year on month-name [1st|2nd|...]",
                      "Recurs yearly on %1 %2",
                      calSys->monthName( recur->yearMonths()[0],
                                         recur->startDate().year() ),
                      dayList[ recur->startDate().day() + 31 ] );
      } else {
        return i18nc( "Recurs Every year on month-name [1st|2nd|...]",
                      "Recurs yearly on %1 %2",
                      calSys->monthName( recur->startDate().month(),
                                         recur->startDate().year() ),
                      dayList[ recur->startDate().day() + 31 ] );
      }
    }
  }
  case Recurrence::rYearlyDay:
    if ( recur->duration() != -1 ) {
      txt = i18ncp( "Recurs every N years on day N until end-date",
                    "Recurs every year on day <numid>%2</numid> until %3",
                    "Recurs every <numid>%1</numid> years"
                    " on day <numid>%2</numid> until %3",
                    recur->frequency(),
                    recur->yearDays()[0],
                    recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18ncp( "Recurs every N YEAR[S] on day N",
                   "Recurs every year on day <numid>%2</numid>",
                   "Recurs every <numid>%1</numid> years"
                   " on day <numid>%2</numid>",
                   recur->frequency(), recur->yearDays()[0] );
  case Recurrence::rYearlyPos:
  {
    KCal::RecurrenceRule::WDayPos rule = recur->yearPositions()[0];
    if ( recur->duration() != -1 ) {
      txt = i18ncp( "Every N years on the [2nd|3rd|...] weekdayname "
                    "of monthname until end-date",
                    "Every year on the %2 %3 of %4 until %5",
                    "Every <numid>%1</numid> years on the %2 %3 of %4"
                    " until %5",
                    recur->frequency(),
                    dayList[rule.pos() + 31],
                    calSys->weekDayName( rule.day(), KCalendarSystem::LongDayName ),
                    calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ),
                    recurEnd( incidence ) );
      if ( recur->duration() >  0 ) {
        txt += i18nc( "number of occurrences",
                      " (<numid>%1</numid> occurrences)",
                      recur->duration() );
      }
      return txt;
    }
    return i18ncp( "Every N years on the [2nd|3rd|...] weekdayname "
                   "of monthname",
                   "Every year on the %2 %3 of %4",
                   "Every <numid>%1</numid> years on the %2 %3 of %4",
                   recur->frequency(),
                   dayList[rule.pos() + 31],
                   calSys->weekDayName( rule.day(), KCalendarSystem::LongDayName ),
                   calSys->monthName( recur->yearMonths()[0], recur->startDate().year() ) );
  }
  default:
    return i18n( "Incidence recurs" );
  }
}

QString IncidenceFormatter::timeToString( const KDateTime &date,
                                          bool shortfmt,
                                          const KDateTime::Spec &spec )
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatTime( date.toTimeSpec( spec ).time(), !shortfmt ) + timeZone;
  } else {
    return KGlobal::locale()->formatTime( date.time(), !shortfmt );
  }
}

QString IncidenceFormatter::dateToString( const KDateTime &date,
                                          bool shortfmt,
                                          const KDateTime::Spec &spec )
{
  if ( spec.isValid() ) {

    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return
      KGlobal::locale()->formatDate( date.toTimeSpec( spec ).date(),
                                     ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) ) +
      timeZone;
  } else {
    return
      KGlobal::locale()->formatDate( date.date(),
                                     ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
}

QString IncidenceFormatter::dateTimeToString( const KDateTime &date,
                                              bool allDay,
                                              bool shortfmt,
                                              const KDateTime::Spec &spec )
{
  if ( allDay ) {
    return dateToString( date, shortfmt, spec );
  }

  if ( spec.isValid() ) {
    QString timeZone;
    if ( spec.timeZone() != KSystemTimeZones::local() ) {
      timeZone = ' ' + spec.timeZone().name();
    }

    return KGlobal::locale()->formatDateTime(
      date.toTimeSpec( spec ).dateTime(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) ) + timeZone;
  } else {
    return  KGlobal::locale()->formatDateTime(
      date.dateTime(),
      ( shortfmt ? KLocale::ShortDate : KLocale::LongDate ) );
  }
}
