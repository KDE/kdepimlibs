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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Incidence class.

  @brief
  Provides the class common to non-FreeBusy (Events, To-dos, Journals)
  calendar components known as incidences.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "incidence.h"
#include "calformat.h"

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <QtCore/QList>
#include <QTextDocument> // for Qt::escape() and Qt::mightBeRichText()

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
      : mDescriptionIsRich( false ),
        mSummaryIsRich( false ),
        mLocationIsRich( false ),
        mRecurrence( 0 ),
        mStatus( StatusNone ),
        mSecrecy( SecrecyPublic ),
        mPriority( 0 ),
        mRelatedTo( 0 )
    {
      mAlarms.setAutoDelete( true );
      mAttachments.setAutoDelete( true );
    }

    Private( const Private &p )
      : mCreated( p.mCreated ),
        mRevision( p.mRevision ),
        mDescription( p.mDescription ),
        mDescriptionIsRich( p.mDescriptionIsRich ),
        mSummary( p.mSummary ),
        mSummaryIsRich( p.mSummaryIsRich ),
        mLocation( p.mLocation ),
        mLocationIsRich( p.mLocationIsRich ),
        mCategories( p.mCategories ),
        mRecurrence( p.mRecurrence ),
        mResources( p.mResources ),
        mStatus( p.mStatus ),
        mStatusString( p.mStatusString ),
        mSecrecy( p.mSecrecy ),
        mPriority( p.mPriority ),
        mSchedulingID( p.mSchedulingID ),
        mRelatedTo( 0 ),
        mRelatedToUid( p.mRelatedToUid )
// TODO: reenable attributes currently commented out.
//  Incidence *mRelatedTo;          Incidence *mRelatedTo;
//  Incidence::List mRelations;    Incidence::List mRelations;
    {
      mAlarms.setAutoDelete( true );
      mAttachments.setAutoDelete( true );
    }

    void clear()
    {
      mAlarms.clearAll();
      mAttachments.clearAll();
      delete mRecurrence;
    }

    KDateTime mCreated;              // creation datetime
    int mRevision;                   // revision number

    QString mDescription;            // description string
    bool mDescriptionIsRich;         // description string is richtext.
    QString mSummary;                // summary string
    bool mSummaryIsRich;             // summary string is richtext.
    QString mLocation;               // location string
    bool mLocationIsRich;            // location string is richtext.
    QStringList mCategories;         // category list
    mutable Recurrence *mRecurrence; // recurrence
    Attachment::List mAttachments;   // attachments list
    Alarm::List mAlarms;             // alarms list
    QStringList mResources;          // resources list (not calendar resources)
    Status mStatus;                  // status
    QString mStatusString;           // status string, for custom status
    Secrecy mSecrecy;                // secrecy
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
}

Incidence::Incidence( const Incidence &i )
  : IncidenceBase( i ),
    Recurrence::RecurrenceObserver(),
    d( new KCal::Incidence::Private( *i.d ) )
{
  init( i );
}

void Incidence::init( const Incidence &i )
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
  foreach ( Alarm* alarm, i.d->mAlarms ) {
    Alarm *b = new Alarm( *alarm );
    b->setParent( this );
    d->mAlarms.append( b );
  }

  foreach ( Attachment* attachment, i.d->mAttachments ) {
    Attachment *a = new Attachment( *attachment );
    d->mAttachments.append( a );
  }

  if ( i.d->mRecurrence ) {
    d->mRecurrence = new Recurrence( *( i.d->mRecurrence ) );
    d->mRecurrence->addObserver( this );
  } else {
    d->mRecurrence = 0;
  }
}

Incidence::~Incidence()
{
  Incidence::List relations = d->mRelations;
  foreach ( Incidence* incidence, relations ) {
    if ( incidence->relatedTo() == this ) {
      incidence->setRelatedTo(0);
    }
  }

  if ( relatedTo() ) {
    relatedTo()->removeRelation( this );
  }
  delete d->mRecurrence;
  delete d;
}

//@cond PRIVATE
// A string comparison that considers that null and empty are the same
static bool stringCompare( const QString &s1, const QString &s2 )
{
  return ( s1.isEmpty() && s2.isEmpty() ) || ( s1 == s2 );
}

