/*
    This file is part of libkabc.
    Copyright (c) 2015 Laurent Montel <montel@kde.org>

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

#include "calendarurltest.h"

#include "emailtest.h"
#include "kabc/calendarurl.h"
#include <qtest_kde.h>

CalendarUrlTest::CalendarUrlTest(QObject *parent)
    : QObject(parent)
{

}

CalendarUrlTest::~CalendarUrlTest()
{

}

void CalendarUrlTest::shouldHaveDefaultValue()
{
    KABC::CalendarUrl calendarUrl;
    QVERIFY(!calendarUrl.isValid());
    QVERIFY(calendarUrl.url().isEmpty());
    QVERIFY(calendarUrl.parameters().isEmpty());
}

void CalendarUrlTest::shouldAssignValue()
{
    const QString mail(QLatin1String("foo@kde.org"));
    QMap<QString, QStringList> params;
    params.insert(QLatin1String("Foo1"), QStringList()<< QLatin1String("bla1") <<QLatin1String("blo1"));
    params.insert(QLatin1String("Foo2"), QStringList()<< QLatin1String("bla2") <<QLatin1String("blo2"));
    KABC::CalendarUrl calendarUrl(mail);
    calendarUrl.setParameters(params);
    QVERIFY(calendarUrl.isValid());
    QVERIFY(!calendarUrl.mail().isEmpty());
    QCOMPARE(calendarUrl.mail(), mail);
    QVERIFY(!calendarUrl.parameters().isEmpty());
    QCOMPARE(calendarUrl.parameters(), params);
}

void CalendarUrlTest::shouldAssignExternal()
{
    KABC::CalendarUrl calendarUrl;
    const QString mail(QLatin1String("foo@kde.org"));
    calendarUrl.setEmail(mail);
    QVERIFY(calendarUrl.isValid());
    QVERIFY(!calendarUrl.url().isEmpty());
    QCOMPARE(calendarUrl.url(), mail);
}

void CalendarUrlTest::shouldSerialized()
{
    KABC::CalendarUrl calendarUrl;
    KABC::CalendarUrl result;
    const QString mail(QLatin1String("foo@kde.org"));
    calendarUrl.setEmail(mail);
    QMap<QString, QStringList> params;
    params.insert(QLatin1String("Foo1"), QStringList()<< QLatin1String("bla1") <<QLatin1String("blo1"));
    params.insert(QLatin1String("Foo2"), QStringList()<< QLatin1String("bla2") <<QLatin1String("blo2"));
    calendarUrl.setParameters(params);


    QByteArray data;
    QDataStream s( &data, QIODevice::WriteOnly );
    s << calendarUrl;

    QDataStream t( &data, QIODevice::ReadOnly );
    t >> result;

    QVERIFY( calendarUrl == result );
}

void CalendarUrlTest::shouldEqualEmail()
{
    KABC::CalendarUrl calendarUrl;
    KABC::CalendarUrl result;
    const QString mail(QLatin1String("foo@kde.org"));
    calendarUrl.setEmail(mail);
    QMap<QString, QStringList> params;
    params.insert(QLatin1String("Foo1"), QStringList()<< QLatin1String("bla1") <<QLatin1String("blo1"));
    params.insert(QLatin1String("Foo2"), QStringList()<< QLatin1String("bla2") <<QLatin1String("blo2"));
    calendarUrl.setParameters(params);

    result = calendarUrl;
    QVERIFY( calendarUrl == result );
}

QTEST_KDEMAIN(CalendarUrlTest, NoGUI)

