/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2010-2012 Sérgio Martins <iamsergio@gmail.com>

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

#include "utils_p.h"
#include <kemailaddress.h>
#include <identitymanager.h>
#include <identity.h>
#include <kmime/kmime_header_parsing.h>

#include <KEMailSettings>
#include <collectiondialog.h>

#include <QWidget>
#include <QPointer>

using namespace Akonadi::CalendarUtils;

Akonadi::Collection
Akonadi::CalendarUtils::selectCollection(QWidget *parent,
        int &dialogCode,
        const QStringList &mimeTypes,
        const Akonadi::Collection &defaultCollection)
{
    QPointer<Akonadi::CollectionDialog> dlg(new Akonadi::CollectionDialog(parent));

    qDebug() << "selecting collections with mimeType in " << mimeTypes;

    dlg->changeCollectionDialogOptions(Akonadi::CollectionDialog::KeepTreeExpanded);
    dlg->setMimeTypeFilter(mimeTypes);
    dlg->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    if (defaultCollection.isValid()) {
        dlg->setDefaultCollection(defaultCollection);
    }
    Akonadi::Collection collection;

    // FIXME: don't use exec.
    dialogCode = dlg->exec();
    if (dialogCode == QDialog::Accepted) {
        collection = dlg->selectedCollection();

        if (!collection.isValid()) {
            qWarning() <<"An invalid collection was selected!";
        }
    }
    delete dlg;

    return collection;
}

QString Akonadi::CalendarUtils::fullName()
{
    KEMailSettings settings;
    QString tusername = settings.getSetting(KEMailSettings::RealName);

    // Quote the username as it might contain commas and other quotable chars.
    tusername = KEmailAddress::quoteNameIfNecessary(tusername);

    QString tname, temail;
    // ignore the return value from extractEmailAddressAndName() because
    // it will always be false since tusername does not contain "@domain".
    KEmailAddress::extractEmailAddressAndName(tusername, temail, tname);
    return tname;
}

QString Akonadi::CalendarUtils::email()
{
    KEMailSettings emailSettings;
    return emailSettings.getSetting(KEMailSettings::EmailAddress);
}

bool Akonadi::CalendarUtils::thatIsMe(const QString &_email)
{
    KIdentityManagement::IdentityManager identityManager(/*ro=*/ true);

    // NOTE: this method is called for every created agenda view item,
    // so we need to keep performance in mind

    /* identityManager()->thatIsMe() is quite expensive since it does parsing of
       _email in a way which is unnecessarily complex for what we can have here,
       so we do that ourselves. This makes sense since this

    if ( Akonadi::identityManager()->thatIsMe( _email ) ) {
      return true;
    }
    */

    // in case email contains a full name, strip it out.
    // the below is the simpler but slower version of the following code:
    // const QString email = KPIM::getEmailAddress( _email );
    const QByteArray tmp = _email.toUtf8();
    const char *cursor = tmp.constData();
    const char *end = tmp.data() + tmp.length();
    KMime::Types::Mailbox mbox;
    KMime::HeaderParsing::parseMailbox(cursor, end, mbox);
    const QString email = mbox.addrSpec().asString();

    KEMailSettings emailSettings;
    const QString myEmail = emailSettings.getSetting(KEMailSettings::EmailAddress);

    if (myEmail == email) {
        return true;
    }

    KIdentityManagement::IdentityManager::ConstIterator it;
    for (it = identityManager.begin();
            it != identityManager.end(); ++it) {
        if ((*it).matchesEmailAddress(email)) {
            return true;
        }
    }

    return false;
}

QStringList Akonadi::CalendarUtils::allEmails()
{
    KIdentityManagement::IdentityManager identityManager(/*ro=*/ true);
    // Grab emails from the email identities
    // Warning, this list could contain duplicates.
    return identityManager.allEmails();
}

KCalCore::Incidence::Ptr Akonadi::CalendarUtils::incidence(const Akonadi::Item &item)
{
    // With this try-catch block, we get a 2x performance improvement in retrieving the payload
    // since we don't call hasPayload()
    try {
        return item.payload<KCalCore::Incidence::Ptr>();
    } catch (Akonadi::PayloadException) {
        return KCalCore::Incidence::Ptr();
    }
}
