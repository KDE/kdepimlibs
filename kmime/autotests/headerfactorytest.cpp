/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "headerfactorytest.h"

//#include <typeinfo>

#include <QDebug>
#include <qtest.h>

#include <kmime_headerfactory_p.h>
#include <kmime_headers.h>

using namespace KMime;
using namespace KMime::Headers;
//using namespace KMime::Headers::Generics;

QTEST_MAIN( HeaderFactoryTest )

// This cannot be defined in a function, because the template code
// in HeaderFactory::registerHeader() needs it.
class MyXHeader : public ContentType
{
  public:
    const char *type() const
    {
      return "X-My-Content-Type";
    }

    virtual Base *clone() const
    {
      MyXHeader *ret = new MyXHeader;
      ret->from7BitString( as7BitString( false ) );
      return ret;
    }
};

template <typename T>
bool isHeaderRegistered()
{
  T dummy;
  Base *h = HeaderFactory::self()->createHeader( dummy.type() );
  if ( h ) {
    delete h;
    return true;
  }
  return false;
}

void HeaderFactoryTest::initTestCase()
{
  HeaderFactory::self();
}

void HeaderFactoryTest::testBuiltInHeaders()
{
  // Abstract headers have pure virtual methods.
  // Generic headers have an empty type().
  // All other built-in headers are supposed to be registered.

  //QVERIFY( isHeaderRegistered<Base>() ); // Abstract.
  //QVERIFY( isHeaderRegistered<Unstructured>() ); // Abstract.
  //QVERIFY( isHeaderRegistered<Structured>() ); // Abstract.
  //QVERIFY( isHeaderRegistered<Address>() ); // Abstract.
  //QVERIFY( isHeaderRegistered<MailboxList>() ); // Generic.
  //QVERIFY( isHeaderRegistered<SingleMailbox>() ); // Generic.
  //QVERIFY( isHeaderRegistered<AddressList>() ); // Generic.
  //QVERIFY( isHeaderRegistered<Ident>() ); // Generic.
  //QVERIFY( isHeaderRegistered<SingleIdent>() ); // Generic.
  //QVERIFY( isHeaderRegistered<Token>() ); // Generic.
  //QVERIFY( isHeaderRegistered<PhraseList>() ); // Generic.
  //QVERIFY( isHeaderRegistered<DotAtom>() ); // Generic.
  //QVERIFY( isHeaderRegistered<Parametrized>() ); // Generic.
  QVERIFY( isHeaderRegistered<ReturnPath>() );
  QVERIFY( isHeaderRegistered<From>() );
  QVERIFY( isHeaderRegistered<Sender>() );
  QVERIFY( isHeaderRegistered<To>() );
  QVERIFY( isHeaderRegistered<Cc>() );
  QVERIFY( isHeaderRegistered<Bcc>() );
  QVERIFY( isHeaderRegistered<ReplyTo>() );
  QVERIFY( isHeaderRegistered<MailCopiesTo>() );
  QVERIFY( isHeaderRegistered<ContentTransferEncoding>() );
  QVERIFY( isHeaderRegistered<Keywords>() );
  QVERIFY( isHeaderRegistered<MIMEVersion>() );
  QVERIFY( isHeaderRegistered<MessageID>() );
  QVERIFY( isHeaderRegistered<ContentID>() );
  QVERIFY( isHeaderRegistered<Supersedes>() );
  QVERIFY( isHeaderRegistered<InReplyTo>() );
  QVERIFY( isHeaderRegistered<References>() );
  QVERIFY( isHeaderRegistered<ContentType>() );
  QVERIFY( isHeaderRegistered<ContentDisposition>() );
  //QVERIFY( isHeaderRegistered<Generic>() ); // Changeable type().
  QVERIFY( isHeaderRegistered<Subject>() );
  QVERIFY( isHeaderRegistered<Organization>() );
  QVERIFY( isHeaderRegistered<ContentDescription>() );
  QVERIFY( isHeaderRegistered<ContentLocation>() );
  QVERIFY( isHeaderRegistered<Control>() );
  QVERIFY( isHeaderRegistered<Date>() );
  QVERIFY( isHeaderRegistered<Newsgroups>() );
  QVERIFY( isHeaderRegistered<FollowUpTo>() );
  QVERIFY( isHeaderRegistered<Lines>() );
  QVERIFY( isHeaderRegistered<UserAgent>() );
}

void HeaderFactoryTest::testCustomHeaders()
{
  MyXHeader dummy;

  // Before registration:
  {
    Base *bh = HeaderFactory::self()->createHeader( dummy.type() );
    QVERIFY( bh == 0 );
  }

  // Register:
  {
    bool ret = HeaderFactory::self()->registerHeader<MyXHeader>();
    QVERIFY( ret == true );
  }

  // After registration:
  {
    Base *bh = HeaderFactory::self()->createHeader( dummy.type() );
    MyXHeader *h = dynamic_cast<MyXHeader*>( bh );
    QVERIFY( h );
  }

  // Should be case-insensitive.
  {
    Base *bh = HeaderFactory::self()->createHeader( "x-mY-CoNtEnT-tYpE" );
    MyXHeader *h = dynamic_cast<MyXHeader*>( bh );
    QVERIFY( h );
  }
}

void HeaderFactoryTest::testErrors()
{
  // Cannot register a generic (empty-type()) header:
  {
    bool ret = HeaderFactory::self()->registerHeader<Generic>();
    QVERIFY( ret == false );
  }

  // Repeated registration should fail.
  MyXHeader dummy;
  bool ret = HeaderFactory::self()->registerHeader<MyXHeader>();
  QVERIFY( ret == false );
}

