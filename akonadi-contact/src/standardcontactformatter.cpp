/*
    This file is part of Akonadi Contact.

    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "standardcontactformatter.h"

#include <item.h>
#include <kcontacts/addressee.h>
#include <kcolorscheme.h>
#include <kconfiggroup.h>

#include <KLocalizedString>
#include <KLocalizedString>
#include <kstringhandler.h>

#include <QtCore/QSet>
#include <QLocale>

using namespace Akonadi;

class Q_DECL_HIDDEN StandardContactFormatter::Private
{
public:
    Private()
        : displayQRcode(true)
    {

    }

    bool displayQRcode;
};

StandardContactFormatter::StandardContactFormatter()
    : d(new Private())
{
}

StandardContactFormatter::~StandardContactFormatter()
{
    delete d;
}

static int contactAge(const QDate &date)
{
    QDate now = QDate::currentDate();
    int age = now.year() - date.year();
    if (date > now.addYears(-age)) {
        age--;
    }
    return age;
}

QString StandardContactFormatter::toHtml(HtmlForm form) const
{
    KContacts::Addressee rawContact;
    const Akonadi::Item localItem = item();
    if (localItem.isValid() && localItem.hasPayload<KContacts::Addressee>()) {
        rawContact = localItem.payload<KContacts::Addressee>();
    } else {
        rawContact = contact();
    }

    if (rawContact.isEmpty()) {
        return QString();
    }

    // We'll be building a table to display the vCard in.
    // Each row of the table will be built using one of these strings for its HTML.

    // single data item:
    //  %1 is the item name
    //  %2 is the item value
    QString rowFmtStr1 = QStringLiteral(
                             "<tr valign=\"top\">"
                             "<td align=\"right\" valign=\"top\" width=\"30%\"><b><font color=\"grey\">%1</font></b></td>\n"
                             "<td colspan=\"2\" align=\"left\" valign=\"top\" width=\"70%\"><font>%2</font></td>\n"
                             "</tr>\n"
                         );

    // data item plus additional icon(s):
    //  %1 is the item name
    //  %2 is the item value
    //  %3 is the icon(s), each as a HTML <a><img> tag
    QString rowFmtStr2 = QStringLiteral(
                             "<tr valign=\"top\">"
                             "<td align=\"right\" valign=\"top\" width=\"30%\"><b><font color=\"grey\">%1</font></b></td>\n"
                             "<td align=\"left\" valign=\"top\"><font>%2</font></td>\n"
                             "<td align=\"left\" valign=\"top\">%3</td>\n"
                             "</tr>\n"
                         );

    // Build the table's rows here
    QString dynamicPart;

    // Birthday
    const QDate date = rawContact.birthday().date();
    const int years = contactAge(date);

    if (date.isValid()) {
        dynamicPart += rowFmtStr1
                       .arg(KContacts::Addressee::birthdayLabel())
                       .arg(QLocale().toString(date) +
                            QLatin1String("&nbsp;&nbsp;") + i18np("(One year old)", "(%1 years old)", years));
    }

    // Phone Numbers
    int counter = 0;
    foreach (const KContacts::PhoneNumber &number, rawContact.phoneNumbers()) {

        QString dispLabel = number.typeLabel().replace(QStringLiteral(" "), QStringLiteral("&nbsp;"));
        QString dispValue = QStringLiteral("<a href=\"phone:?index=%1\">%2</a>").arg(counter).arg(number.number().toHtmlEscaped());
        if (number.type() & KContacts::PhoneNumber::Cell) {
            QString dispIcon = QStringLiteral("<a href=\"sms:?index=%1\" title=\"%2\"><img src=\"sms_icon\" align=\"top\"/>")
                               .arg(counter)
                               .arg(i18nc("@info:tooltip", "Send SMS"));
            dynamicPart += rowFmtStr2
                           .arg(dispLabel)
                           .arg(dispValue)
                           .arg(dispIcon);
        } else {
            dynamicPart += rowFmtStr1
                           .arg(dispLabel)
                           .arg(dispValue);
        }

        ++counter;
    }

    // EMails
    foreach (const QString &email, rawContact.emails()) {
        const QString type = i18nc("a contact's email address", "Email");

        const QString fullEmail = QString::fromLatin1(QUrl::toPercentEncoding(rawContact.fullEmail(email)));

        dynamicPart += rowFmtStr1.arg(type)
                       .arg(QStringLiteral("<a href=\"mailto:%1\">%2</a>")
                            .arg(fullEmail, email));
    }

    // Homepage
    if (rawContact.url().isValid()) {
        QString url = rawContact.url().url().url();
        if (!url.startsWith(QLatin1String("http://")) && !url.startsWith(QLatin1String("https://"))) {
            url = QLatin1String("http://") + url;
        }

        url = KStringHandler::tagUrls(url.toHtmlEscaped());
        dynamicPart += rowFmtStr1.arg(i18n("Homepage")).arg(url);
    }

    // Blog Feed
    const QString blog = rawContact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("BlogFeed"));
    if (!blog.isEmpty()) {
        dynamicPart += rowFmtStr1.arg(i18n("Blog Feed")).arg(KStringHandler::tagUrls(blog.toHtmlEscaped()));
    }

    // Addresses
    counter = 0;
    foreach (const KContacts::Address &address, rawContact.addresses()) {
        QString formattedAddress;

        if (address.label().isEmpty()) {
            formattedAddress = address.formattedAddress().trimmed().toHtmlEscaped();
        } else {
            formattedAddress = address.label().toHtmlEscaped();
        }

        formattedAddress = formattedAddress.replace(QRegExp(QStringLiteral("\n+")), QStringLiteral("<br>"));

        const QString url = QStringLiteral("<a href=\"address:?index=%1\" title=\"%2\"><img src=\"map_icon\" alt=\"%2\"/></a>")
                            .arg(counter)
                            .arg(i18nc("@info:tooltip", "Show address on map"));
        counter++;

        dynamicPart += rowFmtStr2
                       .arg(KContacts::Address::typeLabel(address.type()))
                       .arg(formattedAddress)
                       .arg(url);
    }

    // Note
    QString notes;
    if (!rawContact.note().isEmpty()) {
        notes = rowFmtStr1.arg(i18n("Notes")).arg(rawContact.note().toHtmlEscaped().replace(QLatin1Char('\n'), QLatin1String("<br>")));
    }

    // Custom Data
    QString customData;
    static QMap<QString, QString> titleMap;
    if (titleMap.isEmpty()) {
        titleMap.insert(QStringLiteral("Department"), i18n("Department"));
        titleMap.insert(QStringLiteral("Profession"), i18n("Profession"));
        titleMap.insert(QStringLiteral("AssistantsName"), i18n("Assistant's Name"));
        titleMap.insert(QStringLiteral("ManagersName"), i18n("Manager's Name"));
        titleMap.insert(QStringLiteral("SpousesName"), i18nc("Wife/Husband/...", "Partner's Name"));
        titleMap.insert(QStringLiteral("Office"), i18n("Office"));
        titleMap.insert(QStringLiteral("IMAddress"), i18n("IM Address"));
        titleMap.insert(QStringLiteral("Anniversary"), i18n("Anniversary"));
        titleMap.insert(QStringLiteral("AddressBook"), i18n("Address Book"));
    }

    static QSet<QString> blacklistedKeys;
    if (blacklistedKeys.isEmpty()) {
        blacklistedKeys.insert(QStringLiteral("CRYPTOPROTOPREF"));
        blacklistedKeys.insert(QStringLiteral("OPENPGPFP"));
        blacklistedKeys.insert(QStringLiteral("SMIMEFP"));
        blacklistedKeys.insert(QStringLiteral("CRYPTOSIGNPREF"));
        blacklistedKeys.insert(QStringLiteral("CRYPTOENCRYPTPREF"));
        blacklistedKeys.insert(QStringLiteral("MailPreferedFormatting"));
        blacklistedKeys.insert(QStringLiteral("MailAllowToRemoteContent"));
    }

    if (!rawContact.customs().empty()) {
        const QStringList customs = rawContact.customs();
        foreach (QString custom, customs) {   //krazy:exclude=foreach
            if (custom.startsWith(QStringLiteral("KADDRESSBOOK-"))) {
                custom.remove(QStringLiteral("KADDRESSBOOK-X-"));
                custom.remove(QStringLiteral("KADDRESSBOOK-"));

                int pos = custom.indexOf(QLatin1Char(':'));
                QString key = custom.left(pos);
                QString value = custom.mid(pos + 1);

                // convert anniversary correctly
                if (key == QLatin1String("Anniversary") || key == QLatin1String("ANNIVERSARY")) {
                    const QDateTime dateTime = QDateTime::fromString(value, Qt::ISODate);
                    value = QLocale().toString(dateTime.date());
                } else if (key == QLatin1String("BlogFeed")) {      // blog is handled separated
                    continue;
                } else if (blacklistedKeys.contains(key)) {
                    continue;
                }

                // check whether we have a mapping for the title
                const QMap<QString, QString>::ConstIterator keyIt = titleMap.constFind(key);
                bool needToEscape = true;
                if (keyIt != titleMap.constEnd()) {
                    key = keyIt.value();
                } else {
                    // check whether it is a custom local field
                    foreach (const QVariantMap &description, customFieldDescriptions()) {
                        if (description.value(QStringLiteral("key")).toString() == key) {
                            key = description.value(QStringLiteral("title")).toString();
                            const QString descriptionType = description.value(QStringLiteral("type")).toString();
                            if (descriptionType == QLatin1String("boolean")) {
                                if (value == QLatin1String("true")) {
                                    value = i18nc("Boolean value", "yes");
                                } else {
                                    value = i18nc("Boolean value", "no");
                                }
                            } else if (descriptionType == QLatin1String("date")) {
                                const QDate date = QDate::fromString(value, Qt::ISODate);
                                value = QLocale().toString(date, QLocale::ShortFormat);
                            } else if (descriptionType == QLatin1String("time")) {
                                const QTime time = QTime::fromString(value, Qt::ISODate);
                                value = QLocale().toString(time);
                            } else if (descriptionType == QLatin1String("datetime")) {
                                const QDateTime dateTime = QDateTime::fromString(value, Qt::ISODate);
                                value = QLocale().toString(dateTime, QLocale::ShortFormat);
                            } else if (descriptionType == QLatin1String("url")) {
                                value = KStringHandler::tagUrls(value.toHtmlEscaped());
                                needToEscape = false;
                            }

                            break;
                        }
                    }
                }
                if (needToEscape) {
                    value = value.toHtmlEscaped();
                }
                customData += rowFmtStr1.arg(key).arg(value);
            }
        }
    }

    // Assemble all parts
    QString role = rawContact.title();
    if (role.isEmpty()) {
        role = rawContact.role();
    }
    if (role.isEmpty()) {
        role = rawContact.custom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Profession"));
    }

    QString strAddr = QStringLiteral(
                          "<div align=\"center\">"
                          "<table cellpadding=\"3\" cellspacing=\"1\">"
                          "<tr>"
                          "<td align=\"right\" valign=\"top\" width=\"30%\" rowspan=\"3\">"
                          "<img src=\"%1\" width=\"100\" vspace=\"1\">" // image
                          "</td>"
                          "<td colspan=\"2\" align=\"left\" width=\"70%\"><font size=\"+2\"><b>%2</b></font></td>" // name
                          "</tr>"
                          "<tr>"
                          "<td colspan=\"2\" align=\"left\" width=\"70%\">%3</td>"  // role
                          "</tr>"
                          "<tr>"
                          "<td colspan=\"2\" align=\"left\" width=\"70%\">%4</td>"  // organization
                          "</tr>")
                      .arg(QStringLiteral("contact_photo"))
                      .arg(rawContact.realName().toHtmlEscaped())
                      .arg(role.toHtmlEscaped())
                      .arg(rawContact.organization().toHtmlEscaped());

    strAddr.append(dynamicPart);
    strAddr.append(notes);
    strAddr.append(customData);
    strAddr.append(QLatin1String("</table>"));

#ifdef HAVE_PRISON
    if (d->displayQRcode) {
        KConfig config(QStringLiteral("akonadi_contactrc"));
        KConfigGroup group(&config, QStringLiteral("View"));
        if (group.readEntry("QRCodes", true)) {
            strAddr.append(QString::fromLatin1(
                               "<p align=\"center\">"
                               "<img src=\"%1\" vspace=\"1\">"
                               "<img src=\"%2\" vspace=\"1\">"
                               "</p>"
                           )
                           .arg(QStringLiteral("datamatrix"))
                           .arg(QStringLiteral("qrcode")));
        }
    }
#endif // HAVE_PRISON

    strAddr.append(QLatin1String("</div>\n"));

    if (form == EmbeddableForm) {
        return strAddr;
    }

    const QString document = QStringLiteral(
                                 "<html>"
                                 "<head>"
                                 " <style type=\"text/css\">"
                                 "  a {text-decoration:none; color:%1}"
                                 " </style>"
                                 "</head>"
                                 "<body text=\"%1\" bgcolor=\"%2\">" // text and background color
                                 "%3" // contact part
                                 "</body>"
                                 "</html>")
                             .arg(KColorScheme(QPalette::Active, KColorScheme::View).foreground().color().name())
                             .arg(KColorScheme(QPalette::Active, KColorScheme::View).background().color().name())
                             .arg(strAddr);

    return document;
}

void StandardContactFormatter::setDisplayQRCode(bool show)
{
    d->displayQRcode = show;
}

bool StandardContactFormatter::displayQRCode() const
{
    return d->displayQRcode;
}

