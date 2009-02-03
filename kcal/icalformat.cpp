/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
  This file is part of the API for handling calendar data and
  defines the ICalFormat class.

  @brief
  iCalendar format implementation.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "icalformat.h"
#include "icalformat_p.h"
#include "calendar.h"
#include "calendarlocal.h"
#include "icaltimezones.h"

extern "C" {
  #include <ical.h>
  #include <icalss.h>
  #include <icalparser.h>
  #include <icalrestriction.h>
  #include <icalmemory.h>
}

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QByteArray>
#include <QtGui/QClipboard>

#include <kdebug.h>
#include <klocale.h>
#include <ksavefile.h>

#include <stdio.h>

using namespace KCal;

//@cond PRIVATE
class KCal::ICalFormat::Private
{
  public:
    Private( ICalFormat *parent )
      : mImpl( new ICalFormatImpl( parent ) ),
        mTimeSpec( KDateTime::UTC )
    {}
    ~Private()  { delete mImpl; }
    ICalFormatImpl *mImpl;
    KDateTime::Spec mTimeSpec;
};
//@endcond

ICalFormat::ICalFormat()
  : d( new Private( this ) )
{
}

ICalFormat::~ICalFormat()
{
  delete d;
}

bool ICalFormat::load( Calendar *calendar, const QString &fileName )
{
  kDebug() << fileName;

  clearException();

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    kDebug() << "load error";
    setException( new ErrorFormat( ErrorFormat::LoadError ) );
    return false;
  }
  QTextStream ts( &file );
  ts.setCodec( "ISO 8859-1" );
  QByteArray text = ts.readAll().trimmed().toLatin1();
  file.close();

  if ( text.isEmpty() ) {
    // empty files are valid
    return true;
  } else {
    return fromRawString( calendar, text );
  }
}

bool ICalFormat::save( Calendar *calendar, const QString &fileName )
{
  kDebug() << fileName;

  clearException();

  QString text = toString( calendar );
  if ( text.isEmpty() ) {
    return false;
  }

  // Write backup file
  KSaveFile::backupFile( fileName );

  KSaveFile file( fileName );
  if ( !file.open() ) {
    kDebug() << "err:" << file.errorString();
    setException( new ErrorFormat( ErrorFormat::SaveError,
                                   i18n( "Error saving to '%1'.", fileName ) ) );
    return false;
  }

  // Convert to UTF8 and save
  QByteArray textUtf8 = text.toUtf8();
  file.write( textUtf8.data(), textUtf8.size() );

  if ( !file.finalize() ) {
    kDebug() << "err:" << file.errorString();
    setException( new ErrorFormat( ErrorFormat::SaveError,
                                   i18n( "Could not save '%1'", fileName ) ) );
    return false;
  }

  return true;
}

bool ICalFormat::fromString( Calendar *cal, const QString &string )
{
  return fromRawString( cal, string.toUtf8() );
}

bool ICalFormat::fromRawString( Calendar *cal, const QByteArray &string )
{
  // Get first VCALENDAR component.
  // TODO: Handle more than one VCALENDAR or non-VCALENDAR top components
  icalcomponent *calendar;

  // Let's defend const correctness until the very gates of hell^Wlibical
  calendar = icalcomponent_new_from_string( const_cast<char*>( ( const char * )string ) );
  if ( !calendar ) {
    kDebug() << "parse error";
    setException( new ErrorFormat( ErrorFormat::ParseErrorIcal ) );
    return false;
  }

  bool success = true;

  if ( icalcomponent_isa( calendar ) == ICAL_XROOT_COMPONENT ) {
    icalcomponent *comp;
    for ( comp = icalcomponent_get_first_component( calendar, ICAL_VCALENDAR_COMPONENT );
          comp; comp = icalcomponent_get_next_component( calendar, ICAL_VCALENDAR_COMPONENT ) ) {
      // put all objects into their proper places
      if ( !d->mImpl->populate( cal, comp ) ) {
        kDebug() << "Could not populate calendar";
        if ( !exception() ) {
          setException( new ErrorFormat( ErrorFormat::ParseErrorKcal ) );
        }
        success = false;
      } else {
        setLoadedProductId( d->mImpl->loadedProductId() );
      }
    }
  } else if ( icalcomponent_isa( calendar ) != ICAL_VCALENDAR_COMPONENT ) {
    kDebug() << "No VCALENDAR component found";
    setException( new ErrorFormat( ErrorFormat::NoCalendar ) );
    success = false;
  } else {
    // put all objects into their proper places
    if ( !d->mImpl->populate( cal, calendar ) ) {
      kDebug() << "Could not populate calendar";
      if ( !exception() ) {
        setException( new ErrorFormat( ErrorFormat::ParseErrorKcal ) );
      }
      success = false;
    } else {
      setLoadedProductId( d->mImpl->loadedProductId() );
    }
  }

  icalcomponent_free( calendar );
  icalmemory_free_ring();

  return success;
}

