/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author Rafal Rzepecki \<divide@users.sourceforge.net\>

  @glossary @anchor incidence @b incidence:
  General term for a calendar component.
  Examples are events, to-dos, and journals.

  @glossary @anchor event @b event:
  An @ref incidence that has a start and end time, typically representing some
  occurrence of social or personal importance. May be recurring.
  Examples are appointments, meetings, or holidays.

  @glossary @anchor to-do @b to-do:
  An @ref incidence that has an optional start time and an optional due time
  typically representing some undertaking to be performed. May be recurring.
  Examples are "fix the bug" or "pay the bills".

  @glossary @anchor todo @b todo:
  See @ref to-do.

  @glossary @anchor journal @b journal:
  An @ref incidence with a start date that represents a diary or daily record
  of one's activities. May @b not be recurring.
*/

#ifndef KCAL_INCIDENCEBASE_H
#define KCAL_INCIDENCEBASE_H

#include "attendee.h"
#include "customproperties.h"
#include "duration.h"
#include "sortablelist.h"

#include <kdatetime.h>

#include <QtCore/QStringList>
#include <QtCore/QByteArray>

class KUrl;

namespace KCal {

/** List of dates */
typedef SortableList<QDate> DateList;
/** List of times */
typedef SortableList<KDateTime> DateTimeList;
class Event;
class Todo;
class Journal;
class FreeBusy;

/**
  @brief
  An abstract class that provides a common base for all calendar incidence
  classes.

  define: organizer (person)
  define: uid (same as the attendee uid?)

  Several properties are not allowed for VFREEBUSY objects (see rfc:2445),
  so they are not in IncidenceBase. The hierarchy is:

  IncidenceBase
  + FreeBusy
  + Incidence
    + Event
    + Todo
    + Journal

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
    class KCAL_EXPORT Visitor //krazy:exclude=dpointer
    {
      public:
        /** Destruct Incidence::Visitor */
        virtual ~Visitor() {}

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on an Event object.
          @param event is a pointer to a valid Event object.
        */
        virtual bool visit( Event *event );

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on a Todo object.
          @param todo is a pointer to a valid Todo object.
        */
        virtual bool visit( Todo *todo );

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on an Journal object.
          @param journal is a pointer to a valid Journal object.
        */
        virtual bool visit( Journal *journal );

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on a FreeBusy object.
          @param freebusy is a pointer to a valid FreeBusy object.
        */
        virtual bool visit( FreeBusy *freebusy );

