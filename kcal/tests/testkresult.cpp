/*
  This file is part of the kcal library.
  Copyright (C) 2008 Allen Winter <winter@kde.org>

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
#include "testkresult.h"
#include "testkresult.moc"
#include <qtest_kde.h>

QTEST_KDEMAIN( KResultTest, NoGUI )

#include "kcal/kresult.h"
using namespace KCal;

void KResultTest::testValidity()
{
  KResult res1;
  QVERIFY( res1.isOk() == true );
  KResult *res2 = new KResult( KResult::Ok );
  QVERIFY( res1.isOk() == res2->isOk() );
}

void KResultTest::testCopy()
{
  KResult res1( KResult::ReadError, "Sorry read failure" );
  KResult res2( res1 );
  QVERIFY( res1.error() == res2.error() );
}
