/*
  This file is part of the kcal library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "incidence.h"
#include "calformat.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtCore/QList>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Incidence::Private
{
  public:
    Private()
      : mRecurrence( 0 ),
        mStatus( StatusNone ),
        mSecrecy( SecrecyPublic ),
        mPriority( 0 ),
        mRelatedTo( 0 )
    {}

    KDateTime mCreated;              // creation datetime
    int mRevision;                   // revision number

    QString mDescription;            // description string
    QString mSummary;                // summary string
    QString mLocation;               // location string
    QStringList mCategories;         // category list
    mutable Recurrence *mRecurrence; // recurrence
    Attachment::List mAttachments;   // attachments list
    Alarm::List mAlarms;             // alarms list
    QStringList mResources;          // resources list
    Status mStatus;                  // status
    QString mStatusString;           // status string, for custom status
    int mSecrecy;                    // secrecy
    int mPriority;                   // priority: 1 = highest, 2 = less, etc.
    QString mSchedulingID;           // ID for scheduling mails

    Incidence *mRelatedTo;           // incidence this is related to
    QString mRelatedToUid;           // incidence (by Uid) this is related to
    Incidence::List mRelations;      // a list of incidences this is related to
};
//@endcond

Incidence::Incidence()
  : IncidenceBase(), d( new KCal::Incidence::Private )
{
  recreate();

  d->mAlarms.setAutoDelete( true );
  d->mAttachments.setAutoDelete( true );
}

Incidence::Incidence( const Incidence &i )
  : IncidenceBase( i ), Recurrence::RecurrenceObserver(),
    d( new KCal::Incidence::Private )
{
// TODO: reenable attributes currently commented out.
  d->mRevision = i.d->mRevision;
  d->mCreated = i.d->mCreated;
  d->mDescription = i.d->mDescription;
  d->mSummary = i.d->mSummary;
  d->mCategories = i.d->mCategories;
//  Incidence *mRelatedTo;          Incidence *mRelatedTo;
  d->mRelatedTo = 0;
  d->mRelatedToUid = i.d->mRelatedToUid;
//  Incidence::List mRelations;    Incidence::List mRelations;
  d->mResources = i.d->mResources;
  d->mStatusString = i.d->mStatusString;
  d->mStatus = i.d->mStatus;
  d->mSecrecy = i.d->mSecrecy;
  d->mPriority = i.d->mPriority;
  d->mLocation = i.d->mLocation;

  // Alarms and Attachments are stored in ListBase<...>, which is a QValueList<...*>.
  // We need to really duplicate the objects stored therein, otherwise deleting
  // i will also delete all attachments from this object (setAutoDelete...)
  Alarm::List::ConstIterator it;
  for ( it = i.d->mAlarms.begin(); it != i.d->mAlarms.end(); ++it ) {
    Alarm *b = new Alarm( **it );
    b->setParent( this );
    d->mAlarms.append( b );
  }
  d->mAlarms.setAutoDelete( true );

  Attachment::List::ConstIterator it1;
  for ( it1 = i.d->mAttachments.begin(); it1 != i.d->mAttachments.end(); ++it1 ) {
    Attachment *a = new Attachment( **it1 );
    d->mAttachments.append( a );
  }
  d->mAttachments.setAutoDelete( true );

  if ( i.d->mRecurrence ) {
    d->mRecurrence = new Recurrence( *( i.d->mRecurrence ) );
    d->mRecurrence->addObserver( this );
  } else {
    d->mRecurrence = 0;
  }

  d->mSchedulingID = i.d->mSchedulingID;
}

Incidence::~Incidence()
{
  Incidence::List Relations = d->mRelations;
  List::ConstIterator it;
  for ( it = Relations.begin(); it != Relations.end(); ++it ) {
    if ( (*it)->relatedTo() == this ) {
      (*it)->d->mRelatedTo = 0;
    }
  }
  if ( relatedTo() ) {
    relatedTo()->removeRelation( this );
  }

  delete d->mRecurrence;
  delete d;
}

// A string comparison that considers that null and empty are the same
static bool stringCompare( const QString &s1, const QString &s2 )
{
  return
    ( s1.isEmpty() && s2.isEmpty() ) || ( s1 == s2 );
}

bool Incidence::operator==( const Incidence &i2 ) const
{
  if ( alarms().count() != i2.alarms().count() ) {
    return false; // no need to check further
  }

  Alarm::List::ConstIterator a1 = alarms().begin();
  Alarm::List::ConstIterator a2 = i2.alarms().begin();
  for ( ; a1 != alarms().end() && a2 != i2.alarms().end(); ++a1, ++a2 ) {
    if ( **a1 == **a2 ) {
      continue;
    } else {
      return false;
    }
  }

  if ( !IncidenceBase::operator==( i2 ) ) {
    return false;
  }

  bool recurrenceEqual = ( d->mRecurrence == 0 && i2.d->mRecurrence == 0 );
  if ( !recurrenceEqual ) {
    recurrenceEqual = d->mRecurrence != 0 &&
                      i2.d->mRecurrence != 0 &&
                      *d->mRecurrence == *i2.d->mRecurrence;
  }

  return
    recurrenceEqual &&
    created() == i2.created() &&
    stringCompare( description(), i2.description() ) &&
    stringCompare( summary(), i2.summary() ) &&
    categories() == i2.categories() &&
    // no need to compare mRelatedTo
    stringCompare( relatedToUid(), i2.relatedToUid() ) &&
    relations() == i2.relations() &&
    attachments() == i2.attachments() &&
    resources() == i2.resources() &&
    d->mStatus == i2.d->mStatus &&
    ( d->mStatus == StatusNone ||
      stringCompare( d->mStatusString, i2.d->mStatusString ) ) &&
    secrecy() == i2.secrecy() &&
    priority() == i2.priority() &&
    stringCompare( location(), i2.location() ) &&
    stringCompare( schedulingID(), i2.schedulingID() );
}

void Incidence::recreate()
{
  KDateTime nowUTC = KDateTime::currentUtcDateTime();
  setCreated( nowUTC );

  setUid( CalFormat::createUniqueId() );
  setSchedulingID( QString() );

  setRevision( 0 );

  setLastModified( nowUTC );
}

void Incidence::setReadOnly( bool readOnly )
{
  IncidenceBase::setReadOnly( readOnly );
  if ( d->mRecurrence ) {
    d->mRecurrence->setRecurReadOnly( readOnly );
  }
}

void Incidence::setFloats( bool f )
{
  if ( mReadOnly ) {
    return;
  }
  if ( recurrence() ) {
    recurrence()->setFloats( f );
  }
  IncidenceBase::setFloats( f );
}

void Incidence::setCreated( const KDateTime &created )
{
  if ( mReadOnly ) {
    return;
  }

  d->mCreated = created.toUtc();

// FIXME: Shouldn't we call updated for the creation date, too?
//  updated();
}

KDateTime Incidence::created() const
{
  return d->mCreated;
}

void Incidence::setRevision( int rev )
{
  if ( mReadOnly ) {
    return;
  }

  d->mRevision = rev;

  updated();
}

int Incidence::revision() const
{
  return d->mRevision;
}

void Incidence::setDtStart( const KDateTime &dtStart )
{
  if ( d->mRecurrence ) {
    d->mRecurrence->setStartDateTime( dtStart );
    d->mRecurrence->setFloats( floats() );
  }
  IncidenceBase::setDtStart( dtStart );
}

void Incidence::shiftTimes( const KDateTime::Spec &oldSpec,
                            const KDateTime::Spec &newSpec )
{
  IncidenceBase::shiftTimes( oldSpec, newSpec );
  if ( d->mRecurrence ) {
    d->mRecurrence->shiftTimes( oldSpec, newSpec );
  }
  for ( int i = 0, end = d->mAlarms.count();  i < end;  ++i ) {
    d->mAlarms[i]->shiftTimes( oldSpec, newSpec );
  }
}

void Incidence::setDescription( const QString &description )
{
  if ( mReadOnly ) {
    return;
  }
  d->mDescription = description;
  updated();
}

QString Incidence::description() const
{
  return d->mDescription;
}

void Incidence::setSummary( const QString &summary )
{
  if ( mReadOnly ) {
    return;
  }
  d->mSummary = summary;
  updated();
}

QString Incidence::summary() const
{
  return d->mSummary;
}

void Incidence::setCategories( const QStringList &categories )
{
  if ( mReadOnly ) {
    return;
  }
  d->mCategories = categories;
  updated();
}

// TODO: remove setCategories(QString) function
void Incidence::setCategories( const QString &catStr )
{
  if ( mReadOnly ) {
    return;
  }
  d->mCategories.clear();

  if ( catStr.isEmpty() ) {
    return;
  }

  d->mCategories = catStr.split( "," );

  QStringList::Iterator it;
  for ( it = d->mCategories.begin();it != d->mCategories.end(); ++it ) {
    *it = (*it).trimmed();
  }

  updated();
}

QStringList Incidence::categories() const
{
  return d->mCategories;
}

QString Incidence::categoriesStr() const
{
  return d->mCategories.join( "," );
}

void Incidence::setRelatedToUid( const QString &relatedToUid )
{
  if ( mReadOnly || d->mRelatedToUid == relatedToUid ) {
    return;
  }
  d->mRelatedToUid = relatedToUid;
  updated();
}

QString Incidence::relatedToUid() const
{
  return d->mRelatedToUid;
}

void Incidence::setRelatedTo( Incidence *relatedTo )
{
  if ( mReadOnly || d->mRelatedTo == relatedTo ) {
    return;
  }
  if ( d->mRelatedTo ) {
    d->mRelatedTo->removeRelation( this );
  }
  d->mRelatedTo = relatedTo;
  if ( d->mRelatedTo ) {
    d->mRelatedTo->addRelation( this );
    if ( d->mRelatedTo->uid() != d->mRelatedToUid ) {
      setRelatedToUid( d->mRelatedTo->uid() );
    }
  } else {
    setRelatedToUid( QString() );
  }
}

Incidence *Incidence::relatedTo() const
{
  return d->mRelatedTo;
}

Incidence::List Incidence::relations() const
{
  return d->mRelations;
}

void Incidence::addRelation( Incidence *incidence )
{
  if ( !d->mRelations.contains( incidence ) ) {
    d->mRelations.append( incidence );
  }
}

void Incidence::removeRelation( Incidence *incidence )
{
  kDebug() << "Entering Incidence::removeRelation " << endl;
  d->mRelations.removeRef( incidence );
  d->mRelatedToUid = QString();
//  if (incidence->getRelatedTo() == this) incidence->setRelatedTo(0);
}

// %%%%%%%%%%%%  Recurrence-related methods %%%%%%%%%%%%%%%%%%%%

Recurrence *Incidence::recurrence() const
{
  if ( !d->mRecurrence ) {
    d->mRecurrence = new Recurrence();
    d->mRecurrence->setStartDateTime( IncidenceBase::dtStart() );
    d->mRecurrence->setFloats( floats() );
    d->mRecurrence->setRecurReadOnly( mReadOnly );
    d->mRecurrence->addObserver( const_cast<KCal::Incidence*>( this ) );
  }

  return d->mRecurrence;
}

void Incidence::clearRecurrence()
{
  delete d->mRecurrence;
  d->mRecurrence = 0;
}

uint Incidence::recurrenceType() const
{
  if ( d->mRecurrence ) {
    return d->mRecurrence->recurrenceType();
  } else {
    return Recurrence::rNone;
  }
}

bool Incidence::recurs() const
{
  if ( d->mRecurrence ) {
    return d->mRecurrence->recurs();
  } else {
    return false;
  }
}

bool Incidence::recursOn( const QDate &qd,
                          const KDateTime::Spec &timeSpec ) const
{
  return d->mRecurrence && d->mRecurrence->recursOn( qd, timeSpec );
}

bool Incidence::recursAt( const KDateTime &qdt ) const
{
  return d->mRecurrence && d->mRecurrence->recursAt( qdt );
}

/**
  Calculates the start date/time for all recurrences that happen at some time
  on the given date (might start before that date, but end on or after the
  given date).
  @param date the date where the incidence should occur
  @return the start date/time of all occurrences that overlap with the given
      date. Empty list if the incidence does not overlap with the date at all
*/
QList<KDateTime> Incidence::startDateTimesForDate( const QDate &date,
                                                   const KDateTime::Spec &timeSpec ) const
{
  KDateTime start = dtStart();
  KDateTime end = endDateRecurrenceBase();

  QList<KDateTime> result;

  // TODO_Recurrence: Also work if only due date is given...
  if ( !start.isValid() && ! end.isValid() ) {
    return result;
  }

  // if the incidence doesn't recur,
  KDateTime kdate( date, timeSpec );
  if ( !recurs() ) {
    if ( !( start > kdate || end < kdate ) ) {
      result << start;
    }
    return result;
  }

  int days = start.daysTo( end );
  // Account for possible recurrences going over midnight, while the original event doesn't
  QDate tmpday( date.addDays( -days - 1 ) );
  KDateTime tmp;
  while ( tmpday <= date ) {
    if ( recurrence()->recursOn( tmpday, timeSpec ) ) {
      QList<QTime> times = recurrence()->recurTimesOn( tmpday, timeSpec );
      for ( QList<QTime>::ConstIterator it = times.begin(); it != times.end(); ++it ) {
        tmp = KDateTime( tmpday, *it, start.timeSpec() );
        if ( endDateForStart( tmp ) >= kdate ) {
          result << tmp;
        }
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

/**
  Calculates the start date/time for all recurrences that happen at the given
  time.
  @param datetime the date/time where the incidence should occur
  @return the start date/time of all occurrences that overlap with the given
      date/time. Empty list if the incidence does not happen at the given
      time at all.
*/
QList<KDateTime> Incidence::startDateTimesForDateTime( const KDateTime &datetime ) const
{
  KDateTime start = dtStart();
  KDateTime end = endDateRecurrenceBase();

  QList<KDateTime> result;

  // TODO_Recurrence: Also work if only due date is given...
  if ( !start.isValid() && ! end.isValid() ) {
    return result;
  }

  // if the incidence doesn't recur,
  if ( !recurs() ) {
    if ( !( start > datetime || end < datetime ) ) {
      result << start;
    }
    return result;
  }

  int days = start.daysTo( end );
  // Account for possible recurrences going over midnight, while the original event doesn't
  QDate tmpday( datetime.date().addDays( -days - 1 ) );
  KDateTime tmp;
  while ( tmpday <= datetime.date() ) {
    if ( recurrence()->recursOn( tmpday, datetime.timeSpec() ) ) {
      // Get the times during the day (in start date's time zone) when recurrences happen
      QList<QTime> times = recurrence()->recurTimesOn( tmpday, start.timeSpec() );
      for ( QList<QTime>::ConstIterator it = times.begin(); it != times.end(); ++it ) {
        tmp = KDateTime( tmpday, *it, start.timeSpec() );
        if ( !( tmp > datetime || endDateForStart( tmp ) < datetime ) ) {
          result << tmp;
        }
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

/** Return the end time of the occurrence if it starts at the given date/time */
KDateTime Incidence::endDateForStart( const KDateTime &startDt ) const
{
  KDateTime start = dtStart();
  KDateTime end = endDateRecurrenceBase();
  if ( !end.isValid() ) {
    return start;
  }
  if ( !start.isValid() ) {
    return end;
  }

  return startDt.addSecs( start.secsTo( end ) );
}

// %%%%%%%%%%%%%%%%% begin:RecurrenceRule %%%%%%%%%%%%%%%%%

// Exception Dates
/*void Incidence::setExDates(const DateList &exDates)
{
  if ( mReadOnly ) return;
  recurrence()->setExDates( exDates );
  updated();
}

void Incidence::addExDate( const QDate &date )
{
  if ( mReadOnly ) return;
  recurrence()->addExDate( date );
  updated();
}

DateList Incidence::exDates() const
{
  if ( d->mRecurrence ) return d->mRecurrence->exDates();
  else return DateList();
}

// Exception DateTimes
void Incidence::setExDateTimes( const DateTimeList &exDates )
{
  if ( mReadOnly ) return;
  recurrence()->setExDateTimes( exDates );
  updated();
}

void Incidence::addExDateTime( const KDateTime &date )
{
  if ( mReadOnly ) return;
  recurrence()->addExDateTime( date );
  updated();
}

DateTimeList Incidence::exDateTimes() const
{
  if ( d->mRecurrence ) return d->mRecurrence->exDateTimes();
  else return DateTimeList();
}

// Recurrence Dates
void Incidence::setRDates(const DateList &exDates)
{
  if ( mReadOnly ) return;
  recurrence()->setRDates( exDates );
  updated();
}

void Incidence::addRDate( const QDate &date )
{
  if ( mReadOnly ) return;
  recurrence()->addRDate( date );
  updated();
}

DateList Incidence::rDates() const
{
  if ( d->mRecurrence ) return d->mRecurrence->rDates();
  else return DateList();
}

// Recurrence DateTimes
void Incidence::setRDateTimes( const DateTimeList &exDates )
{
  if ( mReadOnly ) return;
  recurrence()->setRDateTimes( exDates );
  updated();
}

void Incidence::addRDateTime( const KDateTime &date )
{
  if ( mReadOnly ) return;
  recurrence()->addRDateTime( date );
  updated();
}

DateTimeList Incidence::rDateTimes() const
{
  if ( d->mRecurrence ) return d->mRecurrence->rDateTimes();
  else return DateTimeList();
}*/

// %%%%%%%%%%%%%%%%% end:RecurrenceRule %%%%%%%%%%%%%%%%%

void Incidence::addAttachment( Attachment *attachment )
{
  if ( mReadOnly || !attachment ) {
    return;
  }

  d->mAttachments.append( attachment );
  updated();
}

void Incidence::deleteAttachment( Attachment *attachment )
{
  d->mAttachments.removeRef( attachment );
}

void Incidence::deleteAttachments( const QString &mime )
{
  Attachment::List::Iterator it = d->mAttachments.begin();
  while ( it != d->mAttachments.end() ) {
    if ( (*it)->mimeType() == mime ) {
      d->mAttachments.erase( it );
    } else {
      ++it;
    }
  }
}

Attachment::List Incidence::attachments() const
{
  return d->mAttachments;
}

Attachment::List Incidence::attachments( const QString &mime ) const
{
  Attachment::List attachments;
  Attachment::List::ConstIterator it;
  for ( it = d->mAttachments.begin(); it != d->mAttachments.end(); ++it ) {
    if ( (*it)->mimeType() == mime ) {
      attachments.append( *it );
    }
  }
  return attachments;
}

void Incidence::clearAttachments()
{
  d->mAttachments.clear();
}

void Incidence::setResources( const QStringList &resources )
{
  if ( mReadOnly ) {
    return;
  }

  d->mResources = resources;
  updated();
}

QStringList Incidence::resources() const
{
  return d->mResources;
}

void Incidence::setPriority( int priority )
{
  if ( mReadOnly ) {
    return;
  }

  d->mPriority = priority;
  updated();
}

int Incidence::priority() const
{
  return d->mPriority;
}

void Incidence::setStatus( Incidence::Status status )
{
  if ( mReadOnly || status == StatusX ) {
    return;
  }

  d->mStatus = status;
  d->mStatusString.clear();
  updated();
}

void Incidence::setCustomStatus( const QString &status )
{
  if ( mReadOnly ) {
    return;
  }

  d->mStatus = status.isEmpty() ? StatusNone : StatusX;
  d->mStatusString = status;
  updated();
}

Incidence::Status Incidence::status() const
{
  return d->mStatus;
}

QString Incidence::statusStr() const
{
  if ( d->mStatus == StatusX ) {
    return d->mStatusString;
  }

  return statusName( d->mStatus );
}

QString Incidence::statusName( Incidence::Status status )
{
  switch ( status ) {
  case StatusTentative:
    return i18nc( "incidence status", "Tentative" );
  case StatusConfirmed:
    return i18n( "Confirmed" );
  case StatusCompleted:
    return i18n( "Completed" );
  case StatusNeedsAction:
    return i18n( "Needs-Action" );
  case StatusCanceled:
    return i18n( "Canceled" );
  case StatusInProcess:
    return i18n( "In-Process" );
  case StatusDraft:
    return i18n( "Draft" );
  case StatusFinal:
    return i18n( "Final" );
  case StatusX:
  case StatusNone:
  default:
    return QString();
  }
}

void Incidence::setSecrecy( int sec )
{
  if ( mReadOnly ) {
    return;
  }

  d->mSecrecy = sec;
  updated();
}

int Incidence::secrecy() const
{
  return d->mSecrecy;
}

QString Incidence::secrecyStr() const
{
  return secrecyName( d->mSecrecy );
}

QString Incidence::secrecyName( int secrecy )
{
  switch ( secrecy ) {
  case SecrecyPublic:
    return i18n( "Public" );
  case SecrecyPrivate:
    return i18n( "Private" );
  case SecrecyConfidential:
    return i18n( "Confidential" );
  default:
    return i18n( "Undefined" );
  }
}

QStringList Incidence::secrecyList()
{
  QStringList list;
  list << secrecyName( SecrecyPublic );
  list << secrecyName( SecrecyPrivate );
  list << secrecyName( SecrecyConfidential );

  return list;
}

const Alarm::List &Incidence::alarms() const
{
  return d->mAlarms;
}

Alarm *Incidence::newAlarm()
{
  Alarm *alarm = new Alarm( this );
  d->mAlarms.append( alarm );
  return alarm;
}

void Incidence::addAlarm( Alarm *alarm )
{
  d->mAlarms.append( alarm );
  updated();
}

void Incidence::removeAlarm( Alarm *alarm )
{
  d->mAlarms.removeRef( alarm );
  updated();
}

void Incidence::clearAlarms()
{
  d->mAlarms.clear();
  updated();
}

bool Incidence::isAlarmEnabled() const
{
  Alarm::List::ConstIterator it;
  for ( it = d->mAlarms.begin(); it != d->mAlarms.end(); ++it ) {
    if ( (*it)->enabled() ) {
      return true;
    }
  }
  return false;
}

void Incidence::setLocation( const QString &location )
{
  if ( mReadOnly ) {
    return;
  }

  d->mLocation = location;
  updated();
}

QString Incidence::location() const
{
  return d->mLocation;
}

void Incidence::setSchedulingID( const QString &sid )
{
  d->mSchedulingID = sid;
}

QString Incidence::schedulingID() const
{
  if ( d->mSchedulingID.isNull() ) {
    // Nothing set, so use the normal uid
    return uid();
  }
  return d->mSchedulingID;
}

/** Observer interface for the recurrence class. If the recurrence is changed,
    this method will be called for the incidence the recurrence object
    belongs to. */
void Incidence::recurrenceUpdated( Recurrence *recurrence )
{
  if ( recurrence == d->mRecurrence ) {
    updated();
  }
}
