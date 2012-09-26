/*
    This file is part of the kcal library.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "qtopiaformat.h"

#include "calendar.h"
#include "calendarlocal.h"

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QClipboard>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <QTextDocument>

#include <QtXml/QXmlAttributes>
#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlParseException>
#include <QtXml/QXmlInputSource>
#include <QtXml/QXmlSimpleReader>

#include <kdebug.h>
#include <klocale.h>
#include <kdatetime.h>

using namespace KCal;

//@cond PRIVATE
class QtopiaParser : public QXmlDefaultHandler
{
  public:
    QtopiaParser( Calendar *calendar ) : mCalendar( calendar ) {}

    bool startElement( const QString &, const QString &, const QString &qName,
                       const QXmlAttributes &attributes )
    {
      if ( qName == "event" ) {
        Event *event = new Event;
        QString uid = "Qtopia" + attributes.value( "uid" );
        event->setUid( uid );

        event->setSummary( attributes.value( "description" ),
                           Qt::mightBeRichText( attributes.value( "description" ) ) );
        event->setLocation( attributes.value( "location" ),
                            Qt::mightBeRichText( attributes.value( "location" ) ) );
        event->setDescription( attributes.value( "note" ),
                               Qt::mightBeRichText( attributes.value( "note" ) ) );
        event->setDtStart( toDateTime( attributes.value( "start" ) ) );
        event->setDtEnd( toDateTime( attributes.value( "end" ) ) );

        if ( attributes.value( "type" ) == "AllDay" ) {
          event->setAllDay( true );
        } else {
          event->setAllDay( false );
        }

        QString rtype = attributes.value( "rtype" );
        if ( !rtype.isEmpty() ) {
          QDate startDate = event->dtStart().date();

          QString freqStr = attributes.value( "rfreq" );
          int freq = freqStr.toInt();

          QString hasEndDateStr = attributes.value( "rhasenddate" );
          bool hasEndDate = hasEndDateStr == "1";

          QString endDateStr = attributes.value( "enddt" );
          QDate endDate = toDateTime( endDateStr ).date();

          QString weekDaysStr = attributes.value( "rweekdays" );
          int weekDaysNum = weekDaysStr.toInt();
          QBitArray weekDays( 7 );
          int i;
          for ( i = 1; i <= 7; ++i ) {
            weekDays.setBit( i - 1, ( 2 << i ) & weekDaysNum );
          }

          QString posStr = attributes.value( "rposition" );
          int pos = posStr.toInt();

          Recurrence *r = event->recurrence();

          if ( rtype == "Daily" ) {
            r->setDaily( freq );
            if ( hasEndDate ) {
              r->setEndDate( endDate );
            }
          } else if ( rtype == "Weekly" ) {
            r->setWeekly( freq, weekDays );
            if ( hasEndDate ) {
              r->setEndDate( endDate );
            }
          } else if ( rtype == "MonthlyDate" ) {
            r->setMonthly( freq );
            if ( hasEndDate ) {
              r->setEndDate( endDate );
            }
            r->addMonthlyDate( static_cast<short>( startDate.day() ) );
          } else if ( rtype == "MonthlyDay" ) {
            r->setMonthly( freq );
            if ( hasEndDate ) {
              r->setEndDate( endDate );
            }
            QBitArray days( 7 );
            days.fill( false );
            days.setBit( startDate.dayOfWeek() - 1 );
            r->addMonthlyPos( static_cast<short>( pos ), days );
          } else if ( rtype == "Yearly" ) {
            r->setYearly( freq );
            if ( hasEndDate ) {
              r->setEndDate( endDate );
            }
          }
        }

        QString categoryList = attributes.value( "categories" );
        event->setCategories( lookupCategories( categoryList ) );

        QString alarmStr = attributes.value( "alarm" );
        if ( !alarmStr.isEmpty() ) {
          kDebug() << "Alarm:" << alarmStr;
          Alarm *alarm = new Alarm( event );
          alarm->setType( Alarm::Display );
          alarm->setEnabled( true );
          int alarmOffset = alarmStr.toInt();
          alarm->setStartOffset( Duration( alarmOffset * -60 ) );
          event->addAlarm( alarm );
        }

        Event *oldEvent = mCalendar->event( uid );
        if ( oldEvent ) {
          mCalendar->deleteEvent( oldEvent );
        }

        mCalendar->addEvent( event );
      } else if ( qName == "Task" ) {
        Todo *todo = new Todo;

        QString uid = "Qtopia" + attributes.value( "Uid" );
        todo->setUid( uid );

        QString description = attributes.value( "Description" );
        int pos = description.indexOf( '\n' );
        if ( pos > 0 ) {
          QString summary = description.left( pos );
          todo->setSummary( summary, Qt::mightBeRichText( summary ) );
          todo->setDescription( description, Qt::mightBeRichText( description ) );
        } else {
          todo->setSummary( description, Qt::mightBeRichText( description ) );
        }

        int priority = attributes.value( "Priority" ).toInt();
//        if ( priority == 0 ) priority = 3;
        todo->setPriority( priority );

        QString categoryList = attributes.value( "Categories" );
        todo->setCategories( lookupCategories( categoryList ) );

        QString completedStr = attributes.value( "Completed" );
        if ( completedStr == "1" ) {
          todo->setCompleted( true );
        }

        QString hasDateStr = attributes.value( "HasDate" );
        if ( hasDateStr == "1" ) {
          int year = attributes.value( "DateYear" ).toInt();
          int month = attributes.value( "DateMonth" ).toInt();
          int day = attributes.value( "DateDay" ).toInt();

          todo->setDtDue( KDateTime( QDate( year, month, day ), KDateTime::UTC ) );
          todo->setHasDueDate( true );
        }

        Todo *oldTodo = mCalendar->todo( uid );
        if ( oldTodo ) {
          mCalendar->deleteTodo( oldTodo );
        }

        mCalendar->addTodo( todo );
      } else if ( qName == "Category" ) {
        QString id = attributes.value( "id" );
        QString name = attributes.value( "name" );
        setCategory( id, name );
      }

      return true;
    }

    bool warning ( const QXmlParseException &exception )
    {
      kDebug() << "WARNING";
      printException( exception );
      return true;
    }

    bool error ( const QXmlParseException &exception )
    {
      kDebug() << "ERROR";
      printException( exception );
      return false;
    }

    bool fatalError ( const QXmlParseException &exception )
    {
      kDebug() << "FATALERROR";
      printException( exception );
      return false;
    }

    QString errorString () const
    {
      return "QtopiaParser: Error!";
    }

  protected:
    void printException( const QXmlParseException &exception )
    {
      kError() << "XML Parse Error (line" << exception.lineNumber()
                << ", col" << exception.columnNumber() << "):"
                << exception.message() << "(public ID: '"
                << exception.publicId() << "' system ID: '"
                << exception.systemId() << "')";
    }

    KDateTime toDateTime( const QString &value )
    {
      KDateTime dt;
      dt.setTime_t( value.toUInt() );

      return dt;
    }

    QStringList lookupCategories( const QString &categoryList )
    {
      const QStringList categoryIds = categoryList.split( ';' );
      QStringList categories;
      QStringList::ConstIterator it;
      for ( it = categoryIds.constBegin(); it != categoryIds.constEnd(); ++it ) {
        categories.append( category( *it ) );
      }
      return categories;
    }

  private:
    Calendar *mCalendar;

    static QString category( const QString &id )
    {
      QMap<QString,QString>::ConstIterator it = mCategoriesMap.constFind( id );
      if ( it == mCategoriesMap.constEnd() ) {
        return id;
      } else {
        return *it;
      }
    }

    static void setCategory( const QString &id, const QString &name )
    {
      mCategoriesMap.insert( id, name );
    }

    static QMap<QString,QString> mCategoriesMap;
};

QMap<QString,QString> QtopiaParser::mCategoriesMap;
//@endcond

QtopiaFormat::QtopiaFormat() : d( 0 )
{
}

QtopiaFormat::~QtopiaFormat()
{
}

bool QtopiaFormat::load( Calendar *calendar, const QString &fileName )
{
  kDebug() << fileName;

  clearException();

  QtopiaParser handler( calendar );
  QFile xmlFile( fileName );
  QXmlInputSource source( &xmlFile );
  QXmlSimpleReader reader;
  reader.setContentHandler( &handler );
  return reader.parse( source );
}

bool QtopiaFormat::save( Calendar *calendar, const QString &fileName )
{
  kDebug() << fileName;

  clearException();

  QString text = toString( calendar );

  if ( text.isNull() ) {
    return false;
  }

  // TODO: write backup file

  QFile file( fileName );
  if (!file.open( QIODevice::WriteOnly ) ) {
    setException( new ErrorFormat( ErrorFormat::SaveError,
                                   i18n( "Could not open file '%1'", fileName ) ) );
    return false;
  }
  QTextStream ts( &file );
  ts << text;
  file.close();

  return true;
}

bool QtopiaFormat::fromString( Calendar *, const QString & )
{
  kDebug() << "not yet implemented.";
  return false;
}

bool QtopiaFormat::fromRawString( Calendar *, const QByteArray & )
{
  kDebug() << "not yet implemented.";
  return false;
}

QString QtopiaFormat::toString( Calendar * )
{
  return QString();
}
