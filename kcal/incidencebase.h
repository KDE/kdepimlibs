/*
    This file is part of the kcal library.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
  This file is part of the API for handling calendar data and
  defines the IncidenceBase class.

  @author Cornelius Schumacher
  @author Reinhold Kainhofer
*/

#ifndef KCAL_INCIDENCEBASE_H
#define KCAL_INCIDENCEBASE_H

#include <QStringList>
#include <QByteArray>
#include <QList>

#include <kdatetime.h>

#include "attendee.h"
#include "customproperties.h"

class KUrl;

namespace KCal {

/** List of dates */
typedef QList<QDate> DateList;
/** List of times */
typedef QList<KDateTime> DateTimeList;
class Event;
class Todo;
class Journal;
class FreeBusy;

/**
  @brief
  This class provides the base class common to all calendar components.

  define: floats
  define: organizer (person)

  Several properties are not allowed for VFREEBUSY objects (see rfc:2445),
  so they are not in IncidenceBase. The hierarchy is:

  IncidenceBase
  - FreeBusy
  - Incidence
  - Event
  - Todo
  - Journal

  So IncidenceBase contains all properties that are common to all classes,
  and Incidence contains all additional properties that are common to
  Events, Todos and Journals, but are not allowed for FreeBusy entries.
*/
class KCAL_EXPORT IncidenceBase : public CustomProperties
{
  public:
    /**
      This class provides the interface for a visitor of calendar components.
      It serves as base class for concrete visitors, which implement certain
      actions on calendar components. It allows to add functions, which operate
      on the concrete types of calendar components, without changing the
      calendar component classes.
    */
    class Visitor
    {
      public:
        /** Destruct Incidence::Visitor */
        virtual ~Visitor() {}

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on an Event object.
        */
        virtual bool visit( Event *event )
        { Q_UNUSED( event ); return false; }

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on a Todo object.
        */
        virtual bool visit( Todo *todo )
        { Q_UNUSED ( todo ); return false; }

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on an Journal object.
        */
        virtual bool visit( Journal *journal )
        { Q_UNUSED( journal ); return false; }

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on a FreeBusy object.
        */
        virtual bool visit( FreeBusy *freebusy )
        { Q_UNUSED( freebusy ); return false; }

      protected:
        /**
          Constructor is protected to prevent direct creation of visitor
          base class.
        */
        Visitor() {}
    };

    class Observer {
      public:
        virtual ~Observer() {}
        virtual void incidenceUpdated( IncidenceBase * ) = 0;
    };

    IncidenceBase();
    IncidenceBase( const IncidenceBase & );
    virtual ~IncidenceBase();

    bool operator==( const IncidenceBase & ) const;

    /**
      Accept IncidenceVisitor. A class taking part in the visitor mechanism
      has to provide this implementation:
      <pre>
        bool accept(Visitor &v) { return v.visit(this); }
      </pre>
    */
    virtual bool accept( Visitor &v )
    { Q_UNUSED( v ); return false; }

    virtual QByteArray type() const = 0;

    /**
      Sets the unique id for the incidence to @p uid.
    */
    void setUid( const QString &uid );

    /**
      Returns the unique id for the incidence
    */
    QString uid() const;

    /**
      Returns the uri for the incidence, of form urn:x-ical:\<uid\>
    */
    KUrl uri() const;

    /** Sets the time the incidence was last modified. It is stored as a UTC date/time. */
    /**
      Sets the time the incidence was last modified to @p lm.
    */
    void setLastModified( const KDateTime &lm );

    /**
      Returns the time the incidence was last modified.
    */
    KDateTime lastModified() const;

    /**
      Sets the organizer for the incidence.
    */
    void setOrganizer( const Person &organizer );

    /**
      Sets the incidence organizer to any string @p organizer.

      @param organizer is any string to set the incidence organizer.
    */
    void setOrganizer( const QString &organizer );

    /**
      Returns the #Person associated with this incidence.

      @see setOrganizer()
    */
    Person organizer() const;

    /**
      Sets readonly status.
    */
    virtual void setReadOnly( bool );

    /**
      Returns if the object is read-only.
    */
    bool isReadOnly() const { return mReadOnly; }

