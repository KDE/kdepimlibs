/*
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
/**
  @file
  This file is part of the API for handling TNEF data and provides
  static Formatter helpers.

  @brief
  Provides helpers too format @acronym TNEF attachments into different
  formats like eg. a HTML representation.

  @author Cornelius Schumacher
  @author Reinhold Kainhofer
  @author Rafal Rzepecki
*/

#include "formatter.h"
#include "ktnefparser.h"
#include "ktnefmessage.h"
#include "ktnefdefs.h"

#include <kpimutils/email.h>
#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>

#include <kcalcore/calendar.h>
#include <kcalcore/icalformat.h>
#include <kcalutils/incidenceformatter.h>

#include <klocalizedstring.h>
#include <kdatetime.h>

#include <QtCore/QBuffer>

#include <time.h>

using namespace KCalCore;
using namespace KTnef;

/*******************************************************************
 *  Helper functions for the msTNEF -> VPart converter
 *******************************************************************/

//-----------------------------------------------------------------------------
//@cond IGNORE
static QString stringProp( KTNEFMessage *tnefMsg, const quint32 &key,
                           const QString &fallback = QString() )
{
  return tnefMsg->findProp( key < 0x10000 ? key & 0xFFFF : key >> 16, fallback );
}

static QString sNamedProp( KTNEFMessage *tnefMsg, const QString &name,
                           const QString &fallback = QString() )
{
  return tnefMsg->findNamedProp( name, fallback );
}

struct save_tz {
  char *old_tz;
  char *tz_env_str;
};

/* temporarily go to a different timezone */
static struct save_tz set_tz( const char *_tc )
{
  const char *tc = _tc?_tc:"UTC";

  struct save_tz rv;

  rv.old_tz = 0;
  rv.tz_env_str = 0;

  //qDebug() << "set_tz(), timezone before =" << timezone;

  char *tz_env = 0;
  if ( !qgetenv( "TZ" ).isEmpty() ) {
    tz_env = qstrdup( qgetenv( "TZ" ) );
    rv.old_tz = tz_env;
  }
  char *tmp_env = (char*)malloc( strlen( tc ) + 4 );
  strcpy( tmp_env, "TZ=" );
  strcpy( tmp_env+3, tc );
  putenv( tmp_env );

  rv.tz_env_str = tmp_env;

  /* tmp_env is not free'ed -- it is part of the environment */

  tzset();
  //qDebug() << "set_tz(), timezone after =" << timezone;

  return rv;
}

/* restore previous timezone */
static void unset_tz( struct save_tz old_tz )
{
  if ( old_tz.old_tz ) {
    char *tmp_env = (char*)malloc( strlen( old_tz.old_tz ) + 4 );
    strcpy( tmp_env, "TZ=" );
    strcpy( tmp_env+3, old_tz.old_tz );
    putenv( tmp_env );
    /* tmp_env is not free'ed -- it is part of the environment */
    free( old_tz.old_tz );
  } else {
    /* clear TZ from env */
    putenv( strdup( "TZ" ) );
  }
  tzset();

  /* is this OK? */
  if ( old_tz.tz_env_str ) {
    free( old_tz.tz_env_str );
  }
}

static KDateTime utc2Local( const KDateTime &utcdt )
{
  struct tm tmL;

  save_tz tmp_tz = set_tz( "UTC" );
  time_t utc = utcdt.toTime_t();
  unset_tz( tmp_tz );

  localtime_r( &utc, &tmL );
  return KDateTime( QDate( tmL.tm_year + 1900, tmL.tm_mon + 1, tmL.tm_mday ),
                    QTime( tmL.tm_hour, tmL.tm_min, tmL.tm_sec ) );
}