//@endcond
Incidence &Incidence::operator=( const Incidence &other )
{
  // check for self assignment
  if ( &other == this )
    return *this;

  d->clear();
  //TODO: should relations be cleared out, as in destructor???
  IncidenceBase::operator=( other );
  init( other );
  return *this;
}

bool Incidence::operator==( const Incidence &i2 ) const
{
  if ( alarms().count() != i2.alarms().count() ) {
    return false; // no need to check further
  }

  Alarm::List::ConstIterator a1 = alarms().constBegin();
  Alarm::List::ConstIterator a1end = alarms().constEnd();
  Alarm::List::ConstIterator a2 = i2.alarms().begin();
  Alarm::List::ConstIterator a2end = i2.alarms().constEnd();
  for ( ; a1 != a1end && a2 != a2end; ++a1, ++a2 ) {
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

void Incidence::setAllDay( bool allDay )
{
  if ( mReadOnly ) {
    return;
  }
  if ( recurrence() ) {
    recurrence()->setAllDay( allDay );
  }
  IncidenceBase::setAllDay( allDay );
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

void Incidence::setDtStart( const KDateTime &dt )
{
  if ( d->mRecurrence ) {
    d->mRecurrence->setStartDateTime( dt );
    d->mRecurrence->setAllDay( allDay() );
  }
  IncidenceBase::setDtStart( dt );
}

KDateTime Incidence::dtEnd() const
{
  return KDateTime();
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

void Incidence::setDescription( const QString &description, bool isRich )
{
  if ( mReadOnly ) {
    return;
  }
  d->mDescription = description;
  d->mDescriptionIsRich = isRich;
  updated();
}

void Incidence::setDescription( const QString &description )
{
  setDescription( description, Qt::mightBeRichText( description ) );
}

QString Incidence::description() const
{
  return d->mDescription;
}

QString Incidence::richDescription() const
{
  if ( descriptionIsRich() ) {
    return d->mDescription;
  } else {
    return Qt::escape( d->mDescription ).replace( '\n', "<br/>" );
  }
}

bool Incidence::descriptionIsRich() const
{
  return d->mDescriptionIsRich;
}

void Incidence::setSummary( const QString &summary, bool isRich )
{
  if ( mReadOnly ) {
    return;
  }
  d->mSummary = summary;
  d->mSummaryIsRich = isRich;
  updated();
}

void Incidence::setSummary( const QString &summary )
{
  setSummary( summary, Qt::mightBeRichText( summary ) );
}

QString Incidence::summary() const
{
  return d->mSummary;
}

QString Incidence::richSummary() const
{
  if ( summaryIsRich() ) {
    return d->mSummary;
  } else {
    return Qt::escape( d->mSummary ).replace( '\n', "<br/>" );
  }
}

bool Incidence::summaryIsRich() const
{
  return d->mSummaryIsRich;
}

void Incidence::setCategories( const QStringList &categories )
{
  if ( mReadOnly ) {
    return;
  }
  d->mCategories = categories;
  updated();
}

void Incidence::setCategories( const QString &catStr )
{
  if ( mReadOnly ) {
    return;
  }
  d->mCategories.clear();

  if ( catStr.isEmpty() ) {
    return;
  }

  d->mCategories = catStr.split( ',' );

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
  if ( d->mRelatedToUid == relatedToUid ) {
    return;
  }
  d->mRelatedToUid = relatedToUid;
  updated();
}

QString Incidence::relatedToUid() const
{
  return d->mRelatedToUid;
}

void Incidence::setRelatedTo( Incidence *incidence )
{
  if ( d->mRelatedTo == incidence ) {
    return;
  }
  if ( d->mRelatedTo ) {
    d->mRelatedTo->removeRelation( this );
  }
  d->mRelatedTo = incidence;
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
  d->mRelations.removeRef( incidence );
  if ( d->mRelatedToUid == incidence->uid() ) {
    d->mRelatedToUid.clear();
  }
//  if (incidence->getRelatedTo() == this) incidence->setRelatedTo(0);
}

// %%%%%%%%%%%%  Recurrence-related methods %%%%%%%%%%%%%%%%%%%%

Recurrence *Incidence::recurrence() const
{
  if ( !d->mRecurrence ) {
    d->mRecurrence = new Recurrence();
    d->mRecurrence->setStartDateTime( IncidenceBase::dtStart() );
    d->mRecurrence->setAllDay( allDay() );
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

ushort Incidence::recurrenceType() const
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

bool Incidence::recursOn( const QDate &date,
                          const KDateTime::Spec &timeSpec ) const
{
  return d->mRecurrence && d->mRecurrence->recursOn( date, timeSpec );
}

bool Incidence::recursAt( const KDateTime &qdt ) const
{
  return d->mRecurrence && d->mRecurrence->recursAt( qdt );
}

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
      foreach ( const QTime& time, times ) {
        tmp = KDateTime( tmpday, time, start.timeSpec() );
        if ( endDateForStart( tmp ) >= kdate ) {
          result << tmp;
        }
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

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
      foreach ( const QTime& time, times ) {
        tmp = KDateTime( tmpday, time, start.timeSpec() );
        if ( !( tmp > datetime || endDateForStart( tmp ) < datetime ) ) {
          result << tmp;
        }
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

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
      d->mAttachments.removeRef( it );
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
  foreach ( Attachment* attachment, d->mAttachments ) {
    if ( attachment->mimeType() == mime ) {
      attachments.append( attachment );
    }
  }
  return attachments;
}

void Incidence::clearAttachments()
{
  d->mAttachments.clearAll();
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
    return i18nc( "@item event is tentative", "Tentative" );
  case StatusConfirmed:
    return i18nc( "@item event is definite", "Confirmed" );
  case StatusCompleted:
    return i18nc( "@item to-do is complete", "Completed" );
  case StatusNeedsAction:
    return i18nc( "@item to-do needs action", "Needs-Action" );
  case StatusCanceled:
    return i18nc( "@item event orto-do is canceled; journal is removed", "Canceled" );
  case StatusInProcess:
    return i18nc( "@item to-do is in process", "In-Process" );
  case StatusDraft:
    return i18nc( "@item journal is in draft form", "Draft" );
  case StatusFinal:
    return i18nc( "@item journal is in final form", "Final" );
  case StatusX:
  case StatusNone:
  default:
    return QString();
  }
}

void Incidence::setSecrecy( Incidence::Secrecy secrecy )
{
  if ( mReadOnly ) {
    return;
  }

  d->mSecrecy = secrecy;
  updated();
}

Incidence::Secrecy Incidence::secrecy() const
{
  return d->mSecrecy;
}

QString Incidence::secrecyStr() const
{
  return secrecyName( d->mSecrecy );
}

QString Incidence::secrecyName( Incidence::Secrecy secrecy )
{
  switch ( secrecy ) {
  case SecrecyPublic:
    return i18nc( "@item incidence access if for everyone", "Public" );
  case SecrecyPrivate:
    return i18nc( "@item incidence access is by owner only", "Private" );
  case SecrecyConfidential:
    return i18nc( "@item incidence access is by owner and a controlled group", "Confidential" );
  default:
    return QString();  // to make compilers happy
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
  d->mAlarms.clearAll();
  updated();
}

bool Incidence::isAlarmEnabled() const
{
  foreach ( Alarm* alarm, d->mAlarms ) {
    if ( alarm->enabled() ) {
      return true;
    }
  }
  return false;
}

void Incidence::setLocation( const QString &location, bool isRich )
{
  if ( mReadOnly ) {
    return;
  }

  d->mLocation = location;
  d->mLocationIsRich = isRich;
  updated();
}

void Incidence::setLocation( const QString &location )
{
  setLocation( location, Qt::mightBeRichText( location ) );
}

QString Incidence::location() const
{
  return d->mLocation;
}

QString Incidence::richLocation() const
{
  if ( locationIsRich() ) {
    return d->mLocation;
  } else {
    return Qt::escape( d->mLocation ).replace( '\n', "<br/>" );
  }
}

bool Incidence::locationIsRich() const
{
  return d->mLocationIsRich;
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