    /**
      Sets the incidence's starting date/time with a KDateTime.
      The incidence's floating status is set according to whether @p dtStart
      is a date/time (not floating) or date-only (floating).
    */
    virtual void setDtStart( const KDateTime &dtStart );
    virtual KDE_DEPRECATED void setDtStart( const QDateTime &dtStart )
    { setDtStart( KDateTime( dtStart ) ); }  // use local time zone

    /**
      Returns an incidence's starting date/time as a KDateTime.
    */
    virtual KDateTime dtStart() const;

    /**
      Returns an incidence's starting time as a string formatted according
      to the user's locale settings.
    */
    virtual QString dtStartTimeStr() const;

    /**
      Returns an incidence's starting date as a string formatted according
      to the user's locale settings.
    */
    virtual QString dtStartDateStr( bool shortfmt = true ) const;

    /**
      Returns an incidence's starting date and time as a string formatted
      according to the user's locale settings.
    */
    virtual QString dtStartStr() const;

    virtual void setDuration( int seconds );
    int duration() const;
    void setHasDuration( bool );
    bool hasDuration() const;

    /** Returns true or false depending on whether the incidence "floats,"
     * i.e. has a date but no time attached to it. */
    bool doesFloat() const;

    /**
      Sets whether the incidence floats, i.e. has a date but no time attached
      to it.
    */
    void setFloats( bool f );

    /**
      Shift the times of the incidence so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00
      America/New York, using an old viewing time zone (@p oldSpec)
      of Europe/London, to a new time zone (@p newSpec) of Europe/Paris,
      will result in the time being shifted from 14:00 (which is the London
      time of the incidence start) to 14:00 Paris time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    //
    // Comments
    //

    /**
     * Add a comment to this incidence.
     *
     * Does not add a linefeed character.  Just appends the text as passed in.
     *
     * @param comment  The comment to add.
     */
    void addComment( const QString &comment );

    /**
     * Remove a comment from the incidence.
     *
     * Removes first comment whose string is an exact match for the string
     * passed in.
     *
     * @return true if match found, false otherwise.
     */
    bool removeComment( const QString &comment );

    /** Delete all comments associated with this incidence. */
    void clearComments();

    /** Returns all comments associated with this incidence.  */
    QStringList comments() const;

    /**
      Add Attendee to this incidence. IncidenceBase takes ownership of the
      Attendee object.

      @param attendee a pointer to the attendee to add
      @param doUpdate If true the Observers are notified, if false they are not.
    */
    void addAttendee( Attendee *attendee, bool doUpdate = true );

    /**
      Remove all Attendees.
    */
    void clearAttendees();

    /**
      Returns list of attendees.
    */
    const Attendee::List &attendees() const;

    /**
      Returns number of attendees.
    */
    int attendeeCount() const;

    /**
      Returns the Attendee with this email address.
    */
    Attendee *attendeeByMail( const QString &email ) const;

    /**
      Returns first Attendee with one of the given email addresses.
    */
    Attendee *attendeeByMails( const QStringList &,
                               const QString &email = QString() ) const;

    /**
      Returns attendee with given uid.
    */
    Attendee *attendeeByUid( const QString &uid ) const;

    /**
      Pilot synchronization states
    */
    enum {
      SYNCNONE = 0,
      SYNCMOD = 1,
      SYNCDEL = 3
    };

    /**
      Sets synchronization satus.
    */
    void setSyncStatus( int status );

    /**
      Returns synchronization status.
    */
    int syncStatus() const;

    /**
      Sets Pilot Id.
    */
    void setPilotId( unsigned long id );

    /**
      Returns Pilot Id.
    */
    unsigned long pilotId() const;

    /**
      Register observer. The observer is notified when the observed object
      changes.
    */
    void registerObserver( Observer * );

    /**
      Unregister observer. It isn't notified anymore about changes.
    */
    void unRegisterObserver( Observer * );

    /**
      Call this to notify the observers after the IncidenceBas object has
      changed.
    */
    void updated();

  protected:
    bool mReadOnly;

  private:
    class Private;
    Private *d;
};

}

#endif
