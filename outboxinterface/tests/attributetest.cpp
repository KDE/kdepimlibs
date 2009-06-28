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
#include <akonadi/kmime/addressattribute.h>

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

void AttributeTest::testSerialization()
{
  {
    QString from( "from@me.org" );
    QStringList to( "to1@me.org" );
    to << "to2@me.org";
    QStringList cc( "cc1@me.org" );
    cc << "cc2@me.org";
    QStringList bcc( "bcc1@me.org" );
    bcc << "bcc2@me.org";
    AddressAttribute *a = new AddressAttribute( from, to, cc, bcc );
    QByteArray data = a->serialized();
    delete a;
    a = new AddressAttribute;
    a->deserialize( data );
    QCOMPARE( from, a->from() );
    QCOMPARE( to, a->to() );
    QCOMPARE( cc, a->cc() );
    QCOMPARE( bcc, a->bcc() );
  }

  {
    DispatchModeAttribute::DispatchMode mode = DispatchModeAttribute::AfterDueDate;
    QDateTime date = QDateTime::currentDateTime();
    // The serializer does not keep track of milliseconds, so forget them.
    kDebug() << "ms" << date.toString( "z" );
    int ms = date.toString( "z" ).toInt();
    date = date.addMSecs( -ms );
    DispatchModeAttribute *a = new DispatchModeAttribute( mode, date );
    QByteArray data = a->serialized();
    delete a;
    a = new DispatchModeAttribute;
    a->deserialize( data );
    QCOMPARE( mode, a->dispatchMode() );
    QCOMPARE( date, a->dueDate() );
  }

  {
    QString msg( "The #!@$ing thing failed!" );
    ErrorAttribute *a = new ErrorAttribute( msg );
    QByteArray data = a->serialized();
    delete a;
    a = new ErrorAttribute;
    a->deserialize( data );
    QCOMPARE( msg, a->message() );
  }

  {
    SentBehaviourAttribute::SentBehaviour beh = SentBehaviourAttribute::MoveToCollection;
    Collection::Id id = 123456789012345ll;
    SentBehaviourAttribute *a = new SentBehaviourAttribute( beh, id );
    QByteArray data = a->serialized();
    delete a;
    a = new SentBehaviourAttribute;
    a->deserialize( data );
    QCOMPARE( beh, a->sentBehaviour() );
    QCOMPARE( id, a->moveToCollection() );
  }

  {
    int id = 3219;
    TransportAttribute *a = new TransportAttribute( id );
    QByteArray data = a->serialized();
    delete a;
    a = new TransportAttribute;
    a->deserialize( data );
    QCOMPARE( id, a->transportId() );
  }
}

QTEST_AKONADIMAIN( AttributeTest, NoGUI )

#include "attributetest.moc"
