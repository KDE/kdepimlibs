/*
  This file is part of the kcalcore library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003 David Jarvie <software@astrojar.org.uk>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
  defines the Alarm class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCALCORE_ALARM_H
#define KCALCORE_ALARM_H

#include "kcalcore_export.h"
#include "customproperties.h"
#include "duration.h"
#include "person.h"

#include <kdatetime.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace KCalCore {

class Incidence;

/**
  @brief
  Represents an alarm notification.

  Alarms are user notifications that occur at specified times.
  Notifications can be on-screen pop-up dialogs, email messages,
  the playing of audio files, or the running of another program.

  Alarms always belong to a parent Incidence.
*/
class KCALCORE_EXPORT Alarm : public CustomProperties
{
  public:
    /**
      The different types of alarms.
    */
    enum Type {
      Invalid,   /**< Invalid, or no alarm */
      Display,   /**< Display a dialog box */
      Procedure, /**< Call a script */
      Email,     /**< Send email */
      Audio      /**< Play an audio file */
    };

    /**
      A shared pointer to an Alarm object.
    */
    typedef QSharedPointer<Alarm> Ptr;

    /**
       A shared pointer to a non-mutable Alarm object.
    */
    typedef QSharedPointer<const Alarm> ConstPtr;

    /**
      List of alarms.
    */
    typedef QList<Ptr> List;

    /**
      Constructs an alarm belonging to the @p parent Incidence.

      @param parent is the Incidence this alarm will belong to.
    */
    // KDAB_TODO Can't find a way to use a shared pointer here.
    // Incidence incidence.cpp, it does alarm->setParent( this )
    explicit Alarm( Incidence *parent );

    /**
      Copy constructor.
      @param other is the alarm to copy.
    */
    Alarm( const Alarm &other );

    /**
      Destroys the alarm.
    */
    virtual ~Alarm();

    /**
      Copy operator.
    */
    Alarm &operator=( const Alarm & );

    /**
      Compares two alarms for equality.
      @param a is the comparison alarm.
    */
    bool operator==( const Alarm &a ) const;

    /**
      Compares two alarms for inequality.

      @param a is the comparison alarm.
    */
    bool operator!=( const Alarm &a ) const;

    /**
      Sets the @p parent Incidence of the alarm.

      @param parent is alarm parent Incidence to set.

      @see parentUid()
    */
    // KDAB_TODO: is there a way to use QSharedPointer here?
    // although it's safe, Incidence's dtor calls setParent( 0 )
    // se we don't dereference a deleted pointer here.
    // Also, I renamed "Incidence *parent()" to "QString parentUid()"
    // So we don't return raw pointers
    void setParent( Incidence *parent );

    /**
      Returns the parent's incidence UID of the alarm.

      @see setParent()
    */
    // We don't have a share pointer to return, so return the UID.
    QString parentUid() const;

    /**
      Sets the #Type for this alarm to @p type.
      If the specified type is different from the current type of the alarm,
      then the alarm's type-specific properties are re-initialized.

      @param type is the alarm #Type to set.

      @see type()
    */
    void setType( Type type );

    /**
      Returns the #Type of the alarm.

      @see setType()
    */
    Type type() const;

    /**
      Sets the #Display type for this alarm.
      If @p text is specified non-empty, then it is used as the description
      text to display when the alarm is triggered.

      @param text is the description to display when the alarm is triggered.

      @see setText(), text()
    */
    void setDisplayAlarm( const QString &text = QString() );

    /**
      Sets the description @p text to be displayed when the alarm is triggered.
      Ignored if the alarm is not a display alarm.

      @param text is the description to display when the alarm is triggered.

      @see setDisplayAlarm(), text()
    */
    void setText( const QString &text );

    /**
      Returns the display text string for a #Display alarm type.
      Returns an empty string if the alarm is not a #Display type.

      @see setDisplayAlarm(), setText()
    */
    QString text() const;

    /**
      Sets the #Audio type for this alarm and the name of the audio file
      to play when the alarm is triggered.

      @param audioFile is the name of the audio file to play when the alarm
      is triggered.

      @see setAudioFile(), audioFile()
    */
    void setAudioAlarm( const QString &audioFile = QString() );

