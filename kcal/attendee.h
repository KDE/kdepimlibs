/*
    This file is part of the kcal library.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
  defines the Attendee class.

  @author Cornelius Schumacher
*/

#ifndef KCAL_ATTENDEE_H
#define KCAL_ATTENDEE_H

#include <QtCore/QString>

#include "listbase.h"
#include "person.h"

namespace KCal {

/**
  @brief
  Represents information related to an attendee of an Calendar Incidence,
  typically a meeting or task (to-do).

  Attendees are people with a name and (optional) email address who are
  invited to participate in some way in a meeting or task.  This class
  also tracks that status of the invitation: accepted; tentatively accepted;
  declined; delegated to another person; in-progress; completed.

  Attendees may optionally be asked to @acronym RSVP ("Respond Please") to
  the invitation.

  Note that each attendee be can optionally associated with a @acronym UID
  (unique identifier) derived from a Calendar Incidence, Email Message,
  or any other thing you want.
*/
class KCAL_EXPORT Attendee : public Person
{
  public:

    /**
      The different types of participant statuses.
    */
    enum PartStat {
      NeedsAction,     /**< No information about the task/invitation received */
      Accepted,        /**< Accepted the task/invitation */
      Declined,        /**< Declined the task/invitation */
      Tentative,       /**< Tentatively accepted the task/invitation */
      Delegated,       /**< Delegated the task to another */
      Completed,       /**< Completed the task */
      InProcess        /**< Work on the task is in-progress */
    };

    /**
      The different types of meeting roles.
    */
    enum Role {
      ReqParticipant,  /**< Required participant */
      OptParticipant,  /**< Optional participant */
      NonParticipant,  /**< Non-participant */
      Chair            /**< Meeting chairperson */
    };

    /**
      List of attendees.
    */
    typedef ListBase<Attendee> List;

    /**
      Constructs an attendee consisting of a Person name (@p name) and
      email address (@p email); invitation status and #Role;
      an optional @acronym RSVP flag and @acronym UID.

      @param name is person name of the attendee.
      @param email is person email address of the attendee.
      @param rsvp if set (true), the attendee is requested to reply to
      invitations.
      @param status is the #PartStat status of the attendee.
      @param role is the #Role of the attendee.
      @param uid is the @acronym UID of the attendee.
    */
    Attendee( const QString &name, const QString &email,
              bool rsvp = false, PartStat status = NeedsAction,
              Role role = ReqParticipant, const QString &uid = QString() );

    /**
      Destroys the attendee.
    */
    virtual ~Attendee();

    /**
      Sets the #Role of the attendee to @p role.

      @param role is the #Role to use for the attendee.

      @see role()
    */
    void setRole( Role role );

    /**
      Returns the #Role of the attendee.

      @see setRole()
    */
    Role role() const;

    /**
      Returns the attendee #Role as a text string.

      @see role(), roleName()
    */
    QString roleStr() const;

    /**
      Returns the specified #Role @p role as a text string.

      @param role is a #Role value.

      @see role(), roleStr()
    */
    static QString roleName( Role role );

    /**
      Returns a list of strings representing each #Role.
    */
    static QStringList roleList();

    /**
      Sets the @acronym UID of the attendee to @p uid.

      @param uid is the @acronym UID to use for the attendee.

      @see uid()
    */
    void setUid ( const QString &uid );

    /**
      Returns the @acronym UID of the attendee.

      @see setUid()
    */
    QString uid() const;

    /**
      Sets the #PartStat of the attendee to @p status.

      @param status is the #PartStat to use for the attendee.

      @see status()
    */
    void setStatus( PartStat status );

    /**
      Returns the #PartStat of the attendee.

      @see setStatus()
    */
    PartStat status() const;

    /**
      Returns the attendee #PartStat as a text string.

      @see status(), statusName()
    */
    QString statusStr() const;

    /**
      Returns the specified #PartStat @p status  as a text string.

      @param status is a #PartStat value.

      @see status(), statusStr()
    */
    static QString statusName( PartStat status );

    /**
      Returns a list of strings representing each #PartStat.
    */
    static QStringList statusList();

    /**
      Sets the @acronym RSVP flag of the attendee to @p rsvp.

      @param rsvp if set (true), the attendee is requested to reply to
      invitations.

      @see RSVP()
    */
    void setRSVP( bool rsvp );

    /**
      Returns the attendee @acronym RSVP flag.

      @see setRSVP()
    */
    bool RSVP() const;

    /**
      Compares this with @p attendee for equality.

      @param attendee the attendee to compare.
    */
    bool operator==( const Attendee &attendee );

    /**
      Sets the delegate.
    */
    void setDelegate( const QString &delegate );
    /**
      Returns the delegate.
    */
    QString delegate() const;

    /**
      Sets the delegator.
    */
    void setDelegator( const QString &delegator );
    /**
      Returns the delegator.
    */
    QString delegator() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
