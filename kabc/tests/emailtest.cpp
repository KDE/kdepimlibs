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
#include "emailtest.h"
#include "kabc/email.h"
#include <qtest_kde.h>

EmailTest::EmailTest(QObject *parent)
    : QObject(parent)
{

}

EmailTest::~EmailTest()
{

}

void EmailTest::shouldHaveDefaultValue()
{
    KABC::Email email;
    QVERIFY(!email.isValid());
    QVERIFY(email.mail().isEmpty());
    QVERIFY(email.parameters().isEmpty());
    QVERIFY(!email.preferred());
}

void EmailTest::shouldAssignValue()
{
    const QString mail(QLatin1String("foo@kde.org"));
    const bool preferred = true;
    KABC::Email email(mail, preferred);
    QVERIFY(email.isValid());
    QVERIFY(!email.mail().isEmpty());
    QCOMPARE(email.mail(), mail);
    QVERIFY(email.preferred());
}

void EmailTest::shouldAssignExternal()
{
    KABC::Email email;
    const QString mail(QLatin1String("foo@kde.org"));
    const bool preferred = true;
    email.setEmail(mail);
    email.setPreferred(preferred);
    QVERIFY(email.isValid());
    QVERIFY(!email.mail().isEmpty());
    QCOMPARE(email.mail(), mail);
    QVERIFY(email.preferred());
}

void EmailTest::shouldSerialized()
{
    KABC::Email email;
    KABC::Email result;
    const QString mail(QLatin1String("foo@kde.org"));
    const bool preferred = true;
    email.setEmail(mail);
    email.setPreferred(preferred);

    QByteArray data;
    QDataStream s( &data, QIODevice::WriteOnly );
    s << email;

    QDataStream t( &data, QIODevice::ReadOnly );
    t >> result;

    QVERIFY( email == result );
}

QTEST_KDEMAIN(EmailTest, NoGUI)
