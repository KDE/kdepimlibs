/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "attributetest.h"

#include <akonadi/attributefactory.h>
#include <akonadi/qtest_akonadi.h>

#include <outboxinterface/addressattribute.h>
#include <outboxinterface/dispatchmodeattribute.h>
#include <outboxinterface/errorattribute.h>
#include <outboxinterface/sentbehaviourattribute.h>
#include <outboxinterface/transportattribute.h>

using namespace Akonadi;
using namespace OutboxInterface;

void AttributeTest::initTestCase()
{
}

void AttributeTest::testRegistrar()
{
  // The attributes should have been registered without any effort on our part.
  {
    Attribute *a = AttributeFactory::createAttribute( "AddressAttribute" );
    QVERIFY( dynamic_cast<AddressAttribute*>( a ) );
  }

  {
    Attribute *a = AttributeFactory::createAttribute( "DispatchModeAttribute" );
    QVERIFY( dynamic_cast<DispatchModeAttribute*>( a ) );
  }

  {
    Attribute *a = AttributeFactory::createAttribute( "ErrorAttribute" );
    QVERIFY( dynamic_cast<ErrorAttribute*>( a ) );
  }

  {
    Attribute *a = AttributeFactory::createAttribute( "SentBehaviourAttribute" );
    QVERIFY( dynamic_cast<SentBehaviourAttribute*>( a ) );
  }

  {
    Attribute *a = AttributeFactory::createAttribute( "TransportAttribute" );
    QVERIFY( dynamic_cast<TransportAttribute*>( a ) );
  }
}

QTEST_AKONADIMAIN( AttributeTest, NoGUI )

#include "attributetest.moc"
