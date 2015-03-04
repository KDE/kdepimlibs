/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "messagetest.h"
#include <qtest.h>
#include <QFile>
#include <item.h>
#include <messageflags.h>
using namespace KMime;

QTEST_MAIN(MessageTest)


void MessageTest::testCopyFlags()
{
    {
        KMime::Message::Ptr msg = readAndParseMail(QLatin1String("x-pkcs7.mbox"));

        Akonadi::Item item;
        Akonadi::MessageFlags::copyMessageFlags(*msg, item);

        QVERIFY(item.hasFlag(Akonadi::MessageFlags::Signed) == false);
        QVERIFY(item.hasFlag(Akonadi::MessageFlags::Encrypted) == true);
        QVERIFY(item.hasFlag(Akonadi::MessageFlags::HasInvitation) == false);
        QVERIFY(item.hasFlag(Akonadi::MessageFlags::HasAttachment) == false);
    }

    {
        KMime::Message::Ptr msg = readAndParseMail(QLatin1String("signed.mbox"));

        Akonadi::Item item;
        Akonadi::MessageFlags::copyMessageFlags(*msg, item);

        QVERIFY(item.hasFlag(Akonadi::MessageFlags::Signed) == true);
        QVERIFY(item.hasFlag(Akonadi::MessageFlags::Encrypted) == false);
        QVERIFY(item.hasFlag(Akonadi::MessageFlags::HasInvitation) == true);
        QVERIFY(item.hasFlag(Akonadi::MessageFlags::HasAttachment) == true);
    }
}


KMime::Message::Ptr MessageTest::readAndParseMail(const QString &mailFile) const
{
    QFile file(QLatin1String(TEST_DATA_DIR) + QLatin1String("/mails/") + mailFile);
    const bool ok = file.open(QIODevice::ReadOnly);
    if (!ok) {
        qWarning() << file.fileName() << "not found";
    }
    Q_ASSERT(ok);
    const QByteArray data = KMime::CRLFtoLF(file.readAll());
    Q_ASSERT(!data.isEmpty());
    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(data);
    msg->parse();
    return msg;
}

