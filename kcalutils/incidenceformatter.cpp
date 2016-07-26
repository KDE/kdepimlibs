/*
  This file is part of the kcalutils library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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
  This file is part of the API for handling calendar data and provides
  static functions for formatting Incidences for various purposes.

  @brief
  Provides methods to format Incidences in various ways for display purposes.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author Allen Winter \<allen@kdab.com\>
*/
#include "incidenceformatter.h"
#include "stringify.h"

#include <kcalcore/event.h>
#include <kcalcore/freebusy.h>
#include <kcalcore/icalformat.h>
#include <kcalcore/journal.h>
#include <kcalcore/memorycalendar.h>
#include <kcalcore/todo.h>
#include <kcalcore/visitor.h>
using namespace KCalCore;

#include <kpimidentities/identitymanager.h>

#include <kpimutils/email.h>
#include <kpimutils/linklocator.h>

#include <KCalendarSystem>
#include <KDebug>
#include <KIconLoader>
#include <KLocalizedString>
#include <KGlobal>
#include <KMimeType>
#include <KSystemTimeZone>

#include <QtCore/QBitArray>
#include <QApplication>
#include <QPalette>
#include <QTextDocument>

using namespace KCalUtils;
using namespace IncidenceFormatter;

static QString invitationSummary(const Incidence::Ptr &incidence, bool noHtmlMode);
static QString inviteButton(InvitationFormatterHelper *helper,
                            const QString &id, const QString &text,
                            const QString &iconName = QString());

/*******************
 *  General helpers
 *******************/

//@cond PRIVATE
static QString string2HTML(const QString &str)
{
  //  return Qt::convertFromPlainText( str, Qt::WhiteSpaceNormal );
    // use convertToHtml so we get clickable links and other goodies
    return KPIMUtils::LinkLocator::convertToHtml(str);
}

static KPIMIdentities::IdentityManager *s_identityManager = 0;

// Performance optimization so we only create one IdentityManager instead of 1 per attendee.
// Using RAII to protect against future return statements in the middle of code
struct RAIIIdentityManager{
    RAIIIdentityManager()
    {
        //t.start();
        s_identityManager = new KPIMIdentities::IdentityManager(true);
    }

    ~RAIIIdentityManager()
    {
        delete s_identityManager;
        s_identityManager = 0;
        //qDebug() << "Elapsed time: " << t.elapsed();
    }
    //QElapsedTimer t;
};

static bool thatIsMe(const QString &email)
{
    return s_identityManager ? s_identityManager->thatIsMe(email)
                             : KPIMIdentities::IdentityManager(true).thatIsMe(email);
}

static bool iamAttendee(Attendee::Ptr attendee)
{
    // Check if this attendee is the user
    return thatIsMe(attendee->email());
}

static bool iamPerson(const Person &person)
{
    // Check if this person is the user. test email only
    return thatIsMe(person.email());
}

static QString htmlAddLink(const QString &ref, const QString &text,
                           bool newline = true)
{
    QString tmpStr(QLatin1String("<a href=\"") + ref + QLatin1String("\">") + text + QLatin1String("</a>"));
    if (newline) {
        tmpStr += QLatin1Char('\n');
    }
    return tmpStr;
}

static QString htmlAddMailtoLink(const QString &email, const QString &name)
{
    QString str;

    if (!email.isEmpty()) {
        Person person(name, email);
        if (!iamPerson(person)) {     // do not add a link for the user's email
            QString path = person.fullName().simplified();
            if (path.isEmpty() || path.startsWith(QLatin1Char('"'))) {
                path = email;
            }
            KUrl mailto;
            mailto.setProtocol(QLatin1String("mailto"));
            mailto.setPath(path);

            str = htmlAddLink(mailto.url(), name.isEmpty() ? email : name);
        }
    }
    return str;
}

static QString htmlAddUidLink(const QString &email, const QString &name, const QString &uid)
{
    QString str;

    if (!uid.isEmpty()) {
        // There is a UID, so make a link to the addressbook
        const QString url = QLatin1String("uid:") + uid;
        const QString iconPath = KIconLoader::global()->iconPath(QLatin1String("view-pim-contacts"), KIconLoader::Small);
        str = htmlAddLink(url, QString::fromLatin1("<img valign=\"middle\" src=\"%1\">").arg(iconPath));
    }
    return str;
}

static QString htmlAddTag(const QString &tag, const QString &text)
{
    int numLineBreaks = text.count(QLatin1String("\n"));
    QString str = QLatin1Char('<') + tag + QLatin1Char('>');
    QString tmpText = text;
    QString tmpStr = str;
    if (numLineBreaks >= 0) {
        if (numLineBreaks > 0) {
            int pos = 0;
            QString tmp;
            for (int i = 0; i <= numLineBreaks; ++i) {
                pos = tmpText.indexOf(QLatin1String("\n"));
                tmp = tmpText.left(pos);
                tmpText = tmpText.right(tmpText.length() - pos - 1);
                tmpStr += tmp + QLatin1String("<br>");
            }
        } else {
            tmpStr += tmpText;
        }
    }
    tmpStr += QLatin1String("</") + tag + QLatin1Char('>');
    return tmpStr;
}

static QPair<QString, QString> searchNameAndUid(const QString &email, const QString &name,
        const QString &uid)
{
    // Yes, this is a silly method now, but it's predecessor was quite useful in e35.
    // For now, please keep this sillyness until e35 is frozen to ease forward porting.
    // -Allen
    QPair<QString, QString>s;
    s.first = name;
    s.second = uid;
    if (!email.isEmpty() && (name.isEmpty() || uid.isEmpty())) {
        s.second.clear();
    }
    return s;
}

static QString searchName(const QString &email, const QString &name)
{
    const QString printName = name.isEmpty() ? email : name;
    return printName;
}

static bool iamOrganizer(Incidence::Ptr incidence)
{
    // Check if the user is the organizer for this incidence

    if (!incidence) {
        return false;
    }

    return thatIsMe(incidence->organizer()->email());
}

static bool senderIsOrganizer(Incidence::Ptr incidence, const QString &sender)
{
    // Check if the specified sender is the organizer

    if (!incidence || sender.isEmpty()) {
        return true;
    }

    bool isorg = true;
    QString senderName, senderEmail;
    if (KPIMUtils::extractEmailAddressAndName(sender, senderEmail, senderName)) {
        // for this heuristic, we say the sender is the organizer if either the name or the email match.
        if (incidence->organizer()->email() != senderEmail &&
                incidence->organizer()->name() != senderName) {
            isorg = false;
        }
    }
    return isorg;
}

static bool attendeeIsOrganizer(const Incidence::Ptr &incidence, const Attendee::Ptr &attendee)
{
    if (incidence && attendee &&
            (incidence->organizer()->email() == attendee->email())) {
        return true;
    } else {
        return false;
    }
}

static QString organizerName(const Incidence::Ptr incidence, const QString &defName)
{
    QString tName;
    if (!defName.isEmpty()) {
        tName = defName;
    } else {
        tName = i18n("Organizer Unknown");
    }

    QString name;
    if (incidence) {
        name = incidence->organizer()->name();
        if (name.isEmpty()) {
            name = incidence->organizer()->email();
        }
    }
    if (name.isEmpty()) {
        name = tName;
    }
    return name;
}

static QString firstAttendeeName(const Incidence::Ptr &incidence, const QString &defName)
{
    QString tName;
    if (!defName.isEmpty()) {
        tName = defName;
    } else {
        tName = i18n("Sender");
    }

    QString name;
    if (incidence) {
        Attendee::List attendees = incidence->attendees();
        if (attendees.count() > 0) {
            Attendee::Ptr attendee = *attendees.begin();
            name = attendee->name();
            if (name.isEmpty()) {
                name = attendee->email();
            }
        }
    }
    if (name.isEmpty()) {
        name = tName;
    }
    return name;
}

static QString rsvpStatusIconPath(Attendee::PartStat status)
{
    QString iconPath;
    switch (status) {
    case Attendee::Accepted:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("dialog-ok-apply"), KIconLoader::Small);
        break;
    case Attendee::Declined:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("dialog-cancel"), KIconLoader::Small);
        break;
    case Attendee::NeedsAction:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("help-about"), KIconLoader::Small);
        break;
    case Attendee::InProcess:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("help-about"), KIconLoader::Small);
        break;
    case Attendee::Tentative:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("dialog-ok"), KIconLoader::Small);
        break;
    case Attendee::Delegated:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("mail-forward"), KIconLoader::Small);
        break;
    case Attendee::Completed:
        iconPath = KIconLoader::global()->iconPath(QLatin1String("mail-mark-read"), KIconLoader::Small);
    default:
        break;
    }
    return iconPath;
}

//@endcond

/*******************************************************************
 *  Helper functions for the extensive display (display viewer)
 *******************************************************************/

//@cond PRIVATE
static QString displayViewFormatPerson(const QString &email, const QString &name,
                                       const QString &uid, const QString &iconPath)
{
    // Search for new print name or uid, if needed.
    QPair<QString, QString> s = searchNameAndUid(email, name, uid);
    const QString printName = s.first;
    const QString printUid = s.second;

    QString personString;
    if (!iconPath.isEmpty()) {
        personString += QLatin1String("<img valign=\"top\" src=\"") + iconPath + QLatin1String("\">") + QLatin1String("&nbsp;");
    }

    // Make the uid link
    if (!printUid.isEmpty()) {
        personString += htmlAddUidLink(email, printName, printUid);
    } else {
        // No UID, just show some text
        personString += (printName.isEmpty() ? email : printName);
    }

#ifndef KDEPIM_MOBILE_UI
    // Make the mailto link
    if (!email.isEmpty()) {
        personString += QLatin1String("&nbsp;") + htmlAddMailtoLink(email, printName);
    }
#endif

    return personString;
}

static QString displayViewFormatPerson(const QString &email, const QString &name,
                                       const QString &uid, Attendee::PartStat status)
{
    return displayViewFormatPerson(email, name, uid, rsvpStatusIconPath(status));
}

static bool incOrganizerOwnsCalendar(const Calendar::Ptr &calendar,
                                     const Incidence::Ptr &incidence)
{
    //PORTME!  Look at e35's CalHelper::incOrganizerOwnsCalendar

    // For now, use iamOrganizer() which is only part of the check
    Q_UNUSED(calendar);
    return iamOrganizer(incidence);
}

static QString displayViewFormatDescription(const Incidence::Ptr &incidence)
{
    QString tmpStr;
    if (!incidence->description().isEmpty()) {
        QString descStr;
        if (!incidence->descriptionIsRich() &&
            !incidence->description().startsWith(QLatin1String("<!DOCTYPE HTML"))) {
            descStr = string2HTML(incidence->description());
        } else {
            if (!incidence->description().startsWith(QLatin1String("<!DOCTYPE HTML"))) {
                descStr = incidence->richDescription();
            } else {
                descStr = incidence->description();
            }
        }
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Description:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + descStr + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }
    return tmpStr;
}

static QString displayViewFormatAttendeeRoleList(Incidence::Ptr incidence, Attendee::Role role,
        bool showStatus)
{
    QString tmpStr;
    Attendee::List::ConstIterator it;
    Attendee::List attendees = incidence->attendees();

    for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
        Attendee::Ptr a = *it;
        if (a->role() != role) {
            // skip this role
            continue;
        }
        if (attendeeIsOrganizer(incidence, a)) {
            // skip attendee that is also the organizer
            continue;
        }
        tmpStr += displayViewFormatPerson(a->email(), a->name(), a->uid(),
                                          showStatus ? a->status() : Attendee::None);
        if (!a->delegator().isEmpty()) {
            tmpStr += i18n(" (delegated by %1)", a->delegator());
        }
        if (!a->delegate().isEmpty()) {
            tmpStr += i18n(" (delegated to %1)", a->delegate());
        }
        tmpStr += QLatin1String("<br>");
    }
    if (tmpStr.endsWith(QLatin1String("<br>"))) {
        tmpStr.chop(4);
    }
    return tmpStr;
}