Incidence *ICalFormat::fromString( const QString &string )
{
  CalendarLocal cal( d->mTimeSpec );
  fromString( &cal, string );

  Incidence *ical = 0;
  Event::List elist = cal.events();
  if ( elist.count() > 0 ) {
    ical = elist.first();
  } else {
    Todo::List tlist = cal.todos();
    if ( tlist.count() > 0 ) {
      ical = tlist.first();
    } else {
      Journal::List jlist = cal.journals();
      if ( jlist.count() > 0 ) {
        ical = jlist.first();
      }
    }
  }

  return ical ? ical->clone() : 0;
}

QString ICalFormat::toString( Calendar *cal )
{
  icalcomponent *calendar = d->mImpl->createCalendarComponent( cal );
  icalcomponent *component;

  ICalTimeZones *tzlist = cal->timeZones();  // time zones possibly used in the calendar
  ICalTimeZones tzUsedList;                  // time zones actually used in the calendar

  // todos
  Todo::List todoList = cal->rawTodos();
  Todo::List::ConstIterator it;
  for ( it = todoList.begin(); it != todoList.end(); ++it ) {
    component = d->mImpl->writeTodo( *it, tzlist, &tzUsedList );
    icalcomponent_add_component( calendar, component );
  }

  // events
  Event::List events = cal->rawEvents();
  Event::List::ConstIterator it2;
  for ( it2 = events.begin(); it2 != events.end(); ++it2 ) {
    if ( *it2 ) {
      component = d->mImpl->writeEvent( *it2, tzlist, &tzUsedList );
      icalcomponent_add_component( calendar, component );
    }
  }

  // journals
  Journal::List journals = cal->journals();
  Journal::List::ConstIterator it3;
  for ( it3 = journals.begin(); it3 != journals.end(); ++it3 ) {
    component = d->mImpl->writeJournal( *it3, tzlist, &tzUsedList );
    icalcomponent_add_component( calendar, component );
  }

  // time zones
  const ICalTimeZones::ZoneMap zones = tzUsedList.zones();
  for ( ICalTimeZones::ZoneMap::ConstIterator it = zones.begin();  it != zones.end();  ++it ) {
    icaltimezone *tz = (*it).icalTimezone();
    if ( !tz ) {
      kError() << "bad time zone";
    } else {
      component = icalcomponent_new_clone( icaltimezone_get_component( tz ) );
      icalcomponent_add_component( calendar, component );
      icaltimezone_free( tz, 1 );
    }
  }

  QString text = QString::fromUtf8( icalcomponent_as_ical_string( calendar ) );

  icalcomponent_free( calendar );
  icalmemory_free_ring();

  if ( text.isEmpty() ) {
    setException( new ErrorFormat( ErrorFormat::SaveError,
                                   i18n( "libical error" ) ) );
  }

  return text;
}

QString ICalFormat::toICalString( Incidence *incidence )
{
  CalendarLocal cal( d->mTimeSpec );
  cal.addIncidence( incidence->clone() );
  return toString( &cal );
}

QString ICalFormat::toString( Incidence *incidence )
{
  icalcomponent *component;

  component = d->mImpl->writeIncidence( incidence );

  QString text = QString::fromUtf8( icalcomponent_as_ical_string( component ) );

  icalcomponent_free( component );

  return text;
}

QString ICalFormat::toString( RecurrenceRule *recurrence )
{
  icalproperty *property;
  property = icalproperty_new_rrule( d->mImpl->writeRecurrenceRule( recurrence ) );
  QString text = QString::fromUtf8( icalproperty_as_ical_string( property ) );
  icalproperty_free( property );
  return text;
}