      protected:
        /**
          Constructor is protected to prevent direct creation of visitor
          base class.
        */
        Visitor() {}
    };

    /**
      The IncidenceObserver class.
    */
    class IncidenceObserver
    {
      public:

        /**
          Destroys the IncidenceObserver.
        */
        virtual ~IncidenceObserver() {}

        /**
          The IncidenceObserver interface.

          @param incidenceBase is a pointer to an IncidenceBase object.
        */
        virtual void incidenceUpdated( IncidenceBase *incidenceBase ) = 0;
    };

    /**
      Constructs an empty IncidenceBase.
    */
    IncidenceBase();

    /**
      Constructs an IncidenceBase as a copy of another IncidenceBase object.

      @param ib is the IncidenceBase to copy.
    */

    IncidenceBase( const IncidenceBase &ib );

    /**
      Destroys the IncidenceBase.
    */
    virtual ~IncidenceBase();

    /**
      Assignment operator.

      @warning Not polymorphic. Use AssignmentVisitor for correct
      assignment of an instance of type IncidenceBase to another
      instance of type IncidenceBase.

      @param other is the IncidenceBase to assign.

      @see AssignmentVisitor
     */
    // KDE5: make protected to prevent accidental usage
    IncidenceBase &operator=( const IncidenceBase &other );

    /**
      Compares this with IncidenceBase @p ib for equality.

      @warning Not polymorphic. Use ComparisonVisitor for correct
      comparison of two instances of type IncidenceBase.

      @param ib is the IncidenceBase to compare.

      @see ComparisonVisitor
    */
    // KDE5: make protected to prevent accidental usage
    bool operator==( const IncidenceBase &ib ) const;

    /**
      Accept IncidenceVisitor. A class taking part in the visitor mechanism
      has to provide this implementation:
      <pre>
        bool accept(Visitor &v) { return v.visit(this); }
      </pre>

      @param v is a reference to a Visitor object.
    */
    virtual bool accept( Visitor &v )
    {
      Q_UNUSED( v );
      return false;
    }

    /**
      Prints the type of Incidence as a string.
    */
    virtual QByteArray type() const = 0;

    /**
      Returns the type of Incidence as a translated string.
    */
    //KDE5: virtual QString typeStr() const = 0;

    /**
      Sets the unique id for the incidence to @p uid.

      @param uid is the string containing the incidence @ref uid.

      @see uid()
    */
    void setUid( const QString &uid );

    /**
      Returns the unique id (@ref uid) for the incidence.

      @see setUid()
    */
    QString uid() const;

    /**
      Returns the uri for the incidence, of form urn:x-ical:\<uid\>
    */
    KUrl uri() const;

    /**
      Sets the time the incidence was last modified to @p lm.
      It is stored as a UTC date/time.

      @param lm is the KDateTime when the incidence was last modified.

      @see lastModified()
    */
    void setLastModified( const KDateTime &lm );

    /**
      Returns the time the incidence was last modified.

      @see setLastModified()
    */
    KDateTime lastModified() const;

    /**
      Sets the organizer for the incidence.

      @param organizer is a Person to use as the incidence @ref organizer.
      @see organizer(), setOrganizer(const QString &)
    */
    void setOrganizer( const Person &organizer );

    /**
      Sets the incidence organizer to any string @p organizer.

      @param organizer is a string to use as the incidence @ref organizer.
      @see organizer(), setOrganizer(const Person &)
    */
    void setOrganizer( const QString &organizer );

    /**
      Returns the Person associated with this incidence.

      @see setOrganizer(const QString &), setOrganizer(const Person &)
    */
    Person organizer() const;

    /**
      Sets readonly status.

      @param readOnly if set, the incidence is read-only; else the incidence
      can be modified.
      @see isReadOnly().
    */
    virtual void setReadOnly( bool readOnly );

    /**
      Returns true the object is read-only; false otherwise.
      @see setReadOnly()
    */
    bool isReadOnly() const { return mReadOnly; }

    /**
      Sets the incidence's starting date/time with a KDateTime.
      The incidence's all-day status is set according to whether @p dtStart
      is a date/time (not all-day) or date-only (all-day).

      @param dtStart is the incidence start date/time.
      @see dtStart().
    */
    virtual void setDtStart( const KDateTime &dtStart );

    /**
      Returns an incidence's starting date/time as a KDateTime.
      @see setDtStart().
    */
    virtual KDateTime dtStart() const;

    /**
      Returns an incidence's starting time as a string formatted according
      to the user's locale settings.

      @param shortfmt If set to true, use short date format, if set to false use
      long format.
      @param spec If set, return the time in the given spec, else use the
      incidence's current spec.

      @deprecated use IncidenceFormatter::timeToString()
    */
    virtual KDE_DEPRECATED QString dtStartTimeStr(
      bool shortfmt = true, const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns an incidence's starting date as a string formatted according
      to the user's locale settings.

      @param shortfmt If set to true, use short date format, if set to false use
      long format.
      @param spec If set, return the date in the given spec, else use the
      incidence's current spec.

      @deprecated use IncidenceFormatter::dateToString()
    */
    virtual KDE_DEPRECATED QString dtStartDateStr(
      bool shortfmt = true, const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns an incidence's starting date and time as a string formatted
      according to the user's locale settings.

      @param shortfmt If set to true, use short date format, if set to false use
      long format.
      @param spec If set, return the date and time in the given spec, else use
      the incidence's current spec.

      @deprecated use IncidenceFormatter::dateTimeToString()
    */
    virtual KDE_DEPRECATED QString dtStartStr(
      bool shortfmt = true, const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Sets the incidence duration.

      @param duration the incidence duration

      @see duration()
    */
    virtual void setDuration( const Duration &duration );

    /**
      Returns the length of the incidence duration.

      @see setDuration()
    */
    Duration duration() const;

    /**
      Sets if the incidence has a duration.

      @param hasDuration true if the incidence has a duration; false otherwise.

      @see hasDuration()
    */
    void setHasDuration( bool hasDuration );

    /**
      Returns true if the incidence has a duration; false otherwise.

      @see setHasDuration()
    */
    bool hasDuration() const;

    /**
      Returns true or false depending on whether the incidence is all-day.
      i.e. has a date but no time attached to it.

      @see setAllDay()
    */
    bool allDay() const;

    /**
      Sets whether the incidence is all-day, i.e. has a date but no time
      attached to it.

      @param allDay sets whether the incidence is all-day.

      @see allDay()
    */
    void setAllDay( bool allDay );

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

    /**
      Adds a comment to thieincidence. Does not add a linefeed character; simply
      appends the text as specified.

      @param comment is the QString containing the comment to add.
      @see removeComment().
    */
    void addComment( const QString &comment );

    /**
      Removes a comment from the incidence. Removes the first comment whose
      string is an exact match for the specified string in @p comment.

      @param comment is the QString containing the comment to remove.
      @return true if match found, false otherwise.
      @see addComment().
     */
    bool removeComment( const QString &comment );

    /**
      Deletes all incidence comments.
    */
    void clearComments();

    /**
      Returns all incidence comments as a list of strings.
    */
    QStringList comments() const;

    /**
      Add Attendee to this incidence. IncidenceBase takes ownership of the
      Attendee object.

      @param attendee a pointer to the attendee to add
      @param doUpdate If true the Observers are notified, if false they are not.
    */
    void addAttendee( Attendee *attendee, bool doUpdate = true );

    /**
      Removes all attendees from the incidence.
    */
    void clearAttendees();

    /**
      Returns a list of incidence attendees.
    */
    const Attendee::List &attendees() const;

    /**
      Returns the number of incidence attendees.
    */
    int attendeeCount() const;

    /**
      Returns the attendee with the specified email address.

      @param email is a QString containing an email address of the
      form "FirstName LastName <emailaddress>".
      @see attendeeByMails(), attendeesByUid().
    */
    Attendee *attendeeByMail( const QString &email ) const;

    /**
      Returns the first incidence attendee with one of the specified
      email addresses.

      @param emails is a list of QStrings containing email addresses of the
      form "FirstName LastName <emailaddress>".
      @param email is a QString containing a single email address to search
      in addition to the list specified in @p emails.
      @see attendeeByMail(), attendeesByUid().
    */
    Attendee *attendeeByMails( const QStringList &emails,
                               const QString &email = QString() ) const;

    /**
      Returns the incidence attendee with the specified attendee @acronym UID.

      @param uid is a QString containing an attendee @acronym UID.
      @see attendeeByMail(), attendeeByMails().
    */
    Attendee *attendeeByUid( const QString &uid ) const;

    /**
      Register observer. The observer is notified when the observed object
      changes.

      @param observer is a pointer to an IncidenceObserver object that will be
      watching this incidence.
      @see unRegisterObserver()
    */
    void registerObserver( IncidenceObserver *observer );

    /**
      Unregister observer. It isn't notified anymore about changes.

      @param observer is a pointer to an IncidenceObserver object that will be
      watching this incidence.
      @see registerObserver().
    */
    void unRegisterObserver( IncidenceObserver *observer );

    /**
      Call this to notify the observers after the IncidenceBase object has
      changed.
    */
    void updated();

    /**
      Call this when a group of updates is going to be made. This suppresses
      change notifications until endUpdates() is called, at which point
      updated() will automatically be called.
    */
    void startUpdates();

    /**
      Call this when a group of updates is complete, to notify observers that
      the instance has changed. This should be called in conjunction with
      startUpdates().
    */
    void endUpdates();

  protected:
    /**
      @copydoc
      CustomProperties::customPropertyUpdated()
    */
    virtual void customPropertyUpdated();

    /**
      Identifies a read-only incidence.
    */
    bool mReadOnly;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
