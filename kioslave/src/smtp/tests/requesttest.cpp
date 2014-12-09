/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "requesttest.h"
#include "../request.h"
#include <qtest.h>
#include <QUrl>
RequestTest::RequestTest(QObject *parent)
    : QObject(parent)
{

}

RequestTest::~RequestTest()
{

}

void RequestTest::shouldHaveDefaultValue()
{
    KioSMTP::Request request;
    QVERIFY(request.to().isEmpty());
    QVERIFY(request.cc().isEmpty());
    QVERIFY(request.bcc().isEmpty());
    QVERIFY(request.emitHeaders());
    QVERIFY(!request.is8BitBody());
    QVERIFY(request.profileName().isEmpty());
    QVERIFY(request.fromAddress().isEmpty());
    QVERIFY(request.heloHostname().isEmpty());
    QCOMPARE(request.size(), static_cast<unsigned int>(0));
}

void RequestTest::shouldParseRequest_data()
{
    QTest::addColumn<QUrl>("smtpurl");
    QTest::addColumn<QString>("to");
    QTest::addColumn<QString>("from");
    QTest::addColumn<QString>("cc");
    QTest::addColumn<QString>("bcc");
    QTest::addColumn<unsigned int>("size");
    QTest::newRow("correct url") <<  QUrl(QLatin1String("smtps://smtp.kde.org:465/send?headers=0&from=foo%40kde.org&to=foo%40kde.org&size=617"))
                                 << QString(QLatin1String("foo@kde.org"))
                                 << QString(QLatin1String("foo@kde.org"))
                                 << QString()
                                 << QString()
                                 << static_cast<unsigned int>(617);
}

void RequestTest::shouldParseRequest()
{
    QFETCH(QUrl, smtpurl);
    QFETCH(QString, to);
    QFETCH(QString, from);
    QFETCH(QString, cc);
    QFETCH(QString, bcc);
    QFETCH(unsigned int, size);

    KioSMTP::Request request = KioSMTP::Request::fromURL(smtpurl);
    QCOMPARE(request.to().join(QLatin1String(",")), to);
    QCOMPARE(request.cc().join(QLatin1String(",")), cc);
    QCOMPARE(request.fromAddress(), from);
    QCOMPARE(request.bcc().join(QLatin1String(",")), bcc);
    QCOMPARE(request.size(), size);
}

QTEST_MAIN(RequestTest)