    /**
      Sets the name of the audio file to play when the audio alarm is triggered.
      Ignored if the alarm is not an #Audio type.

      @param audioFile is the name of the audio file to play when the alarm
      is triggered.

      @see setAudioAlarm(), audioFile()
    */
    void setAudioFile( const QString &audioFile );

    /**
      Returns the audio file name for an #Audio alarm type.
      Returns an empty string if the alarm is not an #Audio type.

      @see setAudioAlarm(), setAudioFile()
    */
    QString audioFile() const;

    /**
      Sets the #Procedure type for this alarm and the program (with arguments)
      to execute when the alarm is triggered.

      @param programFile is the name of the program file to execute when
      the alarm is triggered.
      @param arguments is a string of arguments to supply to @p programFile.

      @see setProgramFile(), programFile(),
      setProgramArguments(), programArguments()
    */
    void setProcedureAlarm( const QString &programFile,
                            const QString &arguments = QString() );

    /**
      Sets the program file to execute when the alarm is triggered.
      Ignored if the alarm is not a #Procedure type.

      @param programFile is the name of the program file to execute when
      the alarm is triggered.

      @see setProcedureAlarm(), programFile(),
      setProgramArguments(), programArguments()
    */
    void setProgramFile( const QString &programFile );

    /**
      Returns the program file name for a #Procedure alarm type.
      Returns an empty string if the alarm is not a #Procedure type.

      @see setProcedureAlarm(), setProgramFile(),
      setProgramArguments(), programArguments()
    */
    QString programFile() const;

    /**
      Sets the program arguments string when the alarm is triggered.
      Ignored if the alarm is not a #Procedure type.

      @param arguments is a string of arguments to supply to the program.

      @see setProcedureAlarm(), setProgramFile(), programFile(),
      programArguments()
    */
    void setProgramArguments( const QString &arguments );

    /**
      Returns the program arguments string for a #Procedure alarm type.
      Returns an empty string if the alarm is not a #Procedure type.

      @see setProcedureAlarm(), setProgramFile(), programFile(),
      setProgramArguments()
    */
    QString programArguments() const;

    /**
      Sets the #Email type for this alarm and the email @p subject, @p text,
      @p addresses, and @p attachments that make up an email message to be
      sent when the alarm is triggered.

      @param subject is the email subject.
      @param text is a string containing the body of the email message.
      @param addressees is Person list of email addresses.
      @param attachments is a a QStringList of optional file names
      of email attachments.

      @see setMailSubject(), setMailText(), setMailAddresses(),
      setMailAttachments()
    */
    void setEmailAlarm( const QString &subject, const QString &text,
                        const Person::List &addressees,
                        const QStringList &attachments = QStringList() );

    /**
      Sets the email address of an #Email type alarm.
      Ignored if the alarm is not an #Email type.

      @param mailAlarmAddress is a Person to receive a mail message when
      an #Email type alarm is triggered.

      @see setMailSubject(), setMailText(), setMailAddresses(),
      setMailAttachment(), setMailAttachments(), mailAddresses()
    */
    void setMailAddress( const Person::Ptr &mailAlarmAddress );

    /**
      Sets a list of email addresses of an #Email type alarm.
      Ignored if the alarm is not an #Email type.

      @param mailAlarmAddresses is a Person list to receive a mail message
      when an #Email type alarm is triggered.

      @see setMailSubject(), setMailText(), setMailAddress(),
      setMailAttachments(), setMailAttachment(), mailAddresses()
    */
    void setMailAddresses( const Person::List &mailAlarmAddresses );

    /**
      Adds an address to the list of email addresses to send mail to when the
      alarm is triggered.
      Ignored if the alarm is not an #Email type.

      @param mailAlarmAddress is a Person to add to the list of addresses to
      receive a mail message when an #Email type alarm is triggered.

      @see setMailAddress(), setMailAddresses(), mailAddresses()
    */
    void addMailAddress( const Person::Ptr &mailAlarmAddress );

    /**
      Returns the list of addresses for an #Email alarm type.
      Returns an empty list if the alarm is not an #Email type.

      @see addMailAddress(), setMailAddress(), setMailAddresses()
    */
    Person::List mailAddresses() const;