static QString displayViewFormatAttendees(Calendar::Ptr calendar, Incidence::Ptr incidence)
{
    QString tmpStr, str;

    // Add organizer link
    int attendeeCount = incidence->attendees().count();
    if (attendeeCount > 1 ||
            (attendeeCount == 1 &&
             !attendeeIsOrganizer(incidence, incidence->attendees().first()))) {

        QPair<QString, QString> s = searchNameAndUid(incidence->organizer()->email(),
                                    incidence->organizer()->name(),
                                    QString());
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Organizer:") + QLatin1String("</b></td>");
        const QString iconPath =
            KIconLoader::global()->iconPath(QLatin1String("meeting-organizer"), KIconLoader::Small);
        tmpStr += QLatin1String("<td>") + displayViewFormatPerson(incidence->organizer()->email(),
                  s.first, s.second, iconPath) +
                  QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    // Show the attendee status if the incidence's organizer owns the resource calendar,
    // which means they are running the show and have all the up-to-date response info.
    bool showStatus = incOrganizerOwnsCalendar(calendar, incidence);

    // Add "chair"
    str = displayViewFormatAttendeeRoleList(incidence, Attendee::Chair, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Chair:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + str + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    // Add required participants
    str = displayViewFormatAttendeeRoleList(incidence, Attendee::ReqParticipant, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Required Participants:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + str + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    // Add optional participants
    str = displayViewFormatAttendeeRoleList(incidence, Attendee::OptParticipant, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Optional Participants:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + str + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    // Add observers
    str = displayViewFormatAttendeeRoleList(incidence, Attendee::NonParticipant, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Observers:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + str + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    return tmpStr;
}

static QString displayViewFormatAttachments(Incidence::Ptr incidence)
{
    QString tmpStr;
    Attachment::List as = incidence->attachments();
    Attachment::List::ConstIterator it;
    int count = 0;
    for (it = as.constBegin(); it != as.constEnd(); ++it) {
        count++;
        if ((*it)->isUri()) {
            QString name;
            if ((*it)->uri().startsWith(QLatin1String("kmail:"))) {
                name = i18n("Show mail");
            } else {
                if ((*it)->label().isEmpty()) {
                    name = (*it)->uri();
                } else {
                    name = (*it)->label();
                }
            }
            tmpStr += htmlAddLink((*it)->uri(), name);
        } else {
            tmpStr += htmlAddLink(QString::fromLatin1("ATTACH:%1").
                                  arg(QString::fromUtf8((*it)->label().toUtf8().toBase64())),
                                  (*it)->label());
        }
        if (count < as.count()) {
            tmpStr += QLatin1String("<br>");
        }
    }
    return tmpStr;
}

static QString displayViewFormatCategories(Incidence::Ptr incidence)
{
    // We do not use Incidence::categoriesStr() since it does not have whitespace
    return incidence->categories().join(QLatin1String(", "));
}

static QString displayViewFormatCreationDate(Incidence::Ptr incidence, KDateTime::Spec spec)
{
    KDateTime kdt = incidence->created().toTimeSpec(spec);
    return i18n("Creation date: %1", dateTimeToString(incidence->created(), false, true, spec));
}

static QString displayViewFormatBirthday(Event::Ptr event)
{
    if (!event) {
        return QString();
    }
    if (event->customProperty("KABC", "BIRTHDAY") != QLatin1String("YES") &&
            event->customProperty("KABC", "ANNIVERSARY") != QLatin1String("YES")) {
        return QString();
    }

    const QString uid_1 = event->customProperty("KABC", "UID-1");
    const QString name_1 = event->customProperty("KABC", "NAME-1");
    const QString email_1= event->customProperty("KABC", "EMAIL-1");

    KCalCore::Person::Ptr p = Person::fromFullName(email_1);

    const QString tmpStr = displayViewFormatPerson(p->email(), name_1, uid_1, QString());
    return tmpStr;
}

static QString displayViewFormatHeader(Incidence::Ptr incidence)
{
    QString tmpStr = QLatin1String("<table><tr>");

    // show icons
    KIconLoader *iconLoader = KIconLoader::global();
    tmpStr += QLatin1String("<td>");

    QString iconPath;
    if (incidence->customProperty("KABC", "BIRTHDAY") == QLatin1String("YES")) {
        iconPath = iconLoader->iconPath(QLatin1String("view-calendar-birthday"), KIconLoader::Small);
    } else if (incidence->customProperty("KABC", "ANNIVERSARY") == QLatin1String("YES")) {
        iconPath = iconLoader->iconPath(QLatin1String("view-calendar-wedding-anniversary"), KIconLoader::Small);
    } else {
        iconPath = iconLoader->iconPath(incidence->iconName(), KIconLoader::Small);
    }
    tmpStr += QLatin1String("<img valign=\"top\" src=\"") + iconPath + QLatin1String("\">");

    if (incidence->hasEnabledAlarms()) {
        tmpStr += QLatin1String("<img valign=\"top\" src=\"") +
                  iconLoader->iconPath(QLatin1String("preferences-desktop-notification-bell"), KIconLoader::Small) +
                  QLatin1String("\">");
    }
    if (incidence->recurs()) {
        tmpStr += QLatin1String("<img valign=\"top\" src=\"") +
                  iconLoader->iconPath(QLatin1String("edit-redo"), KIconLoader::Small) +
                  QLatin1String("\">");
    }
    if (incidence->isReadOnly()) {
        tmpStr += QLatin1String("<img valign=\"top\" src=\"") +
                  iconLoader->iconPath(QLatin1String("object-locked"), KIconLoader::Small) +
                  QLatin1String("\">");
    }
    tmpStr += QLatin1String("</td>");

    tmpStr += QLatin1String("<td>");
    tmpStr += QLatin1String("<b><u>") + incidence->richSummary() + QLatin1String("</u></b>");
    tmpStr += QLatin1String("</td>");

    tmpStr += QLatin1String("</tr></table>");

    return tmpStr;
}


/* Get a pretty one line summary of an event so that it can be used in a list */
static QString displayViewFormatEventForList(const Calendar::Ptr &calendar,
                                             const Event::Ptr &event,
                                             bool noHtmlMode,
                                             KDateTime::Spec spec)
{
    if (!event || !calendar) {
        return QString::null;
    }

    QString tmpStr;

    tmpStr += invitationSummary(event, noHtmlMode);
    tmpStr += QLatin1String(": ") + IncidenceFormatter::formatStartEnd(event->dtStart(), event->dtEnd(), spec);

    QString calStr = IncidenceFormatter::resourceString(calendar, event);

    if (!calStr.isEmpty()) {
        tmpStr += QString::fromLatin1("<small> (%1)</small>").arg(calStr);
    }

    return tmpStr;
}

static bool slicesInterval(const Event::Ptr &event,
                           const KDateTime &startDt,
                           const KDateTime &endDt)
{
    KDateTime closestStart = event->dtStart();
    KDateTime closestEnd = event->dtEnd();
    if (event->recurs()) {
        if (!event->recurrence()->timesInInterval(startDt, endDt).isEmpty()) {
            // If there is a recurrence in this interval we know already that we slice.
            return true;
        }
        closestStart = event->recurrence()->getPreviousDateTime(startDt);
        if (event->hasEndDate()) {
            closestEnd = closestStart.addSecs(event->dtStart().secsTo(event->dtEnd()));
        }
    } else {
        if (!event->hasEndDate() && event->hasDuration()) {
            closestEnd = closestStart.addSecs(event->duration());
        }
    }

    if (!closestEnd.isValid ()) {
        // All events without an ending still happen if they are
        // started.
        return closestStart <= startDt;
    }

    if (closestStart <= startDt) {
        // It starts before the interval and ends after the start of the interval.
        return closestEnd > startDt;
    }

    // Are start and end both in this interval?
    return (closestStart >= startDt && closestStart <= endDt) &&
           (closestEnd >= startDt && closestEnd <= endDt);
}

/* Format the events on the same day list for an invitation. */
static QString displayViewFormatEventsOnSameDays(InvitationFormatterHelper *helper,
                                                 const Event::Ptr &event,
                                                 KDateTime::Spec spec,
                                                 bool noHtmlMode)
{
    if (!event || !helper) {
        return QString::null;
    }

    // Check calendar
    const QString checkCalBtn = inviteButton(helper, QLatin1String("check_calendar"),
                                             i18n("Check calendar..." ),
                                             QLatin1String("go-jump-today"));

    KDateTime startDay = event->dtStart();
    KDateTime endDay = event->hasEndDate() ? event->dtEnd() : event->dtStart();
    startDay.setTime(QTime(0, 0, 0));
    endDay.setTime(QTime(23, 59, 59));

    Event::List matchingEvents = helper->calendar()->events(startDay.date(), endDay.date(), spec);
    if (matchingEvents.isEmpty()) {
        return checkCalBtn;
    }

    QString tmpStr;
    tmpStr += QLatin1String("<span class=\"leftColumn\">");
    if (event->hasEndDate() && event->dateEnd().daysTo(event->dtStart().date()) > 0) {
        tmpStr += i18n("Events on these days:");
    } else {
        tmpStr += i18n("Events on this day:");
    }
    tmpStr += QString::fromLatin1("&nbsp;&nbsp;%1</span>\n").arg(checkCalBtn);
    tmpStr += QLatin1String("<ul>\n");
    int count = 0;
    for (Event::List::ConstIterator it = matchingEvents.begin(), end = matchingEvents.end();
          it != end && count < 50;
          ++it) {
        if ((*it)->schedulingID() == event->uid()) {
            // Exclude the same event from the list.
            continue;
        }
        if (!slicesInterval(*it, startDay, endDay)) {
            /* Calendar::events includes events that have a recurrence that is
             * "active" in the specified interval. Wether or not the event is actually
             * happening ( has a recurrence that falls into the interval ).
             * This appears to be done deliberately and not to be a bug so we additionally
             * check if the event is actually happening here. */
            continue;
        }
        ++count;
        tmpStr += QString::fromLatin1("<li>%1</li>\n")
                    .arg(displayViewFormatEventForList(helper->calendar(), *it, noHtmlMode, spec));
    }
    if (count == 0) {
        /* Only the same event on this day. */
        return checkCalBtn;
    }
    if (count == 50) {
        /* Abort after 50 entries to limit resource usage */
        tmpStr += QLatin1String("<li>...</li>\n");
    }
    tmpStr += QLatin1String("</ul>");
    return tmpStr;
}

static QString displayViewFormatEvent(const Calendar::Ptr calendar, const QString &sourceName,
                                      const Event::Ptr &event,
                                      const QDate &date, KDateTime::Spec spec)
{
    if (!event) {
        return QString();
    }

    QString tmpStr = displayViewFormatHeader(event);

    tmpStr += QLatin1String("<table>");
    tmpStr += QLatin1String("<col width=\"25%\"/>");
    tmpStr += QLatin1String("<col width=\"75%\"/>");

    const QString calStr = calendar ? resourceString(calendar, event) : sourceName;
    if (!calStr.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Calendar:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + calStr + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    if (!event->location().isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Location:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + event->richLocation() + QLatin1String("</td>");
        tmpStr +=QLatin1String("</tr>");
    }

    KDateTime startDt = event->dtStart();
    KDateTime endDt = event->dtEnd();
    if (event->recurs()) {
        if (date.isValid()) {
            KDateTime kdt(date, QTime(0, 0, 0), KSystemTimeZones::local());
            int diffDays = startDt.daysTo(kdt);
            kdt = kdt.addSecs(-1);
            startDt.setDate(event->recurrence()->getNextDateTime(kdt).date());
            if (event->hasEndDate()) {
                endDt = endDt.addDays(diffDays);
                if (startDt > endDt) {
                    startDt.setDate(event->recurrence()->getPreviousDateTime(kdt).date());
                    endDt = startDt.addDays(event->dtStart().daysTo(event->dtEnd()));
                }
            }
        }
    }

    tmpStr += QLatin1String("<tr>");
    if (event->allDay()) {
        if (event->isMultiDay()) {
            tmpStr += QLatin1String("<td><b>") + i18n("Date:") + QLatin1String("</b></td>");
            tmpStr += QLatin1String("<td>") +
                      i18nc("<beginTime> - <endTime>","%1 - %2",
                            dateToString(startDt, false, spec),
                            dateToString(endDt, false, spec)) +
                      QLatin1String("</td>");
        } else {
            tmpStr += QLatin1String("<td><b>") + i18n("Date:") + QLatin1String("</b></td>");
            tmpStr += QLatin1String("<td>") +
                      i18nc("date as string","%1",
                            dateToString(startDt, false, spec)) +
                      QLatin1String("</td>");
        }
    } else {
        if (event->isMultiDay()) {
            tmpStr += QLatin1String("<td><b>") + i18n("Date:") + QLatin1String("</b></td>");
            tmpStr += QLatin1String("<td>") +
                      i18nc("<beginTime> - <endTime>","%1 - %2",
                            dateToString(startDt, false, spec),
                            dateToString(endDt, false, spec)) +
                      QLatin1String("</td>");
        } else {
            tmpStr += QLatin1String("<td><b>") + i18n("Date:") + QLatin1String("</b></td>");
            tmpStr += QLatin1String("<td>") +
                      i18nc("date as string", "%1",
                            dateToString(startDt, false, spec)) +
                      QLatin1String("</td>");

            tmpStr += QLatin1String("</tr><tr>");
            tmpStr += QLatin1String("<td><b>") + i18n("Time:") + QLatin1String("</b></td>");
            if (event->hasEndDate() && startDt != endDt) {
                tmpStr += QLatin1String("<td>") +
                          i18nc("<beginTime> - <endTime>","%1 - %2",
                                timeToString(startDt, true, spec),
                                timeToString(endDt, true, spec)) +
                          QLatin1String("</td>");
            } else {
                tmpStr += QLatin1String("<td>") +
                          timeToString(startDt, true, spec) +
                          QLatin1String("</td>");
            }
        }
    }
    tmpStr += QLatin1String("</tr>");

    QString durStr = durationString(event);
    if (!durStr.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Duration:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + durStr + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    if (event->recurs() || event->hasRecurrenceId()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Recurrence:") + QLatin1String("</b></td>");

        QString str;
        if (event->hasRecurrenceId()) {
            str = i18n("Exception");
        } else {
            str = recurrenceString(event);
        }

        tmpStr += QLatin1String("<td>") + str +
                  QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    const bool isBirthday = event->customProperty("KABC", "BIRTHDAY") == QLatin1String("YES");
    const bool isAnniversary = event->customProperty("KABC", "ANNIVERSARY") == QLatin1String("YES");

    if (isBirthday || isAnniversary) {
        tmpStr += QLatin1String("<tr>");
        if (isAnniversary) {
            tmpStr += QLatin1String("<td><b>") + i18n("Anniversary:") + QLatin1String("</b></td>");
        } else {
            tmpStr += QLatin1String("<td><b>") + i18n("Birthday:") + QLatin1String("</b></td>");
        }
        tmpStr += QLatin1String("<td>") + displayViewFormatBirthday(event) + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
        tmpStr += QLatin1String("</table>");
        return tmpStr;
    }

    tmpStr += displayViewFormatDescription(event);

    // TODO: print comments?

    int reminderCount = event->alarms().count();
    if (reminderCount > 0 && event->hasEnabledAlarms()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") +
                  i18np("Reminder:", "Reminders:", reminderCount) +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + reminderStringList(event).join(QLatin1String("<br>")) + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    tmpStr += displayViewFormatAttendees(calendar, event);

    int categoryCount = event->categories().count();
    if (categoryCount > 0) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>");
        tmpStr += i18np("Category:", "Categories:", categoryCount) +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + displayViewFormatCategories(event) + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    int attachmentCount = event->attachments().count();
    if (attachmentCount > 0) {
        const QString formattedAttachments = displayViewFormatAttachments(event);
        if (!formattedAttachments.isEmpty()) {
            tmpStr += QLatin1String("<tr>");
            tmpStr += QLatin1String("<td><b>") + i18np("Attachment:", "Attachments:", attachmentCount) + QLatin1String("</b></td>");
            tmpStr += QLatin1String("<td>") + formattedAttachments + QLatin1String("</td>");
            tmpStr += QLatin1String("</tr>");
        }
    }
    tmpStr += QLatin1String("</table>");

    tmpStr += QLatin1String("<p><em>") + displayViewFormatCreationDate(event, spec) + QLatin1String("</em>");

    return tmpStr;
}

static QString displayViewFormatTodo(const Calendar::Ptr &calendar, const QString &sourceName,
                                     const Todo::Ptr &todo,
                                     const QDate &ocurrenceDueDate, KDateTime::Spec spec)
{
    if (!todo) {
        kDebug() << "IncidenceFormatter::displayViewFormatTodo was called without to-do, quitting";
        return QString();
    }

    QString tmpStr = displayViewFormatHeader(todo);

    tmpStr += QLatin1String("<table>");
    tmpStr += QLatin1String("<col width=\"25%\"/>");
    tmpStr += QLatin1String("<col width=\"75%\"/>");

    const QString calStr = calendar ? resourceString(calendar, todo) : sourceName;
    if (!calStr.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Calendar:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + calStr + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    if (!todo->location().isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Location:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + todo->richLocation() + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    const bool hastStartDate = todo->hasStartDate();
    const bool hasDueDate = todo->hasDueDate();

    if (hastStartDate) {
        KDateTime startDt = todo->dtStart(true /**first*/);
        if (todo->recurs() && ocurrenceDueDate.isValid()) {
            if (hasDueDate) {
                // In kdepim all recuring to-dos have due date.
                const int length = startDt.daysTo(todo->dtDue(true /**first*/));
                if (length >= 0) {
                    startDt.setDate(ocurrenceDueDate.addDays(-length));
                } else {
                    kError() << "DTSTART is bigger than DTDUE, todo->uid() is " << todo->uid();
                    startDt.setDate(ocurrenceDueDate);
                }
            } else {
                kError() << "To-do is recurring but has no DTDUE set, todo->uid() is " << todo->uid();
                startDt.setDate(ocurrenceDueDate);
            }
        }
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") +
                  i18nc("to-do start date/time", "Start:") +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") +
                  dateTimeToString(startDt, todo->allDay(), false, spec) +
                  QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    if (hasDueDate) {
        KDateTime dueDt = todo->dtDue();
        if (todo->recurs()) {
            if (ocurrenceDueDate.isValid()) {
                KDateTime kdt(ocurrenceDueDate, QTime(0, 0, 0), KSystemTimeZones::local());
                kdt = kdt.addSecs(-1);
                dueDt.setDate(todo->recurrence()->getNextDateTime(kdt).date());
            }
        }
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") +
                  i18nc("to-do due date/time", "Due:") +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") +
                  dateTimeToString(dueDt, todo->allDay(), false, spec) +
                  QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    QString durStr = durationString(todo);
    if (!durStr.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Duration:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + durStr + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    if (todo->recurs() || todo->hasRecurrenceId()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>")+ i18n("Recurrence:") + QLatin1String("</b></td>");
        QString str;
        if (todo->hasRecurrenceId()) {
            str = i18n("Exception");
        } else {
            str = recurrenceString(todo);
        }
        tmpStr += QLatin1String("<td>") +
                  str +
                  QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    tmpStr += displayViewFormatDescription(todo);

    // TODO: print comments?

    int reminderCount = todo->alarms().count();
    if (reminderCount > 0 && todo->hasEnabledAlarms()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") +
                  i18np("Reminder:", "Reminders:", reminderCount) +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + reminderStringList(todo).join(QLatin1String("<br>")) + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    tmpStr += displayViewFormatAttendees(calendar, todo);

    int categoryCount = todo->categories().count();
    if (categoryCount > 0) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") +
                  i18np("Category:", "Categories:", categoryCount) +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + displayViewFormatCategories(todo) + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    if (todo->priority() > 0) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Priority:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>");
        tmpStr += QString::number(todo->priority());
        tmpStr += QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    tmpStr += QLatin1String("<tr>");
    if (todo->isCompleted()) {
        tmpStr += QLatin1String("<td><b>") + i18nc("Completed: date", "Completed:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>");
        tmpStr += Stringify::todoCompletedDateTime(todo);
    } else {
        tmpStr += QLatin1String("<td><b>") + i18n("Percent Done:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>");
        tmpStr += i18n("%1%", todo->percentComplete());
    }
    tmpStr += QLatin1String("</td>");
    tmpStr += QLatin1String("</tr>");

    int attachmentCount = todo->attachments().count();
    if (attachmentCount > 0) {
        const QString formattedAttachments = displayViewFormatAttachments(todo);
        if (!formattedAttachments.isEmpty()) {
            tmpStr += QLatin1String("<tr>");
            tmpStr += QLatin1String("<td><b>") +
                      i18np("Attachment:", "Attachments:", attachmentCount) +
                      QLatin1String("</b></td>");
            tmpStr += QLatin1String("<td>") + formattedAttachments + QLatin1String("</td>");
            tmpStr += QLatin1String("</tr>");
        }
    }
    tmpStr += QLatin1String("</table>");

    tmpStr += QLatin1String("<p><em>")+ displayViewFormatCreationDate(todo, spec) + QLatin1String("</em>");

    return tmpStr;
}

static QString displayViewFormatJournal(const Calendar::Ptr &calendar, const QString &sourceName,
                                        const Journal::Ptr &journal, KDateTime::Spec spec)
{
    if (!journal) {
        return QString();
    }

    QString tmpStr = displayViewFormatHeader(journal);

    tmpStr += QLatin1String("<table>");
    tmpStr += QLatin1String("<col width=\"25%\"/>");
    tmpStr += QLatin1String("<col width=\"75%\"/>");

    const QString calStr = calendar ? resourceString(calendar, journal) : sourceName;
    if (!calStr.isEmpty()) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") + i18n("Calendar:") + QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + calStr + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    tmpStr += QLatin1String("<tr>");
    tmpStr += QLatin1String("<td><b>") + i18n("Date:") + QLatin1String("</b></td>");
    tmpStr += QLatin1String("<td>") +
              dateToString(journal->dtStart(), false, spec) +
              QLatin1String("</td>");
    tmpStr += QLatin1String("</tr>");

    tmpStr += displayViewFormatDescription(journal);

    int categoryCount = journal->categories().count();
    if (categoryCount > 0) {
        tmpStr += QLatin1String("<tr>");
        tmpStr += QLatin1String("<td><b>") +
                  i18np("Category:", "Categories:", categoryCount) +
                  QLatin1String("</b></td>");
        tmpStr += QLatin1String("<td>") + displayViewFormatCategories(journal) + QLatin1String("</td>");
        tmpStr += QLatin1String("</tr>");
    }

    tmpStr += QLatin1String("</table>");

    tmpStr += QLatin1String("<p><em>") + displayViewFormatCreationDate(journal, spec) + QLatin1String("</em>");

    return tmpStr;
}

static QString displayViewFormatFreeBusy(const Calendar::Ptr &calendar, const QString &sourceName,
        const FreeBusy::Ptr &fb, KDateTime::Spec spec)
{
    Q_UNUSED(calendar);
    Q_UNUSED(sourceName);
    if (!fb) {
        return QString();
    }

    QString tmpStr(
        htmlAddTag(
            QLatin1String("h2"), i18n("Free/Busy information for %1", fb->organizer()->fullName())));

    tmpStr += htmlAddTag(QLatin1String("h4"),
                         i18n("Busy times in date range %1 - %2:",
                              dateToString(fb->dtStart(), true, spec),
                              dateToString(fb->dtEnd(), true, spec)));

    QString text =
        htmlAddTag(QLatin1String("em"),
                   htmlAddTag(QLatin1String("b"), i18nc("tag for busy periods list", "Busy:")));

    Period::List periods = fb->busyPeriods();
    Period::List::iterator it;
    for (it = periods.begin(); it != periods.end(); ++it) {
        Period per = *it;
        if (per.hasDuration()) {
            int dur = per.duration().asSeconds();
            QString cont;
            if (dur >= 3600) {
                cont += i18ncp("hours part of duration", "1 hour ", "%1 hours ", dur / 3600);
                dur %= 3600;
            }
            if (dur >= 60) {
                cont += i18ncp("minutes part duration", "1 minute ", "%1 minutes ", dur / 60);
                dur %= 60;
            }
            if (dur > 0) {
                cont += i18ncp("seconds part of duration", "1 second", "%1 seconds", dur);
            }
            text += i18nc("startDate for duration", "%1 for %2",
                          dateTimeToString(per.start(), false, true, spec),
                          cont);
            text += QLatin1String("<br>");
        } else {
            if (per.start().date() == per.end().date()) {
                text += i18nc("date, fromTime - toTime ", "%1, %2 - %3",
                              dateToString(per.start(), true, spec),
                              timeToString(per.start(), true, spec),
                              timeToString(per.end(), true, spec));
            } else {
                text += i18nc("fromDateTime - toDateTime", "%1 - %2",
                              dateTimeToString(per.start(), false, true, spec),
                              dateTimeToString(per.end(), false, true, spec));
            }
            text += QLatin1String("<br>");
        }
    }
    tmpStr += htmlAddTag(QLatin1String("p"), text);
    return tmpStr;
}
//@endcond

//@cond PRIVATE
class KCalUtils::IncidenceFormatter::EventViewerVisitor : public Visitor
{
public:
    EventViewerVisitor()
        : mCalendar(0), mSpec(KDateTime::Spec()), mResult(QLatin1String("")) {}

    bool act(const Calendar::Ptr &calendar, IncidenceBase::Ptr incidence, const QDate &date,
             KDateTime::Spec spec=KDateTime::Spec())
    {
        mCalendar = calendar;
        mSourceName.clear();
        mDate = date;
        mSpec = spec;
        mResult = QLatin1String("");
        return incidence->accept(*this, incidence);
    }

    bool act(const QString &sourceName, IncidenceBase::Ptr incidence, const QDate &date,
             KDateTime::Spec spec=KDateTime::Spec())
    {
        mSourceName = sourceName;
        mDate = date;
        mSpec = spec;
        mResult = QLatin1String("");
        return incidence->accept(*this, incidence);
    }

    QString result() const {
        return mResult;
    }

protected:
    bool visit(Event::Ptr event)
    {
        mResult = displayViewFormatEvent(mCalendar, mSourceName, event, mDate, mSpec);
        return !mResult.isEmpty();
    }
    bool visit(Todo::Ptr todo)
    {
        mResult = displayViewFormatTodo(mCalendar, mSourceName, todo, mDate, mSpec);
        return !mResult.isEmpty();
    }
    bool visit(Journal::Ptr journal)
    {
        mResult = displayViewFormatJournal(mCalendar, mSourceName, journal, mSpec);
        return !mResult.isEmpty();
    }
    bool visit(FreeBusy::Ptr fb)
    {
        mResult = displayViewFormatFreeBusy(mCalendar, mSourceName, fb, mSpec);
        return !mResult.isEmpty();
    }

protected:
    Calendar::Ptr mCalendar;
    QString mSourceName;
    QDate mDate;
    KDateTime::Spec mSpec;
    QString mResult;
};
//@endcond

QString IncidenceFormatter::extensiveDisplayStr(const Calendar::Ptr &calendar,
        const IncidenceBase::Ptr &incidence,
        const QDate &date,
        KDateTime::Spec spec)
{
    if (!incidence) {
        return QString();
    }

    EventViewerVisitor v;
    if (v.act(calendar, incidence, date, spec)) {
        return v.result();
    } else {
        return QString();
    }
}

QString IncidenceFormatter::extensiveDisplayStr(const QString &sourceName,
        const IncidenceBase::Ptr &incidence,
        const QDate &date,
        KDateTime::Spec spec)
{
    if (!incidence) {
        return QString();
    }

    EventViewerVisitor v;
    if (v.act(sourceName, incidence, date, spec)) {
        return v.result();
    } else {
        return QString();
    }
}
/***********************************************************************
 *  Helper functions for the body part formatter of kmail (Invitations)
 ***********************************************************************/

//@cond PRIVATE
static QString cleanHtml(const QString &html)
{
    QRegExp rx(QLatin1String("<body[^>]*>(.*)</body>"), Qt::CaseInsensitive);
    rx.indexIn(html);
    QString body = rx.cap(1).isEmpty() ? html : rx.cap(1);

    return Qt::escape(body.remove(QRegExp(QLatin1String("<[^>]*>"))).trimmed());
}

static QString invitationSummary(const Incidence::Ptr &incidence, bool noHtmlMode)
{
    QString summaryStr = i18n("Summary unspecified");
    if (!incidence->summary().isEmpty()) {
        if (!incidence->summaryIsRich()) {
            summaryStr = Qt::escape(incidence->summary());
        } else {
            summaryStr = incidence->richSummary();
            if (noHtmlMode) {
                summaryStr = cleanHtml(summaryStr);
            }
        }
    }
    return summaryStr;
}

static QString invitationLocation(const Incidence::Ptr &incidence, bool noHtmlMode)
{
    QString locationStr;
    if (!incidence->location().isEmpty()) {
        if (!incidence->locationIsRich()) {
            locationStr = Qt::escape(incidence->location());
        } else {
            locationStr = incidence->richLocation();
            if (noHtmlMode) {
                locationStr = cleanHtml(locationStr);
            }
        }
    }
    return locationStr;
}

static QString htmlInvitationDetailsBegin(const QString &iconName, const QString &caption)
{
    QString dir = (QApplication::isRightToLeft() ? QLatin1String("rtl") : QLatin1String("ltr"));
    QString html = QString::fromLatin1("<div dir=\"%1\">\n").arg(dir);
    // Start with a caption and an identifing icon on the side
    html += QString::fromLatin1("<h2 class=\"summary\"><img src=\"%1\"/>%2</h2>\n")
              .arg(KIconLoader::global()->iconPath(iconName, KIconLoader::Desktop),
                   caption);
    return html;
}

static QString htmlInvitationDetailsEnd()
{
    return QLatin1String("</div>\n");
}

static QString diffColor()
{
    // Color for printing comparison differences inside invitations.

//  return  "#DE8519"; // hard-coded color from Outlook2007
    return QColor(Qt::red).name();    //krazy:exclude=qenums TODO make configurable
}

static QString noteColor()
{
    // Color for printing notes inside invitations.
    return qApp->palette().color(QPalette::Active, QPalette::Highlight).name();
}

static QString htmlCompare(const QString &value, const QString &oldvalue)
{
    // if 'value' is empty, then print nothing
    if (value.isEmpty()) {
        return QString::null;
    }

    // if 'value' is new or unchanged, then print normally
    if (oldvalue.isEmpty() || value == oldvalue) {
        return value;
    }

    // if 'value' has changed, then make a special print
    QString color = diffColor();
    QString newvalue = QLatin1String("<font color=\"") + color + QLatin1String("\">") + value + QLatin1String("</font>") +
                       QLatin1String("&nbsp;") +
                       QLatin1String("(<strike>") + oldvalue + QLatin1String("</strike>)");
    return newvalue;
}

static QString htmlRow(const QString &title, const QString &value)
{
    if (!value.isEmpty()) {
        return QLatin1String("<tr><td class=\"leftColumn\">") + title + QLatin1String("</td>\n<td>") + value + QLatin1String("</td></tr>\n");
    } else {
        return QString();
    }
}

static QString htmlRow(const QString &title, const QString &value, const QString &oldvalue)
{
    // if 'value' is empty, then print nothing
    if (value.isEmpty()) {
        return QString();
    }

    return htmlRow(title, htmlCompare(value, oldvalue));
}

static Attendee::Ptr findDelegatedFromMyAttendee(const Incidence::Ptr &incidence)
{
    // Return the first attendee that was delegated-from the user

    Attendee::Ptr attendee;
    if (!incidence) {
        return attendee;
    }

    RAIIIdentityManager raiiHelper;
    QString delegatorName, delegatorEmail;
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
        Attendee::Ptr a = *it;
        KPIMUtils::extractEmailAddressAndName(a->delegator(), delegatorEmail, delegatorName);
        if (thatIsMe(delegatorEmail)) {
            attendee = a;
            break;
        }
    }

    return attendee;
}

static Attendee::Ptr findMyAttendee(const Incidence::Ptr &incidence)
{
    // Return the attendee for the incidence that is probably the user

    Attendee::Ptr attendee;
    if (!incidence) {
        return attendee;
    }

    RAIIIdentityManager raiiHelper;
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
        Attendee::Ptr a = *it;
        if (thatIsMe(a->email())) {
            attendee = a;
            break;
        }
    }

    return attendee;
}

static Attendee::Ptr findAttendee(const Incidence::Ptr &incidence,
                                  const QString &email)
{
    // Search for an attendee by email address

    Attendee::Ptr attendee;
    if (!incidence) {
        return attendee;
    }

    RAIIIdentityManager raiiHelper;
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
        Attendee::Ptr a = *it;
        if (email == a->email()) {
            attendee = a;
            break;
        }
    }
    return attendee;
}

static bool rsvpRequested(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return false;
    }

    //use a heuristic to determine if a response is requested.

    bool rsvp = true; // better send superfluously than not at all
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
        if (it == attendees.constBegin()) {
            rsvp = (*it)->RSVP(); // use what the first one has
        } else {
            if ((*it)->RSVP() != rsvp) {
                rsvp = true; // they differ, default
                break;
            }
        }
    }
    return rsvp;
}

static QString rsvpRequestedStr(bool rsvpRequested, const QString &role)
{
    if (rsvpRequested) {
        if (role.isEmpty()) {
            return i18n("Your response is requested");
        } else {
            return i18n("Your response as <b>%1</b> is requested", role);
        }
    } else {
        if (role.isEmpty()) {
            return i18n("No response is necessary");
        } else {
            return i18n("No response as <b>%1</b> is necessary", role);
        }
    }
}

static QString myStatusStr(Incidence::Ptr incidence)
{
    QString ret;
    Attendee::Ptr a = findMyAttendee(incidence);
    if (a &&
            a->status() != Attendee::NeedsAction && a->status() != Attendee::Delegated) {
        ret = i18n("(Note: the Organizer preset your response to <b>%1</b>)",
                   Stringify::attendeeStatus(a->status()));
    }
    return ret;
}

static QString invitationNote(const QString &title, const QString &note,
                              const QString &tag, const QString &color)
{
    QString noteStr;
    if (!note.isEmpty()) {
        noteStr += QLatin1String("<table border=\"0\" style=\"margin-top:4px;\">");
        noteStr += QLatin1String("<tr><center><td>");
        if (!color.isEmpty()) {
            noteStr += QLatin1String("<font color=\"") + color + QLatin1String("\">");
        }
        if (!title.isEmpty()) {
            if (!tag.isEmpty()) {
                noteStr += htmlAddTag(tag, title);
            } else {
                noteStr += title;
            }
        }
        noteStr += QLatin1String("&nbsp;)") + note;
        if (!color.isEmpty()) {
            noteStr += QLatin1String("</font>");
        }
        noteStr += QLatin1String("</td></center></tr>");
        noteStr += QLatin1String("</table>");
    }
    return noteStr;
}

static QString invitationPerson(const QString &email, const QString &name, const QString &uid,
                                const QString &comment)
{
    QPair<QString, QString> s = searchNameAndUid(email, name, uid);
    const QString printName = s.first;
    const QString printUid = s.second;

    QString personString;
    // Make the uid link
    if (!printUid.isEmpty()) {
        personString = htmlAddUidLink(email, printName, printUid);
    }
    // Make the mailto link
    if (!email.isEmpty()) {
        personString += QLatin1String("&nbsp;") + htmlAddMailtoLink(email, printName);
    } else {
        personString = (printName.isEmpty() ? email : printName);
    }

    if (!comment.isEmpty()) {
        // beware  personString might already contain percent escaped values.
        // so you can not chain qt arg replacement here.
        // personString = i18nc("name (comment)", "%1 (%2)", personString, comment);
        personString += QString::fromLatin1(" (%2)").arg(comment);
    }

    personString += QLatin1Char('\n');

    return personString;
}

static QString invitationCommentsIncidence(const Incidence::Ptr &incidence, bool noHtmlMode)
{
    QString html;
    QStringList comments;

    if (!incidence || incidence->comments().isEmpty()) {
        return QString::null;
    }

    // non-empty comments
    QStringList cl = incidence->comments();
    for (QStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
        if (!Qt::mightBeRichText(*it)) {
            comments.append(string2HTML(*it));
        } else {
            if (noHtmlMode) {
                comments.append(cleanHtml(*it));
            } else {
              comments.append(*it);
            }
        }
    }

    if (!comments.isEmpty()) {
        html += QLatin1String("<table><tr>\n<td class=\"leftColumn\">");
        if (comments.count() > 1) {
            html += i18n("Comments:") + QLatin1String("</td>\n<td>\n<ul>\n");
            for (int i = 0; i < comments.count(); ++i) {
                html += QLatin1String("<li>") + comments[i] + QLatin1String("</li>\n");
            }
            html += QLatin1String("</ul>\n");
        } else {
            html += i18n("Comment:") + QLatin1String("</td>\n<td>\n");
            html += comments[0];
        }
        html += QLatin1String("\n</td>\n</tr></table>");
    }

    return html;
}

static QString invitationDescriptionIncidence(const Incidence::Ptr &incidence, bool noHtmlMode)
{
    QString html;
    QString descr;

    if (!incidence->description().isEmpty()) {
        // use description as comments
        if (!incidence->descriptionIsRich()) {
            descr = string2HTML(incidence->description());
        } else {
            descr = incidence->description();
            if (noHtmlMode) {
                descr = cleanHtml(descr);
            }
        }
    }

    if (!descr.isEmpty()) {
        html += QLatin1String("<tr>\n<td class=\"leftColumn\">") + i18n("Description:") + QLatin1String("</td>\n");
        html += QLatin1String("<td>") + descr + QLatin1String("</td>\n</tr>\n");
    }
    return html;
}

QString IncidenceFormatter::formatStartEnd(const KDateTime &start, const KDateTime &end,
                                           const KDateTime::Spec &spec)
{
    QString tmpStr;
    // <startDate[time> [- <[endDate][Time]>]
    // The startDate is always printed.
    // If the event does float the time is omitted.
    //
    // If it has an end dateTime:
    // on the same day -> Only add end time.
    // if it floats also omit the time
    tmpStr += IncidenceFormatter::dateTimeToString(start, false, true, spec);

    if (end.isValid()) {
        if (start.date() == end.date()) {
            // same day
            if (start.time().isValid()) {
                tmpStr += QLatin1String(" - ") + IncidenceFormatter::timeToString(end, true, spec);
            }
        } else {
            tmpStr += QLatin1String(" - ") + IncidenceFormatter::dateTimeToString(end, false, true, spec);
        }
    }
    return tmpStr;
}

static QString invitationDetailsEvent(const Event::Ptr &event,
                                      bool noHtmlMode,
                                      KDateTime::Spec spec)
{
    // Invitation details are formatted into an HTML table
    if (!event) {
        return QString::null;
    }

    QString html = htmlInvitationDetailsBegin(QLatin1String("view-pim-calendar"),
                                              invitationSummary(event, noHtmlMode));

    html += htmlRow(event->recurs() ? i18n("First event:") : i18n("When:"),
                    IncidenceFormatter::formatStartEnd(event->dtStart(), event->dtEnd(), spec));

    const QString location = invitationLocation(event, noHtmlMode);

    if (!location.isEmpty()) {
        html += htmlRow(i18n("Where:"), location);
    }

    if (event->recurs()) {
        html += htmlRow(i18n("Recurrence:"), IncidenceFormatter::recurrenceString(event));
    }

    html += invitationDescriptionIncidence(event, noHtmlMode);
    html += htmlInvitationDetailsEnd();

    return html;
}

static QString invitationDetailsEvent(const Event::Ptr &event, const Event::Ptr &oldevent,
                                      const ScheduleMessage::Ptr message, bool noHtmlMode,
                                      KDateTime::Spec spec)
{
    if (!oldevent) {
        return invitationDetailsEvent(event, noHtmlMode, spec);
    }

    QString html;

    // Print extra info typically dependent on the iTIP
    if (message->method() == iTIPDeclineCounter) {
        html += QLatin1String("<br>");
        html += invitationNote(QString(),
                               i18n("Please respond again to the original proposal."),
                               QString(), noteColor());
    }

    html += htmlInvitationDetailsBegin(QLatin1String("view-pim-calendar"),
                                       htmlCompare(invitationSummary(event, noHtmlMode),
                                                   invitationSummary(oldevent, noHtmlMode)));

    const QString location = htmlCompare(invitationLocation(event, noHtmlMode),
                                         invitationLocation(oldevent, noHtmlMode));

    KDateTime newDateToUse = event->dtStart();
    KDateTime oldDateToUse = oldevent->dtStart();
    const int exDatesCount = event->recurrence()->exDates().count();
    if (event->recurs() && oldevent->recurs() &&
        exDatesCount == oldevent->recurrence()->exDates().count() + 1 &&
        event->dtStart() == oldevent->dtStart() && event->dtEnd() == oldevent->dtEnd())
    {
        // kolab/issue4735 - When you delete an occurrence of a recurring event, the date
        // of the occurence should be used. This is a bit of a hack because we don't support
        // recurrent-id yet
        // TODO KDAB: We do have recurrenceId already. Investigate
        newDateToUse = KDateTime(event->recurrence()->exDates()[exDatesCount-1], QTime(-1, -1, -1));
        oldDateToUse = newDateToUse;
    }

    html += htmlRow(event->recurs() ? i18n("First event:") : i18n("When:"),
                    IncidenceFormatter::formatStartEnd(newDateToUse, event->dtEnd(), spec),
                    IncidenceFormatter::formatStartEnd(oldDateToUse, oldevent->dtEnd(), spec));

    if (!location.isEmpty()) {
        html += htmlRow(i18n("Where:"), location);
    }

    if (event->recurs() || oldevent->recurs()) {
        QString recurStr, oldrecurStr;
        recurStr = IncidenceFormatter::recurrenceString(event);
        oldrecurStr = IncidenceFormatter::recurrenceString(oldevent);
        html += htmlRow(i18n("Recurrence:"), recurStr, oldrecurStr);
    }

    html += invitationDescriptionIncidence(event, noHtmlMode);
    html += htmlInvitationDetailsEnd();

    return html;
}

static QString invitationDetailsTodo(const Todo::Ptr &todo, bool noHtmlMode,
                                     KDateTime::Spec spec)
{
    // To-do details are formatted into an HTML table
    if (!todo) {
        return QString();
    }

    QString html = htmlInvitationDetailsBegin(QLatin1String("view-pim-tasks"),
                                                            invitationSummary(todo, noHtmlMode));

    const QString location = invitationLocation(todo, noHtmlMode);

    if (!location.isEmpty()) {
        html += htmlRow(i18n("Where:"), location);
    }

    if (todo->hasStartDate() && todo->dtStart().isValid()) {
        // Start and end combine into a single when.
        html += htmlRow(i18n("When:"), IncidenceFormatter::formatStartEnd(todo->dtStart(), todo->dtDue(), spec));
    } else if (todo->hasDueDate() && todo->dtDue().isValid()) {
        // Only a due date.
        html += htmlRow(i18n("Due:"), IncidenceFormatter::dateTimeToString(todo->dtDue(), false, true, spec));
    }

    // Completeness
    if (todo->percentComplete() > 0) {
        html += htmlRow(i18n("Percent Done:"), i18n("%1%", todo->percentComplete()));
    }

    // Invitation Recurrence Row
    if (todo->recurs()) {
        html += htmlRow(i18n("Recurrence:"), recurrenceString(todo));
    }

    html += invitationDescriptionIncidence(todo, noHtmlMode);
    html += htmlInvitationDetailsEnd();

    return html;
}

static QString invitationDetailsTodo(const Todo::Ptr &todo, const Todo::Ptr &oldtodo,
                                     const ScheduleMessage::Ptr message, bool noHtmlMode,
                                     KDateTime::Spec spec)
{
    if (!oldtodo) {
        return invitationDetailsTodo(todo, noHtmlMode, spec);
    }

    QString html;

    // Print extra info typically dependent on the iTIP
    if (message->method() == iTIPDeclineCounter) {
        html += QLatin1String("<br>");
        html += invitationNote(QString(),
                               i18n("Please respond again to the original proposal."),
                               QString(), noteColor());
    }

    html += htmlInvitationDetailsBegin(QLatin1String("view-pim-tasks"),
                                       htmlCompare(invitationSummary(todo, noHtmlMode),
                                                   invitationSummary(oldtodo, noHtmlMode)));
    html += htmlRow(i18n("Where:"),
                    invitationLocation(todo, noHtmlMode),
                    invitationLocation(oldtodo, noHtmlMode));


    if ((todo->hasStartDate() && todo->dtStart().isValid()) || oldtodo->hasStartDate()) {
        html += htmlRow(i18n("When:"),
                        IncidenceFormatter::formatStartEnd(todo->dtStart(), todo->dtDue(), spec),
                        IncidenceFormatter::formatStartEnd(oldtodo->dtStart(), oldtodo->dtDue(), spec));
    } else if ((todo->hasDueDate() && todo->dtDue().isValid()) || oldtodo->hasDueDate()) {
        html += htmlRow(i18n("Due:"),
                        IncidenceFormatter::dateTimeToString(todo->dtDue(), false, false, todo->dtDue().timeSpec()),
                        IncidenceFormatter::dateTimeToString(oldtodo->dtDue(), false, false, oldtodo->dtDue().timeSpec()));
    }

    QString completionStr, oldcompletionStr;
    if (todo->percentComplete() > 0 || oldtodo->percentComplete() > 0) {
        completionStr = i18n("%1%", todo->percentComplete());
        oldcompletionStr = i18n("%1%", oldtodo->percentComplete());
        html += htmlRow(i18n("Percent Done:"), completionStr, oldcompletionStr);
    }

    QString recurStr, oldrecurStr;
    if (todo->recurs() || oldtodo->recurs()) {
        recurStr = recurrenceString(todo);
        oldrecurStr = recurrenceString(oldtodo);
        html += htmlRow(i18n("Recurrence:"), recurStr, oldrecurStr);
    }

    html += invitationDescriptionIncidence(todo, noHtmlMode);
    html += htmlInvitationDetailsEnd();

    return html;
}

static QString invitationDetailsJournal(const Journal::Ptr &journal, bool noHtmlMode,
                                        KDateTime::Spec spec)
{
    if (!journal) {
        return QString();
    }

    QString html = htmlInvitationDetailsBegin(QLatin1String("view-pim-journal"),
                                              invitationSummary(journal, noHtmlMode));

    html += htmlRow(i18n("Date:"), dateToString(journal->dtStart(), false, spec));

    html += invitationDescriptionIncidence(journal, noHtmlMode);
    html += htmlInvitationDetailsEnd();

    return html;
}

static QString invitationDetailsJournal(const Journal::Ptr &journal,
                                        const Journal::Ptr &oldjournal,
                                        bool noHtmlMode, KDateTime::Spec spec)
{
    if (!oldjournal) {
        return invitationDetailsJournal(journal, noHtmlMode, spec);
    }

    QString html = htmlInvitationDetailsBegin(QLatin1String("view-pim-journal"),
                                              htmlCompare(invitationSummary(journal, noHtmlMode),
                                                          invitationSummary(oldjournal, noHtmlMode)));

    html += htmlRow(i18n("Date:"),
                    dateToString(journal->dtStart(), false, spec),
                    dateToString(oldjournal->dtStart(), false, spec));

    html += invitationDescriptionIncidence(journal, noHtmlMode);
    html += htmlInvitationDetailsEnd();

    return html;
}

static QString invitationDetailsFreeBusy(const FreeBusy::Ptr &fb, bool noHtmlMode,
        KDateTime::Spec spec)
{
    Q_UNUSED(noHtmlMode);

    if (!fb) {
        return QString();
    }

    QString html = QLatin1String("<table>");

    html += htmlRow(i18n("Person:"), fb->organizer()->fullName());
    html += htmlRow(i18n("Start date:"), dateToString(fb->dtStart(), true, spec));
    html += htmlRow(i18n("End date:"), dateToString(fb->dtEnd(), true, spec));

    html += QLatin1String("<tr><td colspan=2><hr></td></tr>\n");
    html += QLatin1String("<tr><td colspan=2>Busy periods given in this free/busy object:</td></tr>\n");

    Period::List periods = fb->busyPeriods();
    Period::List::iterator it;
    for (it = periods.begin(); it != periods.end(); ++it) {
        Period per = *it;
        if (per.hasDuration()) {
            int dur = per.duration().asSeconds();
            QString cont;
            if (dur >= 3600) {
                cont += i18ncp("hours part of duration", "1 hour ", "%1 hours ", dur / 3600);
                dur %= 3600;
            }
            if (dur >= 60) {
                cont += i18ncp("minutes part of duration", "1 minute", "%1 minutes ", dur / 60);
                dur %= 60;
            }
            if (dur > 0) {
                cont += i18ncp("seconds part of duration", "1 second", "%1 seconds", dur);
            }
            html += htmlRow(QString(),
                            i18nc("startDate for duration", "%1 for %2",
                                  KGlobal::locale()->formatDateTime(
                                      per.start().dateTime(), KLocale::LongDate),
                                  cont));
        } else {
            QString cont;
            if (per.start().date() == per.end().date()) {
                cont = i18nc("date, fromTime - toTime ", "%1, %2 - %3",
                             KGlobal::locale()->formatDate(per.start().date()),
                             KGlobal::locale()->formatTime(per.start().time()),
                             KGlobal::locale()->formatTime(per.end().time()));
            } else {
                cont = i18nc("fromDateTime - toDateTime", "%1 - %2",
                             KGlobal::locale()->formatDateTime(
                                 per.start().dateTime(), KLocale::LongDate),
                             KGlobal::locale()->formatDateTime(
                                 per.end().dateTime(), KLocale::LongDate));
            }

            html += htmlRow(QString(), cont);
        }
    }

    html += QLatin1String("</table>");
    return html;
}

static QString invitationDetailsFreeBusy(const FreeBusy::Ptr &fb, const FreeBusy::Ptr &oldfb,
        bool noHtmlMode, KDateTime::Spec spec)
{
    Q_UNUSED(oldfb);
    return invitationDetailsFreeBusy(fb, noHtmlMode, spec);
}

static bool replyMeansCounter(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
    return false;
    /**
      see kolab/issue 3665 for an example of when we might use this for something

      bool status = false;
      if ( incidence ) {
        // put code here that looks at the incidence and determines that
        // the reply is meant to be a counter proposal.  We think this happens
        // with Outlook counter proposals, but we aren't sure how yet.
        if ( condition ) {
          status = true;
        }
      }
      return status;
    */
}

static QString invitationHeaderEvent(const Event::Ptr &event,
                                     const Incidence::Ptr &existingIncidence,
                                     ScheduleMessage::Ptr msg, const QString &sender)
{
    if (!msg || !event) {
        return QString();
    }

    switch (msg->method()) {
    case iTIPPublish:
        return i18n("This invitation has been published.");
    case iTIPRequest:
        if (existingIncidence && event->revision() > 0) {
            QString orgStr = organizerName(event, sender);
            if (senderIsOrganizer(event, sender)) {
                return i18n("This invitation has been updated by the organizer <b>%1</b>.", orgStr);
            } else {
                return i18n("This invitation has been updated by <b>%1</b> as a representative of <b>%2</b>.",
                            sender, orgStr);
            }
        }
        if (iamOrganizer(event)) {
            return i18n("I created this invitation.");
        } else {
            QString orgStr = organizerName(event, sender);
            if (senderIsOrganizer(event, sender)) {
                return i18n("You received an invitation from <b>%1</b>.", orgStr);
            } else {
                return i18n("You received an invitation from <b>%1</b> as a representative of <b>%2</b>.",
                            sender, orgStr);
            }
        }
    case iTIPRefresh:
        return i18n("This invitation was refreshed.");
    case iTIPCancel:
        if (iamOrganizer(event)) {
            return i18n("This invitation has been canceled.");
        } else {
            return i18n("The organizer has removed you from the invitation.");
        }
    case iTIPAdd:
        return i18n("Addition to the invitation.");
    case iTIPReply:
    {
        if (replyMeansCounter(event)) {
            return i18n("<b>%1</b> makes this counter proposal.", firstAttendeeName(event, sender));
        }

        Attendee::List attendees = event->attendees();
        if (attendees.count() == 0) {
            kDebug() << "No attendees in the iCal reply!";
            return QString();
        }
        if (attendees.count() != 1) {
            kDebug() << "Warning: attendeecount in the reply should be 1"
                     << "but is" << attendees.count();
        }
        QString attendeeName = firstAttendeeName(event, sender);

        QString delegatorName, dummy;
        Attendee::Ptr attendee = *attendees.begin();
        KPIMUtils::extractEmailAddressAndName(attendee->delegator(), dummy, delegatorName);
        if (delegatorName.isEmpty()) {
            delegatorName = attendee->delegator();
        }

        switch (attendee->status()) {
        case Attendee::NeedsAction:
            return i18n("<b>%1</b> indicates this invitation still needs some action.", attendeeName);
        case Attendee::Accepted:
            if (event->revision() > 0) {
                if (!sender.isEmpty()) {
                    return i18n("This invitation has been updated by attendee <b>%1</b>.", sender);
                } else {
                    return i18n("This invitation has been updated by an attendee.");
                }
            } else {
                if (delegatorName.isEmpty()) {
                    return i18n("<b>%1</b> accepts this invitation.", attendeeName);
                } else {
                    return i18n("<b>%1</b> accepts this invitation on behalf of <b>%2</b>.",
                                attendeeName, delegatorName);
                }
            }
        case Attendee::Tentative:
            if (delegatorName.isEmpty()) {
                return i18n("<b>%1</b> tentatively accepts this invitation.", attendeeName);
            } else {
                return i18n("<b>%1</b> tentatively accepts this invitation on behalf of <b>%2</b>.",
                            attendeeName, delegatorName);
            }
        case Attendee::Declined:
            if (delegatorName.isEmpty()) {
                return i18n("<b>%1</b> declines this invitation.", attendeeName);
            } else {
                return i18n("<b>%1</b> declines this invitation on behalf of <b>%2</b>.",
                            attendeeName, delegatorName);
            }
        case Attendee::Delegated:
        {
            QString delegate, dummy;
            KPIMUtils::extractEmailAddressAndName(attendee->delegate(), dummy, delegate);
            if (delegate.isEmpty()) {
                delegate = attendee->delegate();
            }
            if (!delegate.isEmpty()) {
                return i18n("<b>%1</b> has delegated this invitation to <b>%2</b>.", attendeeName, delegate);
            } else {
                return i18n("<b>%1</b> has delegated this invitation.", attendeeName);
            }
        }
        case Attendee::Completed:
            return i18n("This invitation is now completed.");
        case Attendee::InProcess:
            return i18n("<b>%1</b> is still processing the invitation.", attendeeName);
        case Attendee::None:
            return i18n("Unknown response to this invitation.");
        }
        break;
    }
    case iTIPCounter:
        return i18n("<b>%1</b> sent a counter proposal.",
                    firstAttendeeName(event, i18n("Sender")));

    case iTIPDeclineCounter:
    {
        QString orgStr = organizerName(event, sender);
        if (senderIsOrganizer(event, sender)) {
            return i18n("<b>%1</b> declines your counter proposal.", orgStr);
        } else {
            return i18n("<b>%1</b> declines your counter proposal on behalf of <b>%2</b>.", sender, orgStr);
        }
    }

    case iTIPNoMethod:
        return i18n("Error: Event iTIP message with unknown method");
    }
    kError() << "encountered an iTIP method that we do not support";
    return QString();
}

static QString invitationHeaderTodo(const Todo::Ptr &todo,
                                    const Incidence::Ptr &existingIncidence,
                                    ScheduleMessage::Ptr msg, const QString &sender)
{
    if (!msg || !todo) {
        return QString();
    }

    switch (msg->method()) {
    case iTIPPublish:
        //return i18n("This task has been published");
       return QString::null;
    case iTIPRequest:
        if (existingIncidence && todo->revision() > 0) {
            QString orgStr = organizerName(todo, sender);
            if (senderIsOrganizer(todo, sender)) {
                return i18n("This task has been updated by the organizer <b>%1</b>.", orgStr);
            } else {
                return i18n("This task has been updated by <b>%1</b> as a representative of <b>%2</b>.",
                            sender, orgStr);
            }
        } else {
            if (iamOrganizer(todo)) {
                return QString::null; //return i18n("I created this task");
            } else {
                QString orgStr = organizerName(todo, sender);
                if (senderIsOrganizer(todo, sender)) {
                    return i18n("You have been assigned this task by <b>%1</b>.", orgStr);
                } else {
                    return i18n("You have been assigned this task by <b>%1</b> as a representative of <b>%2</b>.",
                                sender, orgStr);
                }
            }
        }
    case iTIPRefresh:
        return i18n("This task was refreshed.");
    case iTIPCancel:
        if (iamOrganizer(todo)) {
            return i18n("This task was canceled.");
        } else {
            return i18n("The organizer has removed you from this task.");
        }
    case iTIPAdd:
        return i18n("Addition to the task.");
    case iTIPReply:
    {
        if (replyMeansCounter(todo)) {
            return i18n("<b>%1</b> send a counter proposal.", firstAttendeeName(todo, sender));
        }

        Attendee::List attendees = todo->attendees();
        if (attendees.count() == 0) {
            kDebug() << "No attendees in the iCal reply!";
            return QString();
        }
        if (attendees.count() != 1) {
            kDebug() << "Warning: attendeecount in the reply should be 1"
                     << "but is" << attendees.count();
        }
        QString attendeeName = firstAttendeeName(todo, sender);

        QString delegatorName, dummy;
        Attendee::Ptr attendee = *attendees.begin();
        KPIMUtils::extractEmailAddressAndName(attendee->delegate(), dummy, delegatorName);
        if (delegatorName.isEmpty()) {
            delegatorName = attendee->delegator();
        }

        switch (attendee->status()) {
        case Attendee::NeedsAction:
            return i18n("<b>%1</b> indicates this task assignment still needs some action",
                        attendeeName);
        case Attendee::Accepted:
            if (todo->revision() > 0) {
                if (!sender.isEmpty()) {
                    if (todo->isCompleted()) {
                        return i18n("This task has been completed by assignee <b>%1</b>", sender);
                    } else {
                        return i18n("This task has been updated by assignee <b>%1</b>.", sender);
                    }
                } else {
                    if (todo->isCompleted()) {
                        return i18n("This task has been completed by an assignee.");
                    } else {
                        return i18n("This task has been updated by an assignee.");
                    }
                }
            } else {
                if (delegatorName.isEmpty()) {
                    return i18n("<b>%1</b> accepts this task.", attendeeName);
                } else {
                    return i18n("<b>%1</b> accepts this task on behalf of <b>%2</b>.",
                                attendeeName, delegatorName);
                }
            }
        case Attendee::Tentative:
            if (delegatorName.isEmpty()) {
                return i18n("<b>%1</b> tentatively accepts this task.", attendeeName);
            } else {
                return i18n("<b>%1</b> tentatively accepts this task on behalf of <b>%2</b>.",
                            attendeeName, delegatorName);
            }
        case Attendee::Declined:
            if (delegatorName.isEmpty()) {
                return i18n("<b>%1</b> declines this task.", attendeeName);
            } else {
                return i18n("<b>%1</b> declines this task on behalf of <b>%2</b>.",
                            attendeeName, delegatorName);
            }
        case Attendee::Delegated:
        {
            QString delegate, dummy;
            KPIMUtils::extractEmailAddressAndName(attendee->delegate(), dummy, delegate);
            if (delegate.isEmpty()) {
                delegate = attendee->delegate();
            }
            if (!delegate.isEmpty()) {
                return i18n("<b>%1</b> has delegated this request for the task to <b>%2</b>.", attendeeName, delegate);
            } else {
                return i18n("<b>%1</b> has delegated this request for the task.", attendeeName);
            }
        }
        case Attendee::Completed:
            return i18n("The request for this task is now completed.");
        case Attendee::InProcess:
            return i18n("<b>%1</b> is still processing the task.", attendeeName);
        case Attendee::None:
            return i18n("Unknown response to this task.");
        }
        break;
    }
    case iTIPCounter:
        return i18n("<b>%1</b> sent a counter proposal.", firstAttendeeName(todo, sender));

    case iTIPDeclineCounter:
    {
        QString orgStr = organizerName(todo, sender);
        if (senderIsOrganizer(todo, sender)) {
            return i18n("<b>%1</b> declines the counter proposal.", orgStr);
        } else {
            return i18n("<b>%1</b> declines the counter proposal on behalf of <b>%2</b>.", sender, orgStr);
        }
    }

    case iTIPNoMethod:
        return i18n("Error: Task iTIP message with unknown method.");
    }
    kError() << "encountered an iTIP method that we do not support";
    return QString();
}

static QString invitationHeaderJournal(const Journal::Ptr &journal,
                                       ScheduleMessage::Ptr msg)
{
    if (!msg || !journal) {
        return QString();
    }

    switch (msg->method()) {
    case iTIPPublish:
        return i18n("This journal has been published.");
    case iTIPRequest:
        return i18n("You have been assigned this journal.");
    case iTIPRefresh:
        return i18n("This journal was refreshed.");
    case iTIPCancel:
        return i18n("This journal was canceled.");
    case iTIPAdd:
        return i18n("Addition to the journal.");
    case iTIPReply:
    {
        if (replyMeansCounter(journal)) {
            return i18n("Sender sent a count proposal.");
        }

        Attendee::List attendees = journal->attendees();
        if (attendees.count() == 0) {
            kDebug() << "No attendees in the iCal reply!";
            return QString();
        }
        if (attendees.count() != 1) {
            kDebug() << "Warning: attendeecount in the reply should be 1 "
                     << "but is " << attendees.count();
        }
        Attendee::Ptr attendee = *attendees.begin();

        switch (attendee->status()) {
        case Attendee::NeedsAction:
            return i18n("Sender indicates this journal assignment still needs some action.,");
        case Attendee::Accepted:
            return i18n("Sender accepts this journal.");
        case Attendee::Tentative:
            return i18n("Sender tentatively accepts this journal.");
        case Attendee::Declined:
            return i18n("Sender declines this journal.");
        case Attendee::Delegated:
            return i18n("Sender has delegated this request for the journal.");
        case Attendee::Completed:
            return i18n("The request for this journal is now completed.");
        case Attendee::InProcess:
            return i18n("Sender is still processing the invitation.");
        case Attendee::None:
            return i18n("Unknown response to this journal.");
        }
        break;
    }
    case iTIPCounter:
        return i18n("Sender sent a counter proposal.");
    case iTIPDeclineCounter:
        return i18n("Sender declines the counter proposal.");
    case iTIPNoMethod:
        return i18n("Error: Journal iTIP message with unknown method.");
    }
    kError() << "encountered an iTIP method that we do not support";
    return QString();
}

static QString invitationHeaderFreeBusy(const FreeBusy::Ptr &fb,
                                        ScheduleMessage::Ptr msg)
{
    if (!msg || !fb) {
        return QString();
    }

    switch (msg->method()) {
    case iTIPPublish:
        return i18n("This free/busy list has been published.");
    case iTIPRequest:
        return i18n("The free/busy list has been requested.");
    case iTIPRefresh:
        return i18n("This free/busy list was refreshed.");
    case iTIPCancel:
        return i18n("This free/busy list was canceled.");
    case iTIPAdd:
        return i18n("Addition to the free/busy list.");
    case iTIPReply:
        return i18n("Reply to the free/busy list.");
    case iTIPCounter:
        return i18n("Sender sent a counter proposal.");
    case iTIPDeclineCounter:
        return i18n("Sender declines the counter proposal.");
    case iTIPNoMethod:
        return i18n("Error: Free/Busy iTIP message with unknown method.");
    }
    kError() << "encountered an iTIP method that we do not support";
    return QString();
}
//@endcond

static QString invitationAttendeeList(const Incidence::Ptr &incidence)
{
    RAIIIdentityManager raiiHelper;

    QString tmpStr;
    if (!incidence) {
        return tmpStr;
    }
    tmpStr += QLatin1String("<tr>\n<td class=\"leftColumn\">");
    if (incidence->type() == Incidence::TypeTodo) {
        tmpStr += i18n("Assignees:");
    } else {
        tmpStr += i18n("Participants:");
    }
    tmpStr += QLatin1String("</td>\n<td>");

    int count=0;
    Attendee::List attendees = incidence->attendees();
    if (!attendees.isEmpty()) {
        QStringList comments;
        Attendee::List::ConstIterator it;
        for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
            Attendee::Ptr a = *it;
            if (!iamAttendee(a)) {
                count++;
                comments.clear();
                if (attendeeIsOrganizer(incidence, a)) {
                    comments << i18n("organizer");
                }
                if (!a->delegator().isEmpty()) {
                    comments << i18n("delegated by %1", a->delegator());
                }
                if (!a->delegate().isEmpty()) {
                    comments << i18n("delegated to %1", a->delegate());
                }
                tmpStr += invitationPerson(a->email(), a->name(), QString(), comments.join(i18nc("Comment list separator", ", ")));
                tmpStr += QLatin1String("<br>\n");
            }
        }
    }
    if (!count) {
        return QString::null;
    }

    tmpStr += QLatin1String("</td>\n</td>\n");

    return tmpStr;
}

static QString invitationRsvpList(const Incidence::Ptr &incidence, const Attendee::Ptr &sender)
{
    QString tmpStr;
    if (!incidence) {
        return tmpStr;
    }

    tmpStr += QLatin1String("<tr>\n<td class=\"leftColumn\">");
    if (incidence->type() == Incidence::TypeTodo) {
        tmpStr += i18n("Assignees:");
    } else {
        tmpStr += i18n("Participants:");
    }
    tmpStr += QLatin1String("</td>\n<td>");

    int count=0;
    Attendee::List attendees = incidence->attendees();
    if (!attendees.isEmpty()) {
        QStringList comments;
        Attendee::List::ConstIterator it;
        for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
            Attendee::Ptr a = *it;
            if (!attendeeIsOrganizer(incidence, a)) {
                QString statusStr;
                const QString iconPath = rsvpStatusIconPath(a->status());
                tmpStr += QString::fromLatin1("<img src=\"%1\"/>").arg(iconPath);
                if (sender && (a->email() == sender->email())) {
                    // use the attendee taken from the response incidence,
                    // rather than the attendee from the calendar incidence.
                    if (a->status() != sender->status()) {
                        statusStr = QLatin1String("<small>") + i18n("(Status not yet recorded)") + QLatin1String("</small>");
                    }
                    a = sender;
                }
                count++;
                comments.clear();
                if (iamAttendee(a)) {
                    comments << i18n("myself");
                }
                if (!a->delegator().isEmpty()) {
                    comments << i18n("delegated by %1", a->delegator());
                }
                if (!a->delegate().isEmpty()) {
                    comments << i18n("delegated to %1", a->delegate());
                }
                tmpStr += invitationPerson(a->email(), a->name(), QString(), comments.join(i18nc("Comment list separator", ", ")));
                tmpStr += QLatin1String(" ") + statusStr + QLatin1String("<br>\n");
            }
        }
    }
    if (!count) {
        tmpStr += QLatin1String("<i> ") + i18nc("no attendees", "None") + QLatin1String("</i>");
    }

    tmpStr += QLatin1String("</td>\n</tr>\n");

    return tmpStr;
}

static QString invitationAttachments(InvitationFormatterHelper *helper,
                                     const Incidence::Ptr &incidence)
{
    QString tmpStr;
    if (!incidence) {
        return tmpStr;
    }

    if (incidence->type() == Incidence::TypeFreeBusy) {
        // A FreeBusy does not have a valid attachment due to the static-cast from IncidenceBase
        return tmpStr;
    }

    Attachment::List attachments = incidence->attachments();
    QString rightCol;
    if (!attachments.isEmpty()) {
        tmpStr += QLatin1String("<tr>\n<td class=\"leftColumn\">") + i18n("Attachments:") + QLatin1String("</td><td>");

        Attachment::List::ConstIterator it;
        for (it = attachments.constBegin(); it != attachments.constEnd(); ++it) {
            Attachment::Ptr a = *it;
            if (a->label().isEmpty()) {
                continue;
            }
            // Attachment icon
            KMimeType::Ptr mimeType = KMimeType::mimeType(a->mimeType());
            const QString iconStr = (mimeType ?
                                     mimeType->iconName(a->uri()) :
                                     QLatin1String("application-octet-stream"));
            const QString iconPath = KIconLoader::global()->iconPath(iconStr, KIconLoader::Small);
            if (!iconPath.isEmpty()) {
                rightCol += QString::fromLatin1("<img valign=\"top\" src=\"%1\">").arg(iconPath);
            }
            rightCol += helper->makeLink(QLatin1String("ATTACH:") + QLatin1String(a->label().toUtf8().toBase64()), a->label());
            rightCol += QLatin1String("<br>");
        }
    }

    return rightCol.isEmpty() ? QString::null : QString(tmpStr + rightCol);
}

//@cond PRIVATE
class KCalUtils::IncidenceFormatter::ScheduleMessageVisitor : public Visitor
{
public:
    ScheduleMessageVisitor() : mMessage(0) {
        mResult = QLatin1String("");
    }
    bool act(const IncidenceBase::Ptr &incidence,
             const Incidence::Ptr &existingIncidence,
             ScheduleMessage::Ptr msg, const QString &sender)
    {
        mExistingIncidence = existingIncidence;
        mMessage = msg;
        mSender = sender;
        return incidence->accept(*this, incidence);
    }
    QString result() const {
        return mResult;
    }

protected:
    QString mResult;
    Incidence::Ptr mExistingIncidence;
    ScheduleMessage::Ptr mMessage;
    QString mSender;
};

class KCalUtils::IncidenceFormatter::InvitationHeaderVisitor :
    public IncidenceFormatter::ScheduleMessageVisitor
{
protected:
    bool visit(Event::Ptr event)
    {
        mResult = invitationHeaderEvent(event, mExistingIncidence, mMessage, mSender);
        return !mResult.isEmpty();
    }
    bool visit(Todo::Ptr todo)
    {
        mResult = invitationHeaderTodo(todo, mExistingIncidence, mMessage, mSender);
        return !mResult.isEmpty();
    }
    bool visit(Journal::Ptr journal)
    {
        mResult = invitationHeaderJournal(journal, mMessage);
        return !mResult.isEmpty();
    }
    bool visit(FreeBusy::Ptr fb)
    {
        mResult = invitationHeaderFreeBusy(fb, mMessage);
        return !mResult.isEmpty();
    }
};

class KCalUtils::IncidenceFormatter::InvitationBodyVisitor
    : public IncidenceFormatter::ScheduleMessageVisitor
{
public:
    InvitationBodyVisitor(bool noHtmlMode, KDateTime::Spec spec)
        : ScheduleMessageVisitor(), mNoHtmlMode(noHtmlMode), mSpec(spec) {}

protected:
    bool visit(Event::Ptr event)
    {
        Event::Ptr oldevent = mExistingIncidence.dynamicCast<Event>();
        mResult = invitationDetailsEvent(event, oldevent, mMessage, mNoHtmlMode, mSpec);
        return !mResult.isEmpty();
    }
    bool visit(Todo::Ptr todo)
    {
        Todo::Ptr oldtodo = mExistingIncidence.dynamicCast<Todo>();
        mResult = invitationDetailsTodo(todo, oldtodo, mMessage, mNoHtmlMode, mSpec);
        return !mResult.isEmpty();
    }
    bool visit(Journal::Ptr journal)
    {
        Journal::Ptr oldjournal = mExistingIncidence.dynamicCast<Journal>();
        mResult = invitationDetailsJournal(journal, oldjournal, mNoHtmlMode, mSpec);
        return !mResult.isEmpty();
    }
    bool visit(FreeBusy::Ptr fb)
    {
        mResult = invitationDetailsFreeBusy(fb, FreeBusy::Ptr(), mNoHtmlMode, mSpec);
        return !mResult.isEmpty();
    }

private:
    bool mNoHtmlMode;
    KDateTime::Spec mSpec;
};
//@endcond

InvitationFormatterHelper::InvitationFormatterHelper()
    : d(0)
{
}

InvitationFormatterHelper::~InvitationFormatterHelper()
{
}

QString InvitationFormatterHelper::generateLinkURL(const QString &id)
{
    return id;
}

QString InvitationFormatterHelper::makeLink(const QString &id, const QString &text)
{
    if (!id.startsWith(QLatin1String("ATTACH:"))) {
        QString res = QString::fromLatin1("<a href=\"%1\"><font size=\"-1\"><b>%2</b></font></a>").
                      arg(generateLinkURL(id), text);
        return res;
    } else {
        // draw the attachment links in non-bold face
        QString res = QString::fromLatin1("<a href=\"%1\">%2</a>").
                      arg(generateLinkURL(id), text);
        return res;
    }
}

// Check if the given incidence is likely one that we own instead one from
// a shared calendar (Kolab-specific)
static bool incidenceOwnedByMe(const Calendar::Ptr &calendar,
                               const Incidence::Ptr &incidence)
{
    Q_UNUSED(calendar);
    Q_UNUSED(incidence);
    return true;
}

static QString inviteButton(InvitationFormatterHelper *helper,
                            const QString &id, const QString &text,
                            const QString &iconName)
{
    QString html;
    if (!helper) {
        return html;
    }

    const QString iconPath = KIconLoader::global()->iconPath(iconName, KIconLoader::Toolbar);
    html = QString::fromLatin1("<a class=\"button\" href=\"%1\"><img src=\"%2\"/>%3</a>  ")
              .arg(helper->generateLinkURL(id), iconPath, text);
    return html;
}

static QString inviteLink(InvitationFormatterHelper *helper,
                          const QString &id, const QString &text)
{
    QString html;

    if (helper && !id.isEmpty()) {
        html += helper->makeLink(id, text);
    } else {
        html += text;
    }
    return html;
}

static QString responseButtons(const Incidence::Ptr &incidence,
                               bool rsvpReq, bool rsvpRec,
                               InvitationFormatterHelper *helper,
                               const Incidence::Ptr &existingInc = Incidence::Ptr())
{
    QString html;
    if (!helper) {
        return html;
    }

    bool hideAccept = false,
         hideTentative = false,
         hideDecline = false;

    if (existingInc) {
        Attendee::Ptr ea = findMyAttendee(existingInc);
        if (ea) {
            // If this is an update of an already accepted incidence
            // to not show the buttons that confirm the status.
            hideAccept = ea->status() == Attendee::Accepted;
            hideDecline = ea->status() == Attendee::Declined;
            hideTentative = ea->status() == Attendee::Tentative;
        }
    }

    if (!rsvpReq && (incidence && incidence->revision() == 0)) {
        // Record only
        html += inviteButton(helper, QLatin1String("record"), i18n("Record"), QLatin1String("dialog-ok"));

        // Move to trash
        html += inviteButton(helper, QLatin1String("delete"), i18n("Move to Trash"), QLatin1String("edittrash"));

    } else {

        // Accept
        if (!hideAccept) {
            html += inviteButton(helper, QLatin1String("accept"),
                                 i18nc("accept invitation", "Accept"),
                                 QLatin1String("dialog-ok-apply"));
        }

        // Tentative
        if (!hideTentative) {
            html += inviteButton(helper, QLatin1String("accept_conditionally"),
                                i18nc("Accept invitation conditionally", "Provisorily"),
                                 QLatin1String("dialog-ok"));
        }

        // Decline
        if (!hideDecline) {
            html += inviteButton(helper, QLatin1String("decline"),
                                 i18nc("decline invitation", "Decline"),
                                 QLatin1String("dialog-cancel"));
        }

        // Counter proposal
        html += inviteButton(helper, QLatin1String("counter"),
                             i18nc("invitation counter proposal", "Counter proposal ..."),
                             QLatin1String("edit-undo"));
    }

    if (!rsvpRec || (incidence && incidence->revision() > 0)) {
        // Delegate
        html += inviteButton(helper, QLatin1String("delegate"),
                             i18nc("delegate inviation to another", "Delegate ..."),
                             QLatin1String("mail-forward"));
    }
    return html;
}

static QString counterButtons(const Incidence::Ptr &incidence,
                              InvitationFormatterHelper *helper)
{
    QString html;
    if (!helper) {
        return html;
    }

    // Accept proposal
    html += inviteButton(helper, QLatin1String("accept_counter"), i18n("Accept"),
                         QLatin1String("dialog-ok-apply"));

    // Decline proposal
    html += inviteButton(helper, QLatin1String("decline_counter"), i18n("Decline"),
                         QLatin1String("dialog-cancel"));

    // Check calendar
    if (incidence) {
        if (incidence->type() == Incidence::TypeTodo) {
            html += inviteButton(helper, QLatin1String("check_calendar"),
                                 i18n("Check my task list"),
                                 QLatin1String("go-jump-today"));
        } else {
            html += inviteButton(helper, QLatin1String("check_calendar"),
                                 i18n("Check my calendar"),
                                 QLatin1String("go-jump-today"));
        }
    }
    return html;
}

static QString recordButtons(const Incidence::Ptr &incidence,
                             InvitationFormatterHelper *helper)
{
    QString html;
    if (!helper) {
        return html;
    }

    if (incidence) {
        if (incidence->type() == Incidence::TypeTodo) {
            html += inviteButton(helper, QLatin1String("reply"),
                                 i18n("Record invitation in my task list"),
                                 QLatin1String("dialog-ok"));
        } else {
            html += inviteButton(helper, QLatin1String("reply"),
                                 i18n("Record invitation in my calendar"),
                                 QLatin1String("dialog-ok"));
        }
    }
    return html;
}

static QString recordResponseButtons(const Incidence::Ptr &incidence,
                                     InvitationFormatterHelper *helper)
{
    QString html;
    if (!helper) {
        return html;
    }

    if (incidence) {
        if (incidence->type() == Incidence::TypeTodo) {
            html += inviteButton(helper, QLatin1String("reply"),
                                 i18n("Record response in my task list"),
                                 QLatin1String("dialog-ok"));
        } else {
            html += inviteButton(helper, QLatin1String("reply"),
                                i18n("Record response in my calendar"),
                                 QLatin1String("dialog-ok"));
        }
    }
    return html;
}

static QString cancelButtons(const Incidence::Ptr &incidence,
                             InvitationFormatterHelper *helper)
{
    QString html;
    if (!helper) {
        return html;
    }

    // Remove invitation
    if (incidence) {
        if (incidence->type() == Incidence::TypeTodo) {
            html += inviteButton(helper, QLatin1String("cancel"),
                                 i18n("Remove invitation from my task list"),
                                 QLatin1String("dialog-cancel"));
        } else {
            html += inviteButton(helper, QLatin1String("cancel"),
                                 i18n("Remove invitation from my calendar"),
                                 QLatin1String("dialog-cancel"));

        }
    }
    return html;
}

Calendar::Ptr InvitationFormatterHelper::calendar() const
{
    return Calendar::Ptr();
}

static QString formatICalInvitationHelper(QString invitation,
        const MemoryCalendar::Ptr &mCalendar,
        InvitationFormatterHelper *helper,
        bool noHtmlMode,
        KDateTime::Spec spec,
        const QString &sender)
{
    if (invitation.isEmpty()) {
        return QString();
    }

    ICalFormat format;
    // parseScheduleMessage takes the tz from the calendar,
    // no need to set it manually here for the format!
    ScheduleMessage::Ptr msg = format.parseScheduleMessage(mCalendar, invitation);

    if (!msg) {
        kDebug() << "Failed to parse the scheduling message";
        Q_ASSERT(format.exception());
        kDebug() << Stringify::errorMessage(*format.exception());   //krazy:exclude=kdebug
        return QString();
    }

    IncidenceBase::Ptr incBase = msg->event();

    incBase->shiftTimes(mCalendar->timeSpec(), KDateTime::Spec::LocalZone());

    // Determine if this incidence is in my calendar (and owned by me)
    Incidence::Ptr existingIncidence;
    if (incBase && helper->calendar()) {
        existingIncidence = helper->calendar()->incidence(incBase->uid());

        if (!incidenceOwnedByMe(helper->calendar(), existingIncidence)) {
            existingIncidence.clear();
        }
        if (!existingIncidence) {
            const Incidence::List list = helper->calendar()->incidences();
            for (Incidence::List::ConstIterator it = list.begin(), end = list.end(); it != end; ++it) {
                if ((*it)->schedulingID() == incBase->uid() &&
                        incidenceOwnedByMe(helper->calendar(), *it)) {
                    existingIncidence = *it;
                    break;
                }
            }
        }
    }

    Incidence::Ptr inc = incBase.staticCast<Incidence>();  // the incidence in the invitation email

    // If the IncidenceBase is a FreeBusy, then we cannot access the revision number in
    // the static-casted Incidence; so for sake of nothing better use 0 as the revision.
    int incRevision = 0;
    if (inc && inc->type() != Incidence::TypeFreeBusy) {
        incRevision = inc->revision();
    }

    // determine if I am the organizer for this invitation
    bool myInc = iamOrganizer(inc);

    // determine if the invitation response has already been recorded
    bool rsvpRec = false;
    Attendee::Ptr ea;
    if (!myInc) {
        Incidence::Ptr rsvpIncidence = existingIncidence;
        if (!rsvpIncidence && inc && incRevision > 0) {
            rsvpIncidence = inc;
        }
        if (rsvpIncidence) {
            ea = findMyAttendee(rsvpIncidence);
        }
        if (ea &&
                (ea->status() == Attendee::Accepted ||
                 ea->status() == Attendee::Declined ||
                 ea->status() == Attendee::Tentative)) {
            rsvpRec = true;
        }
    }

    // determine invitation role
    QString role;
    bool isDelegated = false;
    Attendee::Ptr a = findMyAttendee(inc);
    if (!a && inc) {
        if (!inc->attendees().isEmpty()) {
            a = inc->attendees().first();
        }
    }
    if (a) {
        isDelegated = (a->status() == Attendee::Delegated);
        role = Stringify::attendeeRole(a->role());
    }

    // determine if RSVP needed, not-needed, or response already recorded
    bool rsvpReq = rsvpRequested(inc);

    // Now make the body
    QString html;
    html += QLatin1String("<div id=\"invitation\">\n");

    IncidenceFormatter::InvitationHeaderVisitor headerVisitor;
    // The InvitationHeaderVisitor returns false if the incidence is somehow invalid, or not handled
    if (!headerVisitor.act(inc, existingIncidence, msg, sender)) {
        return QString();
    }

    QString headerResult = headerVisitor.result();
    html += QLatin1String("<p>"); // Header and event info paragraph
    if (!headerResult.isEmpty()) {
        html += headerResult;
    }

    // First make the text of the message
    QString eventInfo;
    if (!myInc && a) {
        if (rsvpRec && inc) {
            if (incRevision == 0) {
                eventInfo = i18n("Your <b>%1</b> response has been recorded.",
                                 Stringify::attendeeStatus(ea->status()));
            } else {
                eventInfo = i18n("Your status for this invitation is <b>%1</b>.",
                                 Stringify::attendeeStatus(ea->status()));
            }
            rsvpReq = false;
        } else if (msg->method() == iTIPCancel) {
            eventInfo = i18n("This invitation was canceled.");
        } else if (msg->method() == iTIPAdd) {
            eventInfo = i18n("This invitation was accepted.");
        } else if (msg->method() == iTIPDeclineCounter) {
            rsvpReq = true;
            //eventInfo = rsvpRequestedStr(rsvpReq, role);
        } else {
            if (!isDelegated) {
                //eventInfo = rsvpRequestedStr(rsvpReq, role);
            } else {
                eventInfo = i18n("Awaiting delegation response.");
            }
        }
    }
    if (!eventInfo.isEmpty()) {
        html += QString::fromLatin1("<br/><i>%1</i>").arg(eventInfo);
    }

    // Print if the organizer gave you a preset status
    if (!myInc) {
        if (inc && incRevision == 0) {
            QString statStr = myStatusStr(inc);
            if (!statStr.isEmpty()) {
                html += QLatin1String("<br>");
                html += QLatin1String("<i>") + statStr + QLatin1String("</i>");
            }
        }
    }

    html += QLatin1String("</p>");

    // Add groupware links

    switch (msg->method()) {
    case iTIPPublish:
    case iTIPRequest:
    case iTIPRefresh:
    case iTIPAdd:
    {
        if (inc && incRevision > 0 && (existingIncidence || !helper->calendar())) {
            html += recordButtons(inc, helper);
        }

        if (!myInc) {
            if (a) {
                html += responseButtons(inc, rsvpReq, rsvpRec, helper);
            } else {
                html += responseButtons(inc, false, false, helper);
            }
        }
        break;
    }

    case iTIPCancel:
        html += cancelButtons(inc, helper);
        break;

    case iTIPReply:
    {
        // Record invitation response
        Attendee::Ptr a;
        Attendee::Ptr ea;
        if (inc) {
            // First, determine if this reply is really a counter in disguise.
            if (replyMeansCounter(inc)) {
                html += counterButtons(inc, helper);
                break;
            }

            // Next, maybe this is a declined reply that was delegated from me?
            // find first attendee who is delegated-from me
            // look a their PARTSTAT response, if the response is declined,
            // then we need to start over which means putting all the action
            // buttons and NOT putting on the [Record response..] button
            a = findDelegatedFromMyAttendee(inc);
            if (a) {
                if (a->status() != Attendee::Accepted ||
                        a->status() != Attendee::Tentative) {
                    html += responseButtons(inc, rsvpReq, rsvpRec, helper);
                    break;
                }
            }

            // Finally, simply allow a Record of the reply
            if (!inc->attendees().isEmpty()) {
                a = inc->attendees().first();
            }
            if (a && helper->calendar()) {
                ea = findAttendee(existingIncidence, a->email());
            }
        }
        if (ea && (ea->status() != Attendee::NeedsAction) && (ea->status() == a->status())) {
            const QString tStr = i18n("The <b>%1</b> response has been recorded",
                                      Stringify::attendeeStatus(ea->status()));
            html += inviteButton(helper, QString(), htmlAddTag(QLatin1String("i"), tStr));
        } else {
            if (inc) {
                html += recordResponseButtons(inc, helper);
            }
        }
        break;
    }

    case iTIPCounter:
        // Counter proposal
        html += counterButtons(inc, helper);
        break;

    case iTIPDeclineCounter:
        html += responseButtons(inc, rsvpReq, rsvpRec, helper);
        break;

    case iTIPNoMethod:
        break;
    }

    html += invitationCommentsIncidence(inc, noHtmlMode);

    html += QLatin1String("\n<hr>\n<table border=\"0\">");

    InvitationBodyVisitor bodyVisitor(noHtmlMode, spec);
    bool bodyOk;
    if (msg->method() == iTIPRequest || msg->method() == iTIPReply ||
        msg->method() == iTIPDeclineCounter ) {
        if (inc && existingIncidence &&
            inc->lastModified() < existingIncidence->lastModified()) {
            bodyOk = bodyVisitor.act(existingIncidence, inc, msg, sender);
        } else {
            bodyOk = bodyVisitor.act(inc, existingIncidence, msg, sender);
        }
    } else {
        bodyOk = bodyVisitor.act(inc, existingIncidence, msg, sender);
    }
    if (bodyOk) {
        html += bodyVisitor.result();
    } else {
        return QString::null;
    }

    // Add the attendee list
    if (myInc) {
        html += invitationRsvpList(existingIncidence, a);
    } else {
        html += invitationAttendeeList(inc);
    }

    // Add the attachment list
    html += invitationAttachments(helper, inc);

    html += QLatin1String("\n</table>");
    html += QLatin1String("\n<hr/>\n");

    // Add events on the same day
    html += displayViewFormatEventsOnSameDays(helper, inc.dynamicCast<KCalCore::Event>(),
                                              spec, noHtmlMode);

    html += QLatin1String("</div>");

    return html;
}
//@endcond

QString IncidenceFormatter::formatICalInvitation(QString invitation,
        const MemoryCalendar::Ptr &calendar,
        InvitationFormatterHelper *helper)
{
    return formatICalInvitationHelper(invitation, calendar, helper, false,
                                      KSystemTimeZones::local(), QString());
}

QString IncidenceFormatter::formatICalInvitationNoHtml(const QString &invitation,
        const MemoryCalendar::Ptr &calendar,
        InvitationFormatterHelper *helper,
        const QString &sender)
{
    return formatICalInvitationHelper(invitation, calendar, helper, true,
                                      KSystemTimeZones::local(), sender);
}

/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

//@cond PRIVATE
class KCalUtils::IncidenceFormatter::ToolTipVisitor : public Visitor
{
public:
    ToolTipVisitor()
        : mRichText(true), mSpec(KDateTime::Spec()), mResult(QLatin1String("")) {}

    bool act(const MemoryCalendar::Ptr &calendar,
             const IncidenceBase::Ptr &incidence,
             const QDate &date=QDate(), bool richText=true,
             KDateTime::Spec spec=KDateTime::Spec())
    {
        mCalendar = calendar;
        mLocation.clear();
        mDate = date;
        mRichText = richText;
        mSpec = spec;
        mResult = QLatin1String("");
        return incidence ? incidence->accept(*this, incidence) : false;
    }

    bool act(const QString &location, const IncidenceBase::Ptr &incidence,
             const QDate &date=QDate(), bool richText=true,
             KDateTime::Spec spec=KDateTime::Spec())
    {
        mLocation = location;
        mDate = date;
        mRichText = richText;
        mSpec = spec;
        mResult = QLatin1String("");
        return incidence ? incidence->accept(*this, incidence) : false;
    }

    QString result() const {
        return mResult;
    }

protected:
    bool visit(Event::Ptr event);
    bool visit(Todo::Ptr todo);
    bool visit(Journal::Ptr journal);
    bool visit(FreeBusy::Ptr fb);

    QString dateRangeText(const Event::Ptr &event, const QDate &date);
    QString dateRangeText(const Todo::Ptr &todo, const QDate &date);
    QString dateRangeText(const Journal::Ptr &journal);
    QString dateRangeText(const FreeBusy::Ptr &fb);

    QString generateToolTip(const Incidence::Ptr &incidence, QString dtRangeText);

protected:
    MemoryCalendar::Ptr mCalendar;
    QString mLocation;
    QDate mDate;
    bool mRichText;
    KDateTime::Spec mSpec;
    QString mResult;
};

QString IncidenceFormatter::ToolTipVisitor::dateRangeText(const Event::Ptr &event,
        const QDate &date)
{
    //FIXME: support mRichText==false
    QString ret;
    QString tmp;

    KDateTime startDt = event->dtStart();
    KDateTime endDt = event->dtEnd();
    if (event->recurs()) {
        if (date.isValid()) {
            KDateTime kdt(date, QTime(0, 0, 0), KSystemTimeZones::local());
            int diffDays = startDt.daysTo(kdt);
            kdt = kdt.addSecs(-1);
            startDt.setDate(event->recurrence()->getNextDateTime(kdt).date());
            if (event->hasEndDate()) {
                endDt = endDt.addDays(diffDays);
                if (startDt > endDt) {
                    startDt.setDate(event->recurrence()->getPreviousDateTime(kdt).date());
                    endDt = startDt.addDays(event->dtStart().daysTo(event->dtEnd()));
                }
            }
        }
    }

    if (event->isMultiDay()) {
        tmp = dateToString(startDt, true, mSpec);
        ret += QLatin1String("<br>") + i18nc("Event start", "<i>From:</i> %1", tmp);

        tmp = dateToString(endDt, true, mSpec);
        ret += QLatin1String("<br>") + i18nc("Event end","<i>To:</i> %1", tmp);

    } else {

        ret += QLatin1String("<br>") +
               i18n("<i>Date:</i> %1", dateToString(startDt, false, mSpec));
        if (!event->allDay()) {
            const QString dtStartTime = timeToString(startDt, true, mSpec);
            const QString dtEndTime = timeToString(endDt, true, mSpec);
            if (dtStartTime == dtEndTime) {
                // to prevent 'Time: 17:00 - 17:00'
                tmp = QLatin1String("<br>") +
                      i18nc("time for event", "<i>Time:</i> %1",
                            dtStartTime);
            } else {
                tmp = QLatin1String("<br>") +
                      i18nc("time range for event",
                            "<i>Time:</i> %1 - %2",
                            dtStartTime, dtEndTime);
            }
            ret += tmp;
        }
    }
    return ret.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText(const Todo::Ptr &todo,
        const QDate &date)
{
    //FIXME: support mRichText==false
    QString ret;
    if (todo->hasStartDate()) {
        KDateTime startDt = todo->dtStart();
        if (todo->recurs() && date.isValid()) {
            startDt.setDate(date);
        }
        ret += QLatin1String("<br>") +
               i18n("<i>Start:</i> %1", dateToString(startDt, false, mSpec));
    }

    if (todo->hasDueDate()) {
        KDateTime dueDt = todo->dtDue();
        if (todo->recurs() && date.isValid()) {
            KDateTime kdt(date, QTime(0, 0, 0), KSystemTimeZones::local());
            kdt = kdt.addSecs(-1);
            dueDt.setDate(todo->recurrence()->getNextDateTime(kdt).date());
        }
        ret += QLatin1String("<br>") +
               i18n("<i>Due:</i> %1",
                    dateTimeToString(dueDt, todo->allDay(), false, mSpec));
    }

    // Print priority and completed info here, for lack of a better place

    if (todo->priority() > 0) {
        ret += QLatin1String("<br>");
        ret += QLatin1String("<i>") + i18n("Priority:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        ret += QString::number(todo->priority());
    }

    ret += QLatin1String("<br>");
    if (todo->isCompleted()) {
        ret += QLatin1String("<i>") + i18nc("Completed: date", "Completed:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        ret += Stringify::todoCompletedDateTime(todo).replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
    } else {
        ret += QLatin1String("<i>")+ i18n("Percent Done:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        ret += i18n("%1%", todo->percentComplete());
    }

    return ret.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText(const Journal::Ptr &journal)
{
    //FIXME: support mRichText==false
    QString ret;
    if (journal->dtStart().isValid()) {
        ret += QLatin1String("<br>") +
               i18n("<i>Date:</i> %1", dateToString(journal->dtStart(), false, mSpec));
    }
    return ret.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
}

QString IncidenceFormatter::ToolTipVisitor::dateRangeText(const FreeBusy::Ptr &fb)
{
    //FIXME: support mRichText==false
    QString ret;
    ret = QLatin1String("<br>") +
          i18n("<i>Period start:</i> %1",
               KGlobal::locale()->formatDateTime(fb->dtStart().dateTime()));
    ret += QLatin1String("<br>") +
           i18n("<i>Period start:</i> %1",
                KGlobal::locale()->formatDateTime(fb->dtEnd().dateTime()));
    return ret.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
}

bool IncidenceFormatter::ToolTipVisitor::visit(Event::Ptr event)
{
    mResult = generateToolTip(event, dateRangeText(event, mDate));
    return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit(Todo::Ptr todo)
{
    mResult = generateToolTip(todo, dateRangeText(todo, mDate));
    return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit(Journal::Ptr journal)
{
    mResult = generateToolTip(journal, dateRangeText(journal));
    return !mResult.isEmpty();
}

bool IncidenceFormatter::ToolTipVisitor::visit(FreeBusy::Ptr fb)
{
    //FIXME: support mRichText==false
    mResult = QLatin1String("<qt><b>") +
              i18n("Free/Busy information for %1", fb->organizer()->fullName()) +
              QLatin1String("</b>");
    mResult += dateRangeText(fb);
    mResult += QLatin1String("</qt>");
    return !mResult.isEmpty();
}

static QString tooltipPerson(const QString &email, const QString &name, Attendee::PartStat status)
{
    // Search for a new print name, if needed.
    const QString printName = searchName(email, name);

    // Get the icon corresponding to the attendee participation status.
    const QString iconPath = rsvpStatusIconPath(status);

    // Make the return string.
    QString personString;
    if (!iconPath.isEmpty()) {
        personString += QLatin1String("<img valign=\"top\" src=\"") + iconPath + QLatin1String("\">") + QLatin1String("&nbsp;");
    }
    if (status != Attendee::None) {
        personString += i18nc("attendee name (attendee status)", "%1 (%2)",
                              printName.isEmpty() ? email : printName,
                              Stringify::attendeeStatus(status));
    } else {
        personString += i18n("%1", printName.isEmpty() ? email : printName);
    }
    return personString;
}

static QString tooltipFormatOrganizer(const QString &email, const QString &name)
{
    // Search for a new print name, if needed
    const QString printName = searchName(email, name);

    // Get the icon for organizer
    const QString iconPath =
        KIconLoader::global()->iconPath(QLatin1String("meeting-organizer"), KIconLoader::Small);

    // Make the return string.
    QString personString;
    personString += QLatin1String("<img valign=\"top\" src=\"") + iconPath + QLatin1String("\">") + QLatin1String("&nbsp;");
    personString += (printName.isEmpty() ? email : printName);
    return personString;
}

static QString tooltipFormatAttendeeRoleList(const Incidence::Ptr &incidence,
        Attendee::Role role, bool showStatus)
{
    int maxNumAtts = 8; // maximum number of people to print per attendee role
    const QString etc = i18nc("elipsis", "...");

    int i = 0;
    QString tmpStr;
    Attendee::List::ConstIterator it;
    Attendee::List attendees = incidence->attendees();

    for (it = attendees.constBegin(); it != attendees.constEnd(); ++it) {
        Attendee::Ptr a = *it;
        if (a->role() != role) {
            // skip not this role
            continue;
        }
        if (attendeeIsOrganizer(incidence, a)) {
            // skip attendee that is also the organizer
            continue;
        }
        if (i == maxNumAtts) {
            tmpStr += QLatin1String("&nbsp;&nbsp;") + etc;
            break;
        }
        tmpStr += QLatin1String("&nbsp;&nbsp;") + tooltipPerson(a->email(), a->name(),
                  showStatus ? a->status() : Attendee::None);
        if (!a->delegator().isEmpty()) {
            tmpStr += i18n(" (delegated by %1)", a->delegator());
        }
        if (!a->delegate().isEmpty()) {
            tmpStr += i18n(" (delegated to %1)", a->delegate());
        }
        tmpStr += QLatin1String("<br>");
        i++;
    }
    if (tmpStr.endsWith(QLatin1String("<br>"))) {
        tmpStr.chop(4);
    }
    return tmpStr;
}

static QString tooltipFormatAttendees(const Calendar::Ptr &calendar,
                                      const Incidence::Ptr &incidence)
{
    QString tmpStr, str;

    // Add organizer link
    int attendeeCount = incidence->attendees().count();
    if (attendeeCount > 1 ||
            (attendeeCount == 1 &&
             !attendeeIsOrganizer(incidence, incidence->attendees().first()))) {
        tmpStr += QLatin1String("<i>") + i18n("Organizer:") + QLatin1String("</i>") + QLatin1String("<br>");
        tmpStr += QLatin1String("&nbsp;&nbsp;") + tooltipFormatOrganizer(incidence->organizer()->email(),
                  incidence->organizer()->name());
    }

    // Show the attendee status if the incidence's organizer owns the resource calendar,
    // which means they are running the show and have all the up-to-date response info.
    const bool showStatus = attendeeCount > 0 && incOrganizerOwnsCalendar(calendar, incidence);

    // Add "chair"
    str = tooltipFormatAttendeeRoleList(incidence, Attendee::Chair, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<br><i>") + i18n("Chair:") + QLatin1String("</i>") + QLatin1String("<br>");
        tmpStr += str;
    }

    // Add required participants
    str = tooltipFormatAttendeeRoleList(incidence, Attendee::ReqParticipant, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<br><i>") + i18n("Required Participants:") + QLatin1String("</i>") + QLatin1String("<br>");
        tmpStr += str;
    }

    // Add optional participants
    str = tooltipFormatAttendeeRoleList(incidence, Attendee::OptParticipant, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<br><i>") + i18n("Optional Participants:") + QLatin1String("</i>") + QLatin1String("<br>");
        tmpStr += str;
    }

    // Add observers
    str = tooltipFormatAttendeeRoleList(incidence, Attendee::NonParticipant, showStatus);
    if (!str.isEmpty()) {
        tmpStr += QLatin1String("<br><i>") + i18n("Observers:") + QLatin1String("</i>") + QLatin1String("<br>");
        tmpStr += str;
    }

    return tmpStr;
}

QString IncidenceFormatter::ToolTipVisitor::generateToolTip(const Incidence::Ptr &incidence,
        QString dtRangeText)
{
    int maxDescLen = 120; // maximum description chars to print (before elipsis)

    //FIXME: support mRichText==false
    if (!incidence) {
        return QString();
    }

    QString tmp = QLatin1String("<qt>");

    // header
    tmp += QLatin1String("<b>") + incidence->richSummary() + QLatin1String("</b>");
    tmp += QLatin1String("<hr>");

    QString calStr = mLocation;
    if (mCalendar) {
        calStr = resourceString(mCalendar, incidence);
    }
    if (!calStr.isEmpty()) {
        tmp += QLatin1String("<i>") + i18n("Calendar:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        tmp += calStr;
    }

    tmp += dtRangeText;

    if (!incidence->location().isEmpty()) {
        tmp += QLatin1String("<br>");
        tmp += QLatin1String("<i>") + i18n("Location:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        tmp += incidence->richLocation();
    }

    QString durStr = durationString(incidence);
    if (!durStr.isEmpty()) {
        tmp += QLatin1String("<br>");
        tmp += QLatin1String("<i>") + i18n("Duration:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        tmp += durStr;
    }

    if (incidence->recurs()) {
        tmp += QLatin1String("<br>");
        tmp += QLatin1String("<i>") + i18n("Recurrence:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        tmp += recurrenceString(incidence);
    }

    if (incidence->hasRecurrenceId()) {
        tmp += QLatin1String("<br>");
        tmp += QLatin1String("<i>") + i18n("Recurrence:") + QLatin1String("</i>") + QLatin1String("&nbsp;");
        tmp += i18n("Exception");
    }

    if (!incidence->description().isEmpty()) {
        QString desc(incidence->description());
        if (!incidence->descriptionIsRich()) {
            if (desc.length() > maxDescLen) {
                desc = desc.left(maxDescLen) + i18nc("elipsis", "...");
            }
            desc = Qt::escape(desc).replace(QLatin1Char('\n'), QLatin1String("<br>"));
        } else {
            // TODO: truncate the description when it's rich text
        }
        tmp += QLatin1String("<hr>");
        tmp += QLatin1String("<i>") + i18n("Description:") + QLatin1String("</i>") + QLatin1String("<br>");
        tmp += desc;
        tmp += QLatin1String("<hr>");
    }

    int reminderCount = incidence->alarms().count();
    if (reminderCount > 0 && incidence->hasEnabledAlarms()) {
        tmp += QLatin1String("<br>");
        tmp += QLatin1String("<i>") + i18np("Reminder:", "Reminders:", reminderCount) + QLatin1String("</i>") + QLatin1String("&nbsp;");
        tmp += reminderStringList(incidence).join(QLatin1String(", "));
    }

    tmp += QLatin1String("<br>");
    tmp += tooltipFormatAttendees(mCalendar, incidence);

    int categoryCount = incidence->categories().count();
    if (categoryCount > 0) {
        tmp += QLatin1String("<br>");
        tmp += QLatin1String("<i>") + i18np("Category:", "Categories:", categoryCount) + QLatin1String("</i>") +QLatin1String("&nbsp;");
        tmp += incidence->categories().join(QLatin1String(", "));
    }

    tmp += QLatin1String("</qt>");
    return tmp;
}
//@endcond

QString IncidenceFormatter::toolTipStr(const QString &sourceName,
                                       const IncidenceBase::Ptr &incidence,
                                       const QDate &date,
                                       bool richText,
                                       KDateTime::Spec spec)
{
    ToolTipVisitor v;
    if (incidence && v.act(sourceName, incidence, date, richText, spec)) {
        return v.result();
    } else {
        return QString();
    }
}

/*******************************************************************
 *  Helper functions for the Incidence tooltips
 *******************************************************************/

//@cond PRIVATE
static QString mailBodyIncidence(const Incidence::Ptr &incidence)
{
    QString body;
    if (!incidence->summary().isEmpty()) {
        body += i18n("Summary: %1\n", incidence->richSummary());
    }
    if (!incidence->organizer()->isEmpty()) {
        body += i18n("Organizer: %1\n", incidence->organizer()->fullName());
    }
    if (!incidence->location().isEmpty()) {
        body += i18n("Location: %1\n", incidence->richLocation());
    }
    return body;
}
//@endcond

//@cond PRIVATE
class KCalUtils::IncidenceFormatter::MailBodyVisitor : public Visitor
{
public:
    MailBodyVisitor()
        : mSpec(KDateTime::Spec()), mResult(QLatin1String("")) {}

    bool act(IncidenceBase::Ptr incidence, KDateTime::Spec spec=KDateTime::Spec())
    {
        mSpec = spec;
        mResult = QLatin1String("");
        return incidence ? incidence->accept(*this, incidence) : false;
    }
    QString result() const
    {
        return mResult;
    }

protected:
    bool visit(Event::Ptr event);
    bool visit(Todo::Ptr todo);
    bool visit(Journal::Ptr journal);
    bool visit(FreeBusy::Ptr)
    {
        mResult = i18n("This is a Free Busy Object");
        return !mResult.isEmpty();
    }
protected:
    KDateTime::Spec mSpec;
    QString mResult;
};

bool IncidenceFormatter::MailBodyVisitor::visit(Event::Ptr event)
{
    QString recurrence[]= {
        i18nc("no recurrence", "None"),
        i18nc("event recurs by minutes", "Minutely"),
        i18nc("event recurs by hours", "Hourly"),
        i18nc("event recurs by days", "Daily"),
        i18nc("event recurs by weeks", "Weekly"),
        i18nc("event recurs same position (e.g. first monday) each month", "Monthly Same Position"),
        i18nc("event recurs same day each month", "Monthly Same Day"),
        i18nc("event recurs same month each year", "Yearly Same Month"),
        i18nc("event recurs same day each year", "Yearly Same Day"),
        i18nc("event recurs same position (e.g. first monday) each year", "Yearly Same Position")
    };

    mResult = mailBodyIncidence(event);
    mResult += i18n("Start Date: %1\n", dateToString(event->dtStart(), true, mSpec));
    if (!event->allDay()) {
        mResult += i18n("Start Time: %1\n", timeToString(event->dtStart(), true, mSpec));
    }
    if (event->dtStart() != event->dtEnd()) {
        mResult += i18n("End Date: %1\n", dateToString(event->dtEnd(), true, mSpec));
    }
    if (!event->allDay()) {
        mResult += i18n("End Time: %1\n", timeToString(event->dtEnd(), true, mSpec));
    }
    if (event->recurs()) {
        Recurrence *recur = event->recurrence();
        // TODO: Merge these two to one of the form "Recurs every 3 days"
        mResult += i18n("Recurs: %1\n", recurrence[ recur->recurrenceType() ]);
        mResult += i18n("Frequency: %1\n", event->recurrence()->frequency());

        if (recur->duration() > 0) {
            mResult += i18np("Repeats once", "Repeats %1 times", recur->duration());
            mResult += QLatin1Char('\n');
        } else {
            if (recur->duration() != -1) {
// TODO_Recurrence: What to do with all-day
                QString endstr;
                if (event->allDay()) {
                    endstr = KGlobal::locale()->formatDate(recur->endDate());
                } else {
                    endstr = KGlobal::locale()->formatDateTime(recur->endDateTime().dateTime());
                }
                mResult += i18n("Repeat until: %1\n", endstr);
            } else {
                mResult += i18n("Repeats forever\n");
            }
        }
    }

    if (!event->description().isEmpty()) {
        QString descStr;
        if (event->descriptionIsRich() ||
                event->description().startsWith(QLatin1String("<!DOCTYPE HTML")))
        {
            descStr = cleanHtml(event->description());
        } else {
            descStr = event->description();
        }
        if (!descStr.isEmpty()) {
            mResult += i18n("Details:\n%1\n", descStr);
        }
    }
    return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit(Todo::Ptr todo)
{
    mResult = mailBodyIncidence(todo);

    if (todo->hasStartDate() && todo->dtStart().isValid()) {
        mResult += i18n("Start Date: %1\n", dateToString(todo->dtStart(false), true, mSpec));
        if (!todo->allDay()) {
            mResult += i18n("Start Time: %1\n", timeToString(todo->dtStart(false), true, mSpec));
        }
    }
    if (todo->hasDueDate() && todo->dtDue().isValid()) {
        mResult += i18n("Due Date: %1\n", dateToString(todo->dtDue(), true, mSpec));
        if (!todo->allDay()) {
            mResult += i18n("Due Time: %1\n", timeToString(todo->dtDue(), true, mSpec));
        }
    }
    QString details = todo->richDescription();
    if (!details.isEmpty()) {
        mResult += i18n("Details:\n%1\n", details);
    }
    return !mResult.isEmpty();
}

bool IncidenceFormatter::MailBodyVisitor::visit(Journal::Ptr journal)
{
    mResult = mailBodyIncidence(journal);
    mResult += i18n("Date: %1\n", dateToString(journal->dtStart(), true, mSpec));
    if (!journal->allDay()) {
        mResult += i18n("Time: %1\n", timeToString(journal->dtStart(), true, mSpec));
    }
    if (!journal->description().isEmpty()) {
        mResult += i18n("Text of the journal:\n%1\n", journal->richDescription());
    }
    return !mResult.isEmpty();
}
//@endcond

QString IncidenceFormatter::mailBodyStr(const IncidenceBase::Ptr &incidence,
                                        KDateTime::Spec spec)
{
    if (!incidence) {
        return QString();
    }

    MailBodyVisitor v;
    if (v.act(incidence, spec)) {
        return v.result();
    }
    return QString();
}

//@cond PRIVATE
static QString recurEnd(const Incidence::Ptr &incidence)
{
    QString endstr;
    if (incidence->allDay()) {
        endstr = KGlobal::locale()->formatDate(incidence->recurrence()->endDate());
    } else {
        endstr = KGlobal::locale()->formatDateTime(incidence->recurrence()->endDateTime());
    }
    return endstr;
}
//@endcond

/************************************
 *  More static formatting functions
 ************************************/

QString IncidenceFormatter::recurrenceString(const Incidence::Ptr &incidence)
{
    if (incidence->hasRecurrenceId()) {
        return QLatin1String("Recurrence exception");
    }

    if (!incidence->recurs()) {
        return i18n("No recurrence");
    }
    static QStringList dayList;
    if (dayList.isEmpty()) {
        dayList.append(i18n("31st Last"));
        dayList.append(i18n("30th Last"));
        dayList.append(i18n("29th Last"));
        dayList.append(i18n("28th Last"));
        dayList.append(i18n("27th Last"));
        dayList.append(i18n("26th Last"));
        dayList.append(i18n("25th Last"));
        dayList.append(i18n("24th Last"));
        dayList.append(i18n("23rd Last"));
        dayList.append(i18n("22nd Last"));
        dayList.append(i18n("21st Last"));
        dayList.append(i18n("20th Last"));
        dayList.append(i18n("19th Last"));
        dayList.append(i18n("18th Last"));
        dayList.append(i18n("17th Last"));
        dayList.append(i18n("16th Last"));
        dayList.append(i18n("15th Last"));
        dayList.append(i18n("14th Last"));
        dayList.append(i18n("13th Last"));
        dayList.append(i18n("12th Last"));
        dayList.append(i18n("11th Last"));
        dayList.append(i18n("10th Last"));
        dayList.append(i18n("9th Last"));
        dayList.append(i18n("8th Last"));
        dayList.append(i18n("7th Last"));
        dayList.append(i18n("6th Last"));
        dayList.append(i18n("5th Last"));
        dayList.append(i18n("4th Last"));
        dayList.append(i18n("3rd Last"));
        dayList.append(i18n("2nd Last"));
        dayList.append(i18nc("last day of the month", "Last"));
        dayList.append(i18nc("unknown day of the month", "unknown"));     //#31 - zero offset from UI
        dayList.append(i18n("1st"));
        dayList.append(i18n("2nd"));
        dayList.append(i18n("3rd"));
        dayList.append(i18n("4th"));
        dayList.append(i18n("5th"));
        dayList.append(i18n("6th"));
        dayList.append(i18n("7th"));
        dayList.append(i18n("8th"));
        dayList.append(i18n("9th"));
        dayList.append(i18n("10th"));
        dayList.append(i18n("11th"));
        dayList.append(i18n("12th"));
        dayList.append(i18n("13th"));
        dayList.append(i18n("14th"));
        dayList.append(i18n("15th"));
        dayList.append(i18n("16th"));
        dayList.append(i18n("17th"));
        dayList.append(i18n("18th"));
        dayList.append(i18n("19th"));
        dayList.append(i18n("20th"));
        dayList.append(i18n("21st"));
        dayList.append(i18n("22nd"));
        dayList.append(i18n("23rd"));
        dayList.append(i18n("24th"));
        dayList.append(i18n("25th"));
        dayList.append(i18n("26th"));
        dayList.append(i18n("27th"));
        dayList.append(i18n("28th"));
        dayList.append(i18n("29th"));
        dayList.append(i18n("30th"));
        dayList.append(i18n("31st"));
    }

    const int weekStart = KGlobal::locale()->weekStartDay();
    QString dayNames;
    const KCalendarSystem *calSys = KGlobal::locale()->calendar();

    Recurrence *recur = incidence->recurrence();

    QString txt, recurStr;
    static QString noRecurrence = i18n("No recurrence");
    switch (recur->recurrenceType()) {
    case Recurrence::rNone:
        return noRecurrence;

    case Recurrence::rMinutely:
        if (recur->duration() != -1) {
            recurStr = i18np("Recurs every minute until %2",
                             "Recurs every %1 minutes until %2",
                             recur->frequency(), recurEnd(incidence));
            if (recur->duration() >  0) {
                recurStr += i18nc("number of occurrences",
                                  " (<numid>%1</numid> occurrences)",
                                  recur->duration());
            }
        } else {
            recurStr = i18np("Recurs every minute",
                             "Recurs every %1 minutes", recur->frequency());
        }
        break;

    case Recurrence::rHourly:
        if (recur->duration() != -1) {
            recurStr = i18np("Recurs hourly until %2",
                             "Recurs every %1 hours until %2",
                             recur->frequency(), recurEnd(incidence));
            if (recur->duration() >  0) {
                recurStr += i18nc("number of occurrences",
                                  " (<numid>%1</numid> occurrences)",
                                  recur->duration());
            }
        } else {
            recurStr = i18np("Recurs hourly", "Recurs every %1 hours", recur->frequency());
        }
        break;

    case Recurrence::rDaily:
        if (recur->duration() != -1) {
            recurStr = i18np("Recurs daily until %2",
                             "Recurs every %1 days until %2",
                             recur->frequency(), recurEnd(incidence));
            if (recur->duration() >  0) {
                recurStr += i18nc("number of occurrences",
                                  " (<numid>%1</numid> occurrences)",
                                  recur->duration());
            }
        } else {
            recurStr = i18np("Recurs daily", "Recurs every %1 days", recur->frequency());
        }
        break;

    case Recurrence::rWeekly:
    {
        bool addSpace = false;
        for (int i = 0; i < 7; ++i) {
            if (recur->days().testBit((i + weekStart + 6) % 7)) {
                if (addSpace) {
                    dayNames.append(i18nc("separator for list of days", ", "));
                }
                dayNames.append(calSys->weekDayName(((i + weekStart + 6) % 7) + 1,
                                                    KCalendarSystem::ShortDayName));
                addSpace = true;
            }
        }
        if (dayNames.isEmpty()) {
            dayNames = i18nc("Recurs weekly on no days", "no days");
        }
        if (recur->duration() != -1) {
            recurStr = i18ncp("Recurs weekly on [list of days] until end-date",
                              "Recurs weekly on %2 until %3",
                              "Recurs every <numid>%1</numid> weeks on %2 until %3",
                              recur->frequency(), dayNames, recurEnd(incidence));
            if (recur->duration() >  0) {
                recurStr += i18nc("number of occurrences",
                                  " (<numid>%1</numid> occurrences)",
                                  recur->duration());
            }
        } else {
            recurStr = i18ncp("Recurs weekly on [list of days]",
                              "Recurs weekly on %2",
                              "Recurs every <numid>%1</numid> weeks on %2",
                              recur->frequency(), dayNames);
        }
        break;
    }
    case Recurrence::rMonthlyPos:
    {
        if (!recur->monthPositions().isEmpty()) {
            RecurrenceRule::WDayPos rule = recur->monthPositions()[0];
            if (recur->duration() != -1) {
                recurStr = i18ncp("Recurs every N months on the [2nd|3rd|...]"
                                  " weekdayname until end-date",
                                  "Recurs every month on the %2 %3 until %4",
                                  "Recurs every <numid>%1</numid> months on the %2 %3 until %4",
                                  recur->frequency(),
                                  dayList[rule.pos() + 31],
                                  calSys->weekDayName(rule.day(), KCalendarSystem::LongDayName),
                                  recurEnd(incidence));
                if (recur->duration() >  0) {
                    recurStr += i18nc("number of occurrences",
                                      " (<numid>%1</numid> occurrences)",
                                      recur->duration());
                }
            } else {
                recurStr = i18ncp("Recurs every N months on the [2nd|3rd|...] weekdayname",
                                  "Recurs every month on the %2 %3",
                                  "Recurs every %1 months on the %2 %3",
                                  recur->frequency(),
                                  dayList[rule.pos() + 31],
                                  calSys->weekDayName(rule.day(), KCalendarSystem::LongDayName));
            }
        }
        break;
    }
    case Recurrence::rMonthlyDay:
    {
        if (!recur->monthDays().isEmpty()) {
            int days = recur->monthDays()[0];
            if (recur->duration() != -1) {
                recurStr = i18ncp("Recurs monthly on the [1st|2nd|...] day until end-date",
                                  "Recurs monthly on the %2 day until %3",
                                  "Recurs every %1 months on the %2 day until %3",
                                  recur->frequency(),
                                  dayList[days + 31],
                                  recurEnd(incidence));
                if (recur->duration() >  0) {
                    recurStr += i18nc("number of occurrences",
                                      " (<numid>%1</numid> occurrences)",
                                      recur->duration());
                }
            } else {
                recurStr = i18ncp("Recurs monthly on the [1st|2nd|...] day",
                                  "Recurs monthly on the %2 day",
                                  "Recurs every <numid>%1</numid> month on the %2 day",
                                  recur->frequency(),
                                  dayList[days + 31]);
            }
        }
        break;
    }
    case Recurrence::rYearlyMonth:
    {
        if (recur->duration() != -1) {
            if (!recur->yearDates().isEmpty() && !recur->yearMonths().isEmpty()) {
                recurStr = i18ncp("Recurs Every N years on month-name [1st|2nd|...]"
                                  " until end-date",
                                  "Recurs yearly on %2 %3 until %4",
                                  "Recurs every %1 years on %2 %3 until %4",
                                  recur->frequency(),
                                  calSys->monthName(recur->yearMonths()[0], recur->startDate().year()),
                                  dayList[ recur->yearDates()[0] + 31 ],
                                  recurEnd(incidence));
                if (recur->duration() >  0) {
                    recurStr += i18nc("number of occurrences",
                                      " (<numid>%1</numid> occurrences)",
                                      recur->duration());
                }
            }
        } else {
            if (!recur->yearDates().isEmpty() && !recur->yearMonths().isEmpty()) {
                recurStr = i18ncp("Recurs Every N years on month-name [1st|2nd|...]",
                                  "Recurs yearly on %2 %3",
                                  "Recurs every %1 years on %2 %3",
                                  recur->frequency(),
                                  calSys->monthName(recur->yearMonths()[0],
                                                    recur->startDate().year()),
                                  dayList[ recur->yearDates()[0] + 31 ]);
            } else {
                if (!recur->yearMonths().isEmpty()) {
                    recurStr = i18nc("Recurs Every year on month-name [1st|2nd|...]",
                                     "Recurs yearly on %1 %2",
                                     calSys->monthName(recur->yearMonths()[0],
                                                       recur->startDate().year()),
                                     dayList[ recur->startDate().day() + 31 ]);
                } else {
                    recurStr = i18nc("Recurs Every year on month-name [1st|2nd|...]",
                                     "Recurs yearly on %1 %2",
                                     calSys->monthName(recur->startDate().month(),
                                                       recur->startDate().year()),
                                     dayList[ recur->startDate().day() + 31 ]);
                }
            }
        }
        break;
    }
    case Recurrence::rYearlyDay:
        if (!recur->yearDays().isEmpty()) {
            if (recur->duration() != -1) {
                recurStr = i18ncp("Recurs every N years on day N until end-date",
                                  "Recurs every year on day <numid>%2</numid> until %3",
                                  "Recurs every <numid>%1</numid> years"
                                  " on day <numid>%2</numid> until %3",
                                  recur->frequency(),
                                  recur->yearDays()[0],
                                  recurEnd(incidence));
                if (recur->duration() >  0) {
                    recurStr += i18nc("number of occurrences",
                                      " (<numid>%1</numid> occurrences)",
                                      recur->duration());
                }
            } else {
                recurStr = i18ncp("Recurs every N YEAR[S] on day N",
                                  "Recurs every year on day <numid>%2</numid>",
                                  "Recurs every <numid>%1</numid> years"
                                  " on day <numid>%2</numid>",
                                  recur->frequency(), recur->yearDays()[0]);
            }
        }
        break;
    case Recurrence::rYearlyPos:
    {
        if (!recur->yearMonths().isEmpty() && !recur->yearPositions().isEmpty()) {
            RecurrenceRule::WDayPos rule = recur->yearPositions()[0];
            if (recur->duration() != -1) {
                recurStr = i18ncp("Every N years on the [2nd|3rd|...] weekdayname "
                                  "of monthname until end-date",
                                  "Every year on the %2 %3 of %4 until %5",
                                  "Every <numid>%1</numid> years on the %2 %3 of %4"
                                  " until %5",
                                  recur->frequency(),
                                  dayList[rule.pos() + 31],
                                  calSys->weekDayName(rule.day(), KCalendarSystem::LongDayName),
                                  calSys->monthName(recur->yearMonths()[0], recur->startDate().year()),
                                  recurEnd(incidence));
                if (recur->duration() >  0) {
                    recurStr += i18nc("number of occurrences",
                                      " (<numid>%1</numid> occurrences)",
                                      recur->duration());
                }
            } else {
                recurStr = i18ncp("Every N years on the [2nd|3rd|...] weekdayname "
                                  "of monthname",
                                  "Every year on the %2 %3 of %4",
                                  "Every <numid>%1</numid> years on the %2 %3 of %4",
                                  recur->frequency(),
                                  dayList[rule.pos() + 31],
                                  calSys->weekDayName(rule.day(), KCalendarSystem::LongDayName),
                                  calSys->monthName(recur->yearMonths()[0], recur->startDate().year()));
            }
        }
    }
    break;
    }

    if (recurStr.isEmpty()) {
        recurStr = i18n("Incidence recurs");
    }

    // Now, append the EXDATEs
    DateTimeList l = recur->exDateTimes();
    DateTimeList::ConstIterator il;
    QStringList exStr;
    for (il = l.constBegin(); il != l.constEnd(); ++il) {
        switch (recur->recurrenceType()) {
        case Recurrence::rMinutely:
            exStr << i18n("minute %1", (*il).time().minute());
            break;
        case Recurrence::rHourly:
            exStr << KGlobal::locale()->formatTime((*il).time());
            break;
        case Recurrence::rDaily:
            exStr << KGlobal::locale()->formatDate((*il).date(), KLocale::ShortDate);
            break;
        case Recurrence::rWeekly:
            exStr << calSys->weekDayName((*il).date(), KCalendarSystem::ShortDayName);
            break;
        case Recurrence::rMonthlyPos:
            exStr << KGlobal::locale()->formatDate((*il).date(), KLocale::ShortDate);
            break;
        case Recurrence::rMonthlyDay:
            exStr << KGlobal::locale()->formatDate((*il).date(), KLocale::ShortDate);
            break;
        case Recurrence::rYearlyMonth:
            exStr << calSys->monthName((*il).date(), KCalendarSystem::LongName);
            break;
        case Recurrence::rYearlyDay:
            exStr << KGlobal::locale()->formatDate((*il).date(), KLocale::ShortDate);
            break;
        case Recurrence::rYearlyPos:
            exStr << KGlobal::locale()->formatDate((*il).date(), KLocale::ShortDate);
            break;
        }
    }

    DateList d = recur->exDates();
    DateList::ConstIterator dl;
    for (dl = d.constBegin(); dl != d.constEnd(); ++dl) {
        switch (recur->recurrenceType()) {
        case Recurrence::rDaily:
            exStr << KGlobal::locale()->formatDate((*dl), KLocale::ShortDate);
            break;
        case Recurrence::rWeekly:
            // exStr << calSys->weekDayName( (*dl), KCalendarSystem::ShortDayName );
            // kolab/issue4735, should be ( excluding 3 days ), instead of excluding( Fr,Fr,Fr )
            if (exStr.isEmpty()) {
                exStr << i18np("1 day", "%1 days", recur->exDates().count());
            }
            break;
        case Recurrence::rMonthlyPos:
            exStr << KGlobal::locale()->formatDate((*dl), KLocale::ShortDate);
            break;
        case Recurrence::rMonthlyDay:
            exStr << KGlobal::locale()->formatDate((*dl), KLocale::ShortDate);
            break;
        case Recurrence::rYearlyMonth:
            exStr << calSys->monthName((*dl), KCalendarSystem::LongName);
            break;
        case Recurrence::rYearlyDay:
            exStr << KGlobal::locale()->formatDate((*dl), KLocale::ShortDate);
            break;
        case Recurrence::rYearlyPos:
            exStr << KGlobal::locale()->formatDate((*dl), KLocale::ShortDate);
            break;
        }
    }

    if (!exStr.isEmpty()) {
        recurStr = i18n("%1 (excluding %2)", recurStr, exStr.join(QLatin1String(",")));
    }

    return recurStr;
}

QString IncidenceFormatter::timeToString(const KDateTime &date,
        bool shortfmt,
        const KDateTime::Spec &spec)
{
    if (spec.isValid()) {

        QString timeZone;
        if (spec.timeZone() != KSystemTimeZones::local()) {
            timeZone = QLatin1Char(' ') + spec.timeZone().name();
        }

        return KGlobal::locale()->formatTime(date.toTimeSpec(spec).time(), !shortfmt) + timeZone;
    } else {
        return KGlobal::locale()->formatTime(date.time(), !shortfmt);
    }
}

QString IncidenceFormatter::dateToString(const KDateTime &date,
        bool shortfmt,
        const KDateTime::Spec &spec)
{
    if (spec.isValid()) {

        QString timeZone;
        if (spec.timeZone() != KSystemTimeZones::local()) {
            timeZone = QLatin1Char(' ') + spec.timeZone().name();
        }

        return
            KGlobal::locale()->formatDate(date.toTimeSpec(spec).date(),
                                          (shortfmt ? KLocale::ShortDate : KLocale::LongDate)) +
            timeZone;
    } else {
        return
            KGlobal::locale()->formatDate(date.date(),
                                          (shortfmt ? KLocale::ShortDate : KLocale::LongDate));
    }
}

QString IncidenceFormatter::dateTimeToString(const KDateTime &date,
        bool allDay,
        bool shortfmt,
        const KDateTime::Spec &spec)
{
    if (allDay) {
        return dateToString(date, shortfmt, spec);
    }

    if (spec.isValid()) {
        QString timeZone;
        if (spec.timeZone() != KSystemTimeZones::local()) {
            timeZone = QLatin1Char(' ') + spec.timeZone().name();
        }

        return KGlobal::locale()->formatDateTime(
                   date.toTimeSpec(spec).dateTime(),
                   (shortfmt ? KLocale::ShortDate : KLocale::LongDate)) + timeZone;
    } else {
        return  KGlobal::locale()->formatDateTime(
                    date.dateTime(),
                    (shortfmt ? KLocale::ShortDate : KLocale::LongDate));
    }
}

QString IncidenceFormatter::resourceString(const Calendar::Ptr &calendar,
        const Incidence::Ptr &incidence)
{
    Q_UNUSED(calendar);
    Q_UNUSED(incidence);
    return QString();
}

static QString secs2Duration(int secs)
{
    QString tmp;
    int days = secs / 86400;
    if (days > 0) {
        tmp += i18np("1 day", "%1 days", days);
        tmp += QLatin1Char(' ');
        secs -= (days * 86400);
    }
    int hours = secs / 3600;
    if (hours > 0) {
        tmp += i18np("1 hour", "%1 hours", hours);
        tmp += QLatin1Char(' ');
        secs -= (hours * 3600);
    }
    int mins = secs / 60;
    if (mins > 0) {
        tmp += i18np("1 minute", "%1 minutes", mins);
    }
    return tmp;
}

QString IncidenceFormatter::durationString(const Incidence::Ptr &incidence)
{
    QString tmp;
    if (incidence->type() == Incidence::TypeEvent) {
        Event::Ptr event = incidence.staticCast<Event>();
        if (event->hasEndDate()) {
            if (!event->allDay()) {
                tmp = secs2Duration(event->dtStart().secsTo(event->dtEnd()));
            } else {
                tmp = i18np("1 day", "%1 days",
                            event->dtStart().date().daysTo(event->dtEnd().date()) + 1);
            }
        } else {
            tmp = i18n("forever");
        }
    } else if (incidence->type() == Incidence::TypeTodo) {
        Todo::Ptr todo = incidence.staticCast<Todo>();
        if (todo->hasDueDate()) {
            if (todo->hasStartDate()) {
                if (!todo->allDay()) {
                    tmp = secs2Duration(todo->dtStart().secsTo(todo->dtDue()));
                } else {
                    tmp = i18np("1 day", "%1 days",
                                todo->dtStart().date().daysTo(todo->dtDue().date()) + 1);
                }
            }
        }
    }
    return tmp;
}

QStringList IncidenceFormatter::reminderStringList(const Incidence::Ptr &incidence,
        bool shortfmt)
{
    //TODO: implement shortfmt=false
    Q_UNUSED(shortfmt);

    QStringList reminderStringList;

    if (incidence) {
        Alarm::List alarms = incidence->alarms();
        Alarm::List::ConstIterator it;
        for (it = alarms.constBegin(); it != alarms.constEnd(); ++it) {
            Alarm::Ptr alarm = *it;
            int offset = 0;
            QString remStr, atStr, offsetStr;
            if (alarm->hasTime()) {
                offset = 0;
                if (alarm->time().isValid()) {
                    atStr = KGlobal::locale()->formatDateTime(alarm->time());
                }
            } else if (alarm->hasStartOffset()) {
                offset = alarm->startOffset().asSeconds();
                if (offset < 0) {
                    offset = -offset;
                    offsetStr = i18nc("N days/hours/minutes before the start datetime",
                                      "%1 before the start", secs2Duration(offset));
                } else if (offset > 0) {
                    offsetStr = i18nc("N days/hours/minutes after the start datetime",
                                      "%1 after the start", secs2Duration(offset));
                } else { //offset is 0
                    if (incidence->dtStart().isValid()) {
                        atStr = KGlobal::locale()->formatDateTime(incidence->dtStart());
                    }
                }
            } else if (alarm->hasEndOffset()) {
                offset = alarm->endOffset().asSeconds();
                if (offset < 0) {
                    offset = -offset;
                    if (incidence->type() == Incidence::TypeTodo) {
                        offsetStr = i18nc("N days/hours/minutes before the due datetime",
                                          "%1 before the to-do is due", secs2Duration(offset));
                    } else {
                        offsetStr = i18nc("N days/hours/minutes before the end datetime",
                                          "%1 before the end", secs2Duration(offset));
                    }
                } else if (offset > 0) {
                    if (incidence->type() == Incidence::TypeTodo) {
                        offsetStr = i18nc("N days/hours/minutes after the due datetime",
                                          "%1 after the to-do is due", secs2Duration(offset));
                    } else {
                        offsetStr = i18nc("N days/hours/minutes after the end datetime",
                                          "%1 after the end", secs2Duration(offset));
                    }
                } else { //offset is 0
                    if (incidence->type() == Incidence::TypeTodo) {
                        Todo::Ptr t = incidence.staticCast<Todo>();
                        if (t->dtDue().isValid()) {
                            atStr = KGlobal::locale()->formatDateTime(t->dtDue());
                        }
                    } else {
                        Event::Ptr e = incidence.staticCast<Event>();
                        if (e->dtEnd().isValid()) {
                            atStr = KGlobal::locale()->formatDateTime(e->dtEnd());
                        }
                    }
                }
            }
            if (offset == 0) {
                if (!atStr.isEmpty()) {
                    remStr = i18nc("reminder occurs at datetime", "at %1", atStr);
                }
            } else {
                remStr = offsetStr;
            }

            if (alarm->repeatCount() > 0) {
                QString countStr = i18np("repeats once", "repeats %1 times", alarm->repeatCount());
                QString intervalStr = i18nc("interval is N days/hours/minutes",
                                            "interval is %1",
                                            secs2Duration(alarm->snoozeTime().asSeconds()));
                QString repeatStr = i18nc("(repeat string, interval string)",
                                          "(%1, %2)", countStr, intervalStr);
                remStr = remStr + QLatin1Char(' ') + repeatStr;
            }
            reminderStringList << remStr;
        }
    }

    return reminderStringList;
}
