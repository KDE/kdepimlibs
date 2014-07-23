/*
    ktnefwriter.cpp

    Copyright (C) 2002 Bo Thorsen  <bo@sonofthor.dk>

    This file is part of KTNEF, the KDE TNEF support library/program.

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
 * @file
 * This file is part of the API for handling TNEF data and
 * defines the KTNEFWriter class.
 *
 * @author Bo Thorsen
 */

#ifndef KTNEFWRITER_H
#define KTNEFWRITER_H

class QString;
class QVariant;
class QIODevice;
class QDataStream;
class QDateTime;
class QStringList;

#include "ktnef_export.h"
#include <qglobal.h>

namespace KTnef
{

/**
 * @brief
 * Manages the writing of @acronym TNEF attachments.
 */
class KTNEF_EXPORT KTNEFWriter
{
public:
    /**
     * The different types of messages.
     */
    enum MessageType {
        Appointment,     /**< Appointment */
        MeetingCancelled,/**< The meeting is cancelled */
        MeetingRequest,  /**< Meeting request */
        MeetingNo,       /**< Negative response to a meeting request */
        MeetingYes,      /**< Affirmative response to a meeting request */
        MeetingTent      /**< Tentative affirmative to a meeting request */
    };

    /**
     * The different types of message statuses.
     */
    enum Method {
        PublishNew,      /**< Publish new */
        Obsolete,        /**< Replace the message */
        RequestNew,      /**< Request a new message */
        RequestUpdate,   /**< Request an update */
        Unknown          /**< Unknown */
    };

    /**
     * The different types of meeting roles.
     */
    enum Role {
        ReqParticipant,  /**< Required participant */
        OptParticipant,  /**< Optional participant */
        NonParticipant,  /**< Non-participant */
        Chair            /**< Meeting chairperson */
    };

    /**
     * The different types of participant statuses.
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
     * The different priorities.
     */
    enum Priority {
        High = 2,        /**< High priority task */
        Normal = 3,      /**< Normal priority task */
        Low = 1          /**< Low priority task */
    };

    /**
     * The different alarm actions.
     */
    enum AlarmAction {
        Display          /**< Display the alarm */
    };

    /**
     * Constructs a @acronym TNEF writer object.
     */
    KTNEFWriter();

    /**
     * Destroys the @acronym TNEF writer object.
     */
    ~KTNEFWriter();

    /**
     * Adds a @acronym TNEF property.
     *
     * @param tag is the @acronym TNEF tag
     * @param type is the property type
     * @param value is the property value
     */
    void addProperty(int tag, int type, const QVariant &value);

    /**
     * Writes a @acronym TNEF property to the #QDataStream specified by @p stream.
     *
     * A @acronym TNEF property has a 1 byte type (LVL_MESSAGE or LVL_ATTACHMENT),
     * a 4 byte type/tag, a 4 byte length, the data and finally the checksum.
     *
     * The checksum is a 16 byte int with all bytes in the data added.
     *
     * @param stream is the #QDataStream to write
     * @param bytes is a pointer to an int type that will contain
     * the number of bytes written to the @p stream
     * @param tag is the @acronym TNEF tag
     *
     * @return false if an invalid @acronym TNEF tag was specified by @p tag or
     * if there are no properties to write; else true.
     */
    bool writeProperty(QDataStream &stream, int &bytes, int tag) const;

    /**
     * Writes the attachment to the #QIODevice specified by @p file.
     *
     * @param file is the #QIODevice to write.
     * @return true if the write was successful; otherwise false.
     */
    bool writeFile(QIODevice &file) const;

    /**
     * Writes the attachment to the #QDataStream specified by @p stream.
     *
     * @param stream is the #QDataStream to write.
     * @return true if the write was successful; otherwise false.
     */
    bool writeFile(QDataStream &stream) const;

    /**
     * Sets the sender's @p name and @p email address.
     *
     * @param name is the sender's name.
     * @param email is the sender's email address.
     */
    void setSender(const QString &name, const QString &email);

    /**
     * Sets the #MessageType to @p methodType.
     *
     * @param methodType is the #MessageType.
     */
    void setMessageType(MessageType methodType);

    /**
     * Sets the #Method to @p method.
     *
     * @param method is the #Method.
     */
    void setMethod(Method method);

    /**
     * Clears the attendees list.
     */
    void clearAttendees();

    /**
     * Adds a meeting participant (attendee).
     *
     * @param name is the name of the attendee
     * @param role is the #Role of the attendee
     * @param partstat is the status #PartStat of the attendee
     * @param rsvp is true if the attendee will attend the meeting; else false
     * @param email is the email address of the attendee
     */
    void addAttendee(const QString &name, Role role, PartStat partstat,
                     bool rsvp, const QString &email);

    /**
     * Sets the name of the organizer to @p organizer.
     * The organizer is any string identifier; it could be the name
     * of a person, or the application that sent the invitation, for example.
     *
     * @param organizer is the organizer identifier.
     */
    void setOrganizer(const QString &organizer);

    /**
     * Sets the Starting Date and Time to @p dtStart.
     *
     * @param dtStart is the starting date/time.
     */
    void setDtStart(const QDateTime &dtStart);

    /**
     * Sets the Ending Date and Time to @p dtEnd.
     *
     * @param dtEnd is the ending date/time.
     */
    void setDtEnd(const QDateTime &dtEnd);

    /**
     * Sets the Location to @p location.
     *
     * @param location is the location.
     */
    void setLocation(const QString &location);

    /**
     * Sets the @acronym UID to @p uid.
     *
     * @param uid is the @acronym UID.
     */
    void setUID(const QString &uid);

    /**
     * Sets the timestamp to @p dtStamp.
     *
     * @param dtStamp is the timestamp.
     */
    void setDtStamp(const QDateTime &dtStamp);

    /**
     * Sets the category list to @p categories.
     *
     * @param categories is the list categories.
     */
    void setCategories(const QStringList &categories);

    /**
     * Sets the description to @p description.
     *
     * @param description is the description.
     */
    void setDescription(const QString &description);

    /**
     * Sets the summary to @p summary.
     *
     * @param summary is the summary.
     */
    void setSummary(const QString &summary);

    /**
     * Sets the priority to @p priority.
     *
     * @param priority is the #Priority.
     */
    void setPriority(Priority priority);

    /**
     * Sets the alarm.
     *
     * @param description is the alarm description
     * @param action is the alaram #AlarmAction
     * @param wakeBefore is the alarm Date/Time
     */
    void setAlarm(const QString &description, AlarmAction action,
                  const QDateTime &wakeBefore);

private:
    //@cond PRIVATE
    class PrivateData;
    PrivateData *const d;
    //@endcond

    Q_DISABLE_COPY(KTNEFWriter)
};

}

#endif // KTNEFWRITER_H