    /**
      Sets the subject line of a mail message for an #Email alarm type.
      Ignored if the alarm is not an #Email type.

      @param mailAlarmSubject is a string to be used as a subject line
      of an email message to send when the #Email type alarm is triggered.

      @see setMailText(), setMailAddress(), setMailAddresses(),
      setMailAttachment(), setMailAttachments(), mailSubject()
    */
    void setMailSubject( const QString &mailAlarmSubject );

    /**
      Returns the subject line string for an #Email alarm type.
      Returns an empty string if the alarm is not an #Email type.

      @see setMailSubject()
    */
    QString mailSubject() const;

    /**
      Sets the filename to attach to a mail message for an #Email alarm type.
      Ignored if the alarm is not an #Email type.

      @param mailAttachFile is a string containing a filename to be attached
      to an email message to send when the #Email type alarm is triggered.

      @see setMailSubject(), setMailText(), setMailAddress(),
      setMailAddresses(), setMailAttachments(), mailAttachments()
    */
    void setMailAttachment( const QString &mailAttachFile );

    /**
      Sets a list of filenames to attach to a mail message for an #Email
      alarm type. Ignored if the alarm is not an #Email type.

      @param mailAttachFiles is a QString list of filenames to attach to
      a mail message when an #Email type alarm is triggered.

      @see setMailSubject(), setMailText(), setMailAttachment(),
      setMailAddress(), setMailAddresses()
    */
    void setMailAttachments( const QStringList &mailAttachFiles );

    /**
      Adds a filename to the list of files to attach to a mail message for
      an #Email alarm type. Ignored if the alarm is not an #Email type.

      @param mailAttachFile is a string containing a filename to be attached
      to an email message to send when the #Email type alarm is triggered.

      @see setMailAttachment(), setMailAttachments(), mailAttachments()
    */
    void addMailAttachment( const QString &mailAttachFile );

    /**
      Returns the list of attachment filenames for an #Email alarm type.
      Returns an empty list if the alarm is not an #Email type.

      @see addMailAttachment(), setMailAttachment(), setMailAttachments()
    */
    QStringList mailAttachments() const;

    /**
      Sets the body text for an #Email alarm type.
      Ignored if the alarm is not an #Email type.

      @param text is a string containing the body text of a mail message
      when an #Email type alarm is triggered.

      @see setMailSubject(), setMailAddress(), setMailAddresses(),
      setMailAttachment(), setMailAttachments()
    */
    void setMailText( const QString &text );

    /**
      Returns the body text for an #Email alarm type.
      Returns an empty string if the alarm is not an #Email type.

      @see setMailText()
    */
    QString mailText() const;

    /**
      Sets the trigger time of the alarm.

      @param alarmTime is the KDateTime alarm trigger.

      @see time()
    */
    void setTime( const KDateTime &alarmTime );

    /**
      Returns the alarm trigger date/time.

      @see setTime()
    */
    KDateTime time() const;

    /**
      Returns the next alarm trigger date/time after given date/time.
      Takes recurrent incidences into account.

      @param preTime date/time from where to start
      @param ignoreRepetitions don't take repetitions into account
      @see nextRepetition()
    */
    KDateTime nextTime( const KDateTime &preTime, bool ignoreRepetitions = false ) const;

    /**
      Returns the date/time when the last repetition of the alarm goes off.
      If the alarm does not repeat this is equivalent to calling time().

      @see setTime()
    */
    KDateTime endTime() const;

    /**
      Returns true if the alarm has a trigger date/time.
    */
    bool hasTime() const;

    /**
      Sets the alarm offset relative to the start of the parent Incidence.

      @param offset is a Duration to be used as a time relative to the
      start of the parent Incidence to be used as the alarm trigger.

      @see setEndOffset(), startOffset(), endOffset()
    */
    void setStartOffset( const Duration &offset );

    /**
      Returns offset of alarm in time relative to the start of the parent
      Incidence.  If the alarm's time is not defined in terms of an offset
      relative  to the start of the event, returns zero.

      @see setStartOffset(), hasStartOffset()
    */
    Duration startOffset() const;

    /**
      Returns whether the alarm is defined in terms of an offset relative
      to the start of the parent Incidence.

      @see startOffset(), setStartOffset()
    */
    bool hasStartOffset() const;