static KDateTime pureISOToLocalQDateTime( const QString &dtStr,
                                          bool bDateOnly = false )
{
  QDate tmpDate;
  QTime tmpTime;
  int year, month, day, hour, minute, second;

  if ( bDateOnly ) {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = 0;
    minute = 0;
    second = 0;
  } else {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = dtStr.mid( 9, 2 ).toInt();
    minute = dtStr.mid( 11, 2 ).toInt();
    second = dtStr.mid( 13, 2 ).toInt();
  }
  tmpDate.setYMD( year, month, day );
  tmpTime.setHMS( hour, minute, second );

  if ( tmpDate.isValid() && tmpTime.isValid() ) {
    KDateTime dT = KDateTime( tmpDate, tmpTime );

    if ( !bDateOnly ) {
      // correct for GMT ( == Zulu time == UTC )
      if ( dtStr.at( dtStr.length() - 1 ) == QLatin1Char('Z') ) {
        //dT = dT.addSecs( 60 * KRFCDate::localUTCOffset() );
        //localUTCOffset( dT ) );
        dT = utc2Local( dT );
      }
    }
    return dT;
  } else {
    return KDateTime();
  }
}
//@endcond

QString KTnef::msTNEFToVPart( const QByteArray &tnef )
{
  bool bOk = false;

  KTNEFParser parser;
  QByteArray b( tnef );
  QBuffer buf( &b );
  MemoryCalendar::Ptr cal( new MemoryCalendar( KDateTime::UTC ) );
  KABC::Addressee addressee;
  ICalFormat calFormat;
  Event::Ptr event( new Event() );

  if ( parser.openDevice( &buf ) ) {
    KTNEFMessage *tnefMsg = parser.message();
    //QMap<int,KTNEFProperty*> props = parser.message()->properties();

    // Everything depends from property PR_MESSAGE_CLASS
    // (this is added by KTNEFParser):
    QString msgClass = tnefMsg->findProp( 0x001A, QString(), true ).toUpper();
    if ( !msgClass.isEmpty() ) {
      // Match the old class names that might be used by Outlook for
      // compatibility with Microsoft Mail for Windows for Workgroups 3.1.
      bool bCompatClassAppointment = false;
      bool bCompatMethodRequest = false;
      bool bCompatMethodCancled = false;
      bool bCompatMethodAccepted = false;
      bool bCompatMethodAcceptedCond = false;
      bool bCompatMethodDeclined = false;
      if ( msgClass.startsWith( QLatin1String( "IPM.MICROSOFT SCHEDULE." ) ) ) {
        bCompatClassAppointment = true;
        if ( msgClass.endsWith( QLatin1String( ".MTGREQ" ) ) ) {
          bCompatMethodRequest = true;
        }
        if ( msgClass.endsWith( QLatin1String( ".MTGCNCL" ) ) ) {
          bCompatMethodCancled = true;
        }
        if ( msgClass.endsWith( QLatin1String( ".MTGRESPP" ) ) ) {
          bCompatMethodAccepted = true;
        }
        if ( msgClass.endsWith( QLatin1String( ".MTGRESPA" ) ) ) {
          bCompatMethodAcceptedCond = true;
        }
        if ( msgClass.endsWith( QLatin1String( ".MTGRESPN" ) ) ) {
          bCompatMethodDeclined = true;
        }
      }
      bool bCompatClassNote = ( msgClass == QLatin1String("IPM.MICROSOFT MAIL.NOTE") );

      if ( bCompatClassAppointment || QLatin1String("IPM.APPOINTMENT") == msgClass ) {
        // Compose a vCal
        bool bIsReply = false;
        QString prodID = QLatin1String("-//Microsoft Corporation//Outlook ");
        prodID += tnefMsg->findNamedProp( QLatin1String("0x8554"), QLatin1String("9.0") );
        prodID += QLatin1String("MIMEDIR/EN\n");
        prodID += QLatin1String("VERSION:2.0\n");
        calFormat.setApplication( QLatin1String("Outlook"), prodID );

        iTIPMethod method;
        if ( bCompatMethodRequest ) {
          method = iTIPRequest;
        } else if ( bCompatMethodCancled ) {
          method = iTIPCancel;
        } else if ( bCompatMethodAccepted || bCompatMethodAcceptedCond ||
                 bCompatMethodDeclined ) {
          method = iTIPReply;
          bIsReply = true;
        } else {
          // pending(khz): verify whether "0x0c17" is the right tag ???
          //
          // at the moment we think there are REQUESTS and UPDATES
          //
          // but WHAT ABOUT REPLIES ???
          //
          //

          if ( tnefMsg->findProp(0x0c17) == QLatin1String("1") ) {
            bIsReply = true;
          }
          method = iTIPRequest;
        }

        /// ###  FIXME Need to get this attribute written
        //ScheduleMessage schedMsg( event, method, ScheduleMessage::Unknown );

        QString sSenderSearchKeyEmail( tnefMsg->findProp( 0x0C1D ) );

        if ( !sSenderSearchKeyEmail.isEmpty() ) {
          int colon = sSenderSearchKeyEmail.indexOf( QLatin1Char(':') );
          // May be e.g. "SMTP:KHZ@KDE.ORG"
          if ( sSenderSearchKeyEmail.indexOf( QLatin1Char(':') ) == -1 ) {
            sSenderSearchKeyEmail.remove( 0, colon+1 );
          }
        }

        QString s( tnefMsg->findProp( 0x8189 ) );
        const QStringList attendees = s.split( QLatin1Char(';') );
        if ( attendees.count() ) {
          for ( QStringList::const_iterator it = attendees.begin();
               it != attendees.end(); ++it ) {
            // Skip all entries that have no '@' since these are
            // no mail addresses
            if ( (*it).indexOf( QLatin1Char('@') ) == -1 ) {
              s = (*it).trimmed();

              Attendee::Ptr attendee( new Attendee( s, s, true ) );
              if ( bIsReply ) {
                if ( bCompatMethodAccepted ) {
                  attendee->setStatus( Attendee::Accepted );
                }
                if ( bCompatMethodDeclined ) {
                  attendee->setStatus( Attendee::Declined );
                }
                if ( bCompatMethodAcceptedCond ) {
                  attendee->setStatus( Attendee::Tentative );
                }
              } else {
                attendee->setStatus( Attendee::NeedsAction );
                attendee->setRole( Attendee::ReqParticipant );
              }
              event->addAttendee( attendee );
            }
          }
        } else {
          // Oops, no attendees?
          // This must be old style, let us use the PR_SENDER_SEARCH_KEY.
          s = sSenderSearchKeyEmail;
          if ( !s.isEmpty() ) {
            Attendee::Ptr attendee( new Attendee( QString(), QString(), true ) );
            if ( bIsReply ) {
              if ( bCompatMethodAccepted ) {
                attendee->setStatus( Attendee::Accepted );
              }
              if ( bCompatMethodAcceptedCond ) {
                attendee->setStatus( Attendee::Declined );
              }
              if ( bCompatMethodDeclined ) {
                attendee->setStatus( Attendee::Tentative );
              }
            } else {
              attendee->setStatus( Attendee::NeedsAction );
              attendee->setRole( Attendee::ReqParticipant );
            }
            event->addAttendee( attendee );
          }
        }
        s = tnefMsg->findProp( 0x3ff8 ); // look for organizer property
        if ( s.isEmpty() && !bIsReply ) {
          s = sSenderSearchKeyEmail;
        }
        // TODO: Use the common name?
        if ( !s.isEmpty() ) {
          event->setOrganizer( s );
        }

        s = tnefMsg->findProp( 0x819b ).remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) );
        event->setDtStart( KDateTime::fromString( s ) ); // ## Format??

        s = tnefMsg->findProp( 0x819c ).remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) );
        event->setDtEnd( KDateTime::fromString( s ) );

        s = tnefMsg->findProp( 0x810d );
        event->setLocation( s );
        // is it OK to set this to OPAQUE always ??
        //vPart += "TRANSP:OPAQUE\n"; ###FIXME, portme!
        //vPart += "SEQUENCE:0\n";

        // is "0x0023" OK  -  or should we look for "0x0003" ??
        s = tnefMsg->findProp( 0x0023 );
        event->setUid( s );

        // PENDING(khz): is this value in local timezone? Must it be
        // adjusted? Most likely this is a bug in the server or in
        // Outlook - we ignore it for now.
        s = tnefMsg->findProp( 0x8202 ).remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) );
        // ### kcal always uses currentDateTime()
        // event->setDtStamp( QDateTime::fromString( s ) );

        s = tnefMsg->findNamedProp( QLatin1String("Keywords") );
        event->setCategories( s );

        s = tnefMsg->findProp( 0x1000 );
        event->setDescription( s );

        s = tnefMsg->findProp( 0x0070 );
        event->setSummary( s );

        s = tnefMsg->findProp( 0x0026 );
        event->setPriority( s.toInt() );
        // is reminder flag set ?
        if ( !tnefMsg->findProp( 0x8503 ).isEmpty() ) {
          Alarm::Ptr alarm( new Alarm( event.data() ) ); // TODO: fix when KCalCore::Alarm is fixed
          KDateTime highNoonTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8502 ).
                                     remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) ) );
          KDateTime wakeMeUpTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8560, QString() ).
                                     remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) ) );
          alarm->setTime( wakeMeUpTime );

          if ( highNoonTime.isValid() && wakeMeUpTime.isValid() ) {
            alarm->setStartOffset( Duration( highNoonTime, wakeMeUpTime ) );
          } else {
            // default: wake them up 15 minutes before the appointment
            alarm->setStartOffset( Duration( 15 * 60 ) );
          }
          alarm->setDisplayAlarm( i18n( "Reminder" ) );

          // Sorry: the different action types are not known (yet)
          //        so we always set 'DISPLAY' (no sounds, no images...)
          event->addAlarm( alarm );
        }
        //ensure we have a uid for this event
        if ( event->uid().isEmpty() ) {
          event->setUid( CalFormat::createUniqueId() );
        }
        cal->addEvent( event );
        bOk = true;
        // we finished composing a vCal
      } else if ( bCompatClassNote || QLatin1String("IPM.CONTACT") == msgClass ) {
        addressee.setUid( stringProp( tnefMsg, attMSGID ) );
        addressee.setFormattedName( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME ) );
        addressee.insertEmail( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_EMAIL1EMAILADDRESS) ), true );
        addressee.insertEmail( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_EMAIL2EMAILADDRESS) ), false );
        addressee.insertEmail( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_EMAIL3EMAILADDRESS) ), false );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-IMAddress"),
                                sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_IMADDRESS) ) );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-SpousesName"),
                                stringProp( tnefMsg, MAPI_TAG_PR_SPOUSE_NAME ) );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-ManagersName"),
                                stringProp( tnefMsg, MAPI_TAG_PR_MANAGER_NAME ) );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-AssistantsName"),
                                stringProp( tnefMsg, MAPI_TAG_PR_ASSISTANT ) );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-Department"),
                                stringProp( tnefMsg, MAPI_TAG_PR_DEPARTMENT_NAME ) );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-Office"),
                                stringProp( tnefMsg, MAPI_TAG_PR_OFFICE_LOCATION ) );
        addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-Profession"),
                                stringProp( tnefMsg, MAPI_TAG_PR_PROFESSION ) );

        QString s = tnefMsg->findProp( MAPI_TAG_PR_WEDDING_ANNIVERSARY ).
                    remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) );
        if ( !s.isEmpty() ) {
          addressee.insertCustom( QLatin1String("KADDRESSBOOK"), QLatin1String("X-Anniversary"), s );
        }

        addressee.setUrl( QUrl( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_WEBPAGE) ) ) );

        // collect parts of Name entry
        addressee.setFamilyName( stringProp( tnefMsg, MAPI_TAG_PR_SURNAME ) );
        addressee.setGivenName( stringProp( tnefMsg, MAPI_TAG_PR_GIVEN_NAME ) );
        addressee.setAdditionalName( stringProp( tnefMsg, MAPI_TAG_PR_MIDDLE_NAME ) );
        addressee.setPrefix( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME_PREFIX ) );
        addressee.setSuffix( stringProp( tnefMsg, MAPI_TAG_PR_GENERATION ) );

        addressee.setNickName( stringProp( tnefMsg, MAPI_TAG_PR_NICKNAME ) );
        addressee.setRole( stringProp( tnefMsg, MAPI_TAG_PR_TITLE ) );
        addressee.setOrganization( stringProp( tnefMsg, MAPI_TAG_PR_COMPANY_NAME ) );
        /*
        the MAPI property ID of this (multiline) )field is unknown:
        vPart += stringProp(tnefMsg, "\n","NOTE", ... , "" );
        */

        KABC::Address adr;
        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STREET ) );
        adr.setLocality( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_CITY ) );
        adr.setRegion( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_COUNTRY ) );
        adr.setType( KABC::Address::Home );
        addressee.insertAddress( adr );

        adr.setPostOfficeBox( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_BUSINESSADDRESSPOBOX) ) );
        adr.setStreet( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_BUSINESSADDRESSSTREET) ) );
        adr.setLocality( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_BUSINESSADDRESSCITY) ) );
        adr.setRegion( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_BUSINESSADDRESSSTATE) ) );
        adr.setPostalCode( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_BUSINESSADDRESSPOSTALCODE) ) );
        adr.setCountry( sNamedProp( tnefMsg, QLatin1String(MAPI_TAG_CONTACT_BUSINESSADDRESSCOUNTRY) ) );
        adr.setType( KABC::Address::Work );
        addressee.insertAddress( adr );

        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STREET ) );
        adr.setLocality( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_CITY ) );
        adr.setRegion( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_COUNTRY ) );
        adr.setType( KABC::Address::Dom );
        addressee.insertAddress( adr );

        // problem: the 'other' address was stored by KOrganizer in
        //          a line looking like the following one:
        // vPart += "\nADR;TYPE=dom;TYPE=intl;TYPE=parcel;TYPE=postal;TYPE=work;"
        //          "TYPE=home:other_pobox;;other_str1\nother_str2;other_loc;other_region;"
        //          "other_pocode;other_country"

        QString nr;
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber(
          KABC::PhoneNumber( nr, KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber(
          KABC::PhoneNumber( nr, KABC::PhoneNumber::Work ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_MOBILE_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber(
          KABC::PhoneNumber( nr, KABC::PhoneNumber::Cell ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_FAX_NUMBER );
        addressee.insertPhoneNumber(
          KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_FAX_NUMBER );
        addressee.insertPhoneNumber(
          KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work ) );

        s = tnefMsg->findProp( MAPI_TAG_PR_BIRTHDAY ).
            remove( QLatin1Char( '-' ) ).remove( QLatin1Char( ':' ) );
        if ( !s.isEmpty() ) {
          addressee.setBirthday( QDateTime::fromString( s ) );
        }

        bOk = ( !addressee.isEmpty() );
      } else if ( QLatin1String("IPM.NOTE") == msgClass ) {

      } // else if ... and so on ...
    }
  }

  // Compose return string
  // KDAB_TODO: Interesting, without the explicit QString the toString call is
  //            reported to be ambigious with toString( const Incidence::Ptr & ).
  const QString iCal = calFormat.toString( cal, QString() );
  if ( !iCal.isEmpty() ) {
    // This was an iCal
    return iCal;
  }

  // Not an iCal - try a vCard
  KABC::VCardConverter converter;
  return QString::fromUtf8( converter.createVCard( addressee ) );
}

QString KTnef::formatTNEFInvitation( const QByteArray &tnef,
                                     const MemoryCalendar::Ptr &cal,
                                     KCalUtils::InvitationFormatterHelper *h )
{
  const QString vPart = msTNEFToVPart( tnef );
  QString iCal = KCalUtils::IncidenceFormatter::formatICalInvitation( vPart, cal, h, true );
  if ( !iCal.isEmpty() ) {
    return iCal;
  } else {
    return vPart;
  }
}