bool ICalFormat::fromString( RecurrenceRule *recurrence, const QString &rrule )
{
  if ( !recurrence ) {
    return false;
  }
  bool success = true;
  icalerror_clear_errno();
  struct icalrecurrencetype recur = icalrecurrencetype_from_string( rrule.toLatin1() );
  if ( icalerrno != ICAL_NO_ERROR ) {
    kDebug() << "Recurrence parsing error:" << icalerror_strerror( icalerrno );
    success = false;
  }

  if ( success ) {
    d->mImpl->readRecurrence( recur, recurrence );
  }

  return success;
}

QString ICalFormat::createScheduleMessage( IncidenceBase *incidence,
                                           iTIPMethod method )
{
  icalcomponent *message = 0;

  // Handle scheduling ID being present
  if ( incidence->type() == "Event" || incidence->type() == "Todo" ) {
    Incidence *i = static_cast<Incidence*>( incidence );
    if ( i->schedulingID() != i->uid() ) {
      // We have a separation of scheduling ID and UID
      i = i->clone();
      i->setUid( i->schedulingID() );
      i->setSchedulingID( QString() );

      // Build the message with the cloned incidence
      message = d->mImpl->createScheduleComponent( i, method );

      // And clean up
      delete i;
    }
  }

  if ( message == 0 ) {
    message = d->mImpl->createScheduleComponent( incidence, method );
  }

  QString messageText = QString::fromUtf8( icalcomponent_as_ical_string( message ) );

  icalcomponent_free( message );
  return messageText;
}

FreeBusy *ICalFormat::parseFreeBusy( const QString &str )
{
  clearException();

  icalcomponent *message;
  message = icalparser_parse_string( str.toUtf8() );

  if ( !message ) {
    return 0;
  }

  FreeBusy *freeBusy = 0;

  icalcomponent *c;
  for ( c = icalcomponent_get_first_component( message, ICAL_VFREEBUSY_COMPONENT );
        c != 0; c = icalcomponent_get_next_component( message, ICAL_VFREEBUSY_COMPONENT ) ) {
    FreeBusy *fb = d->mImpl->readFreeBusy( c );

    if ( freeBusy ) {
      freeBusy->merge( fb );
      delete fb;
    } else {
      freeBusy = fb;
    }
  }

  if ( !freeBusy ) {
    kDebug() << "object is not a freebusy.";
  }
  return freeBusy;
}