    /**
      Sets the alarm offset relative to the end of the parent Incidence.

      @param offset is a Duration to be used as a time relative to the
      end of the parent Incidence to be used as the alarm trigger.

      @see setStartOffset(), startOffset(), endOffset()
    */
    void setEndOffset( const Duration &offset );

    /**
      Returns offset of alarm in time relative to the end of the event.
      If the alarm's time is not defined in terms of an offset relative
      to the end of the event, returns zero.

      @see setEndOffset(), hasEndOffset()
    */
    Duration endOffset() const;

    /**
      Returns whether the alarm is defined in terms of an offset relative
      to the end of the event.

      @see endOffset(), setEndOffset()
    */
    bool hasEndOffset() const;

    /**
      Shift the times of the alarm so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual alarm time zone.

      For example, shifting an alarm whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldSpec) of Europe/London, to a new
      time zone (@p newSpec) of Europe/Paris, will result in the time being
      shifted from 14:00 (which is the London time of the alarm start) to
      14:00 Paris time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification
    */
    void shiftTimes( const KDateTime::Spec &oldSpec,
                     const KDateTime::Spec &newSpec );

    /**
      Sets the snooze time interval for the alarm.

      @param alarmSnoozeTime the time between snoozes.

      @see snoozeTime()
    */
    void setSnoozeTime( const Duration &alarmSnoozeTime );

    /**
      Returns the snooze time interval.

      @see setSnoozeTime()
    */
    Duration snoozeTime() const;

    /**
      Sets how many times an alarm is to repeat itself after its initial
      occurrence (w/snoozes).

      @param alarmRepeatCount is the number of times an alarm may repeat,
      excluding the initial occurrence.

      @see repeatCount()
    */
    void setRepeatCount( int alarmRepeatCount );

    /**
      Returns how many times an alarm may repeats after its initial occurrence.

      @see setRepeatCount()
    */
    int repeatCount() const;

    /**
      Returns the date/time of the alarm's initial occurrence or its next
      repetition after a given time.

      @param preTime the date/time after which to find the next repetition.

      @return the date/time of the next repetition, or an invalid date/time
      if the specified time is at or after the alarm's last repetition.

      @see previousRepetition()
    */
    KDateTime nextRepetition( const KDateTime &preTime ) const;

    /**
      Returns the date/time of the alarm's latest repetition or, if none,
      its initial occurrence before a given time.

      @param afterTime is the date/time before which to find the latest
      repetition.

      @return the date and time of the latest repetition, or an invalid
      date/time if the specified time is at or before the alarm's initial
      occurrence.

      @see nextRepetition()
    */
    KDateTime previousRepetition( const KDateTime &afterTime ) const;

    /**
      Returns the interval between the alarm's initial occurrence and
      its final repetition.
    */
    Duration duration() const;

    /**
      Toggles the alarm status, i.e, an enable alarm becomes disabled
      and a disabled alarm becomes enabled.

      @see enabled(), setEnabled()
    */
    void toggleAlarm();

    /**
      Sets the enabled status of the alarm.
      @param enable if true, then enable the alarm; else disable the alarm.

      @see enabled(), toggleAlarm()
    */
    void setEnabled( bool enable );

    /**
      Returns the alarm enabled status: true (enabled) or false (disabled).

      @see setEnabled(), toggleAlarm()
    */
    bool enabled() const;

    /**
      Set if the location radius for the alarm has been defined.
      @param hasLocationRadius if true, then this alarm has a location radius.

      @see setLocationRadius()
    */
    void setHasLocationRadius( bool hasLocationRadius );

    /**
      Returns true if alarm has location radius defined.

      @see setLocationRadius()
    */
    bool hasLocationRadius() const;

    /**
      Set location radius for the alarm. This means that alarm will be
      triggered when user approaches the location. Given value will be
      stored into custom properties as X-LOCATION-RADIUS.

      @param locationRadius radius in meters
      @see locationRadius()
    */
    void setLocationRadius( int locationRadius );

    /**
      Returns the location radius in meters.

      @see setLocationRadius()
    */
    int locationRadius() const;

  protected:
    /**
      @copydoc
      CustomProperties::customPropertyUpdated()
    */
    virtual void customPropertyUpdated();

    /**
      @copydoc
      IncidenceBase::virtual_hook()
    */
    virtual void virtual_hook( int id, void *data );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
