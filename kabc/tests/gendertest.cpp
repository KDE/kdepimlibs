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

#include "gendertest.h"
#include "gender.h"
#include <qtest_kde.h>

GenderTest::GenderTest(QObject *parent)
    : QObject(parent)
{

}

GenderTest::~GenderTest()
{

}

void GenderTest::shouldHaveDefaultValue()
{
    KABC::Gender gender;
    QVERIFY(!gender.isValid());
    QVERIFY(gender.gender().isEmpty());
    QVERIFY(gender.parameters().isEmpty());
}

void GenderTest::shouldAssignValue()
{
    const QString genderStr(QLatin1String("F"));
    QMap<QString, QStringList> params;
    params.insert(QLatin1String("Foo1"), QStringList()<< QLatin1String("bla1") <<QLatin1String("blo1"));
    params.insert(QLatin1String("Foo2"), QStringList()<< QLatin1String("bla2") <<QLatin1String("blo2"));
    KABC::Gender gender(genderStr);
    gender.setParameters(params);
    QVERIFY(gender.isValid());
    QVERIFY(!gender.gender().isEmpty());
    QCOMPARE(gender.gender(), genderStr);
    QVERIFY(!gender.parameters().isEmpty());
    QCOMPARE(gender.parameters(), params);
}

void GenderTest::shouldAssignExternal()
{
    KABC::Gender gender;
    const QString genderStr(QLatin1String("H"));
    gender.setGender(genderStr);
    QVERIFY(gender.isValid());
    QVERIFY(!gender.gender().isEmpty());
    QCOMPARE(gender.gender(), genderStr);
}

void GenderTest::shouldSerialized()
{
    KABC::Gender gender;
    KABC::Gender result;
    const QString genderStr(QLatin1String("H"));
    gender.setGender(genderStr);
    QMap<QString, QStringList> params;
    params.insert(QLatin1String("Foo1"), QStringList()<< QLatin1String("bla1") <<QLatin1String("blo1"));
    params.insert(QLatin1String("Foo2"), QStringList()<< QLatin1String("bla2") <<QLatin1String("blo2"));
    gender.setParameters(params);


    QByteArray data;
    QDataStream s( &data, QIODevice::WriteOnly );
    s << gender;

    QDataStream t( &data, QIODevice::ReadOnly );
    t >> result;

    QVERIFY( gender == result );
}

void GenderTest::shouldEqualGender()
{
    KABC::Gender gender;
    KABC::Gender result;
    const QString genderStr(QLatin1String("H"));
    gender.setGender(genderStr);
    QMap<QString, QStringList> params;
    params.insert(QLatin1String("Foo1"), QStringList()<< QLatin1String("bla1") <<QLatin1String("blo1"));
    params.insert(QLatin1String("Foo2"), QStringList()<< QLatin1String("bla2") <<QLatin1String("blo2"));
    gender.setParameters(params);

    result = gender;
    QVERIFY( gender == result );
}

QTEST_KDEMAIN(GenderTest, NoGUI)