ScheduleMessage *ICalFormat::parseScheduleMessage( Calendar *cal,
                                                   const QString &messageText )
{
  setTimeSpec( cal->timeSpec() );
  clearException();

  if ( messageText.isEmpty() ) {
    setException(
      new ErrorFormat( ErrorFormat::ParseErrorKcal,
                       QLatin1String( "messageText is empty, unable "
                                      "to parse into a ScheduleMessage" ) ) );
    return 0;
  }

  icalcomponent *message;
  message = icalparser_parse_string( messageText.toUtf8() );

  if ( !message ) {
    setException(
      new ErrorFormat( ErrorFormat::ParseErrorKcal,
                       QLatin1String( "icalparser is unable to parse "
                                      "messageText into a ScheduleMessage" ) ) );
    return 0;
  }

  icalproperty *m =
    icalcomponent_get_first_property( message, ICAL_METHOD_PROPERTY );
  if ( !m ) {
    setException(
      new ErrorFormat( ErrorFormat::ParseErrorKcal,
                       QLatin1String( "message does not contain an "
                                      "ICAL_METHOD_PROPERTY" ) ) );
    return 0;
  }

  // Populate the message's time zone collection with all VTIMEZONE components
  ICalTimeZones tzlist;
  ICalTimeZoneSource tzs;
  tzs.parse( message, tzlist );

  icalcomponent *c;

  IncidenceBase *incidence = 0;
  c = icalcomponent_get_first_component( message, ICAL_VEVENT_COMPONENT );
  if ( c ) {
    incidence = d->mImpl->readEvent( c, &tzlist );
  }

  if ( !incidence ) {
    c = icalcomponent_get_first_component( message, ICAL_VTODO_COMPONENT );
    if ( c ) {
      incidence = d->mImpl->readTodo( c, &tzlist );
    }
  }

  if ( !incidence ) {
    c = icalcomponent_get_first_component( message, ICAL_VJOURNAL_COMPONENT );
    if ( c ) {
      incidence = d->mImpl->readJournal( c, &tzlist );
    }
  }

  if ( !incidence ) {
    c = icalcomponent_get_first_component( message, ICAL_VFREEBUSY_COMPONENT );
    if ( c ) {
      incidence = d->mImpl->readFreeBusy( c );
    }
  }

  if ( !incidence ) {
    kDebug() << "object is not a freebusy, event, todo or journal";
    setException(
      new ErrorFormat( ErrorFormat::ParseErrorKcal,
                       QLatin1String( "object is not a freebusy, event, "
                                      "todo or journal" ) ) );
    return 0;
  }

  kDebug() << "getting method...";

  icalproperty_method icalmethod = icalproperty_get_method( m );
  iTIPMethod method;

  switch ( icalmethod ) {
  case ICAL_METHOD_PUBLISH:
    method = iTIPPublish;
    break;
  case ICAL_METHOD_REQUEST:
    method = iTIPRequest;
    break;
  case ICAL_METHOD_REFRESH:
    method = iTIPRefresh;
    break;
  case ICAL_METHOD_CANCEL:
    method = iTIPCancel;
    break;
  case ICAL_METHOD_ADD:
    method = iTIPAdd;
    break;
  case ICAL_METHOD_REPLY:
    method = iTIPReply;
    break;
  case ICAL_METHOD_COUNTER:
    method = iTIPCounter;
    break;
  case ICAL_METHOD_DECLINECOUNTER:
    method = iTIPDeclineCounter;
    break;
  default:
    method = iTIPNoMethod;
    kDebug() << "Unknown method";
    break;
  }

  kDebug() << "restriction...";

  if ( !icalrestriction_check( message ) ) {
    kWarning() << endl
               << "kcal library reported a problem while parsing:";
    kWarning() << Scheduler::translatedMethodName( method ) << ":"
               << d->mImpl->extractErrorProperty( c );
  }

  Incidence *existingIncidence = cal->incidenceFromSchedulingID( incidence->uid() );

  icalcomponent *calendarComponent = 0;
  if ( existingIncidence ) {
    calendarComponent = d->mImpl->createCalendarComponent( cal );

    // TODO: check, if cast is required, or if it can be done by virtual funcs.
    // TODO: Use a visitor for this!
    if ( existingIncidence->type() == "Todo" ) {
      Todo *todo = static_cast<Todo *>( existingIncidence );
      icalcomponent_add_component( calendarComponent,
                                   d->mImpl->writeTodo( todo ) );
    }
    if ( existingIncidence->type() == "Event" ) {
      Event *event = static_cast<Event *>( existingIncidence );
      icalcomponent_add_component( calendarComponent,
                                   d->mImpl->writeEvent( event ) );
    }
  }

  kDebug() << "classify...";

  icalproperty_xlicclass result =
    icalclassify( message, calendarComponent, (char *)"" );

  kDebug() << "returning with result = " << result;

  ScheduleMessage::Status status;

  switch ( result ) {
  case ICAL_XLICCLASS_PUBLISHNEW:
    status = ScheduleMessage::PublishNew;
    break;
  case ICAL_XLICCLASS_PUBLISHUPDATE:
    status = ScheduleMessage::PublishUpdate;
    break;
  case ICAL_XLICCLASS_OBSOLETE:
    status = ScheduleMessage::Obsolete;
    break;
  case ICAL_XLICCLASS_REQUESTNEW:
    status = ScheduleMessage::RequestNew;
    break;
  case ICAL_XLICCLASS_REQUESTUPDATE:
    status = ScheduleMessage::RequestUpdate;
    break;
  case ICAL_XLICCLASS_UNKNOWN:
  default:
    status = ScheduleMessage::Unknown;
    break;
  }

  kDebug() << "status =" << status;

  icalcomponent_free( message );
  if ( calendarComponent )
    icalcomponent_free( calendarComponent );
  return new ScheduleMessage( incidence, method, status );
}

void ICalFormat::setTimeSpec( const KDateTime::Spec &timeSpec )
{
  d->mTimeSpec = timeSpec;
}

KDateTime::Spec ICalFormat::timeSpec() const
{
  return d->mTimeSpec;
}

QString ICalFormat::timeZoneId() const
{
  KTimeZone tz = d->mTimeSpec.timeZone();
  return tz.isValid() ? tz.name() : QString();
}
