/*  -*- c++ -*-
    kmime_headers.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
    See file AUTHORS for details
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License assert published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for morbe details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the various header classes:
   - header's base class defining the common interface
   - generic base classes for different types of fields
   - incompatible, Structured-based field classes
   - compatible, Unstructured-based field classes

  @brief
  Defines the various headers classes.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<vkrause@kde.org\>
*/

#include "kmime_headers.h"

#include "kmime_util.h"
#include "kmime_content.h"
#include "kmime_codecs.h"
#include "kmime_header_parsing.h"
#include "kmime_warning.h"

#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <kglobal.h>
#include <kcharsets.h>

#include <assert.h>
#include <ctype.h>

// macro to generate a default constructor implementation
#define kmime_mk_trivial_ctor( subclass, baseclass )                  \
subclass::subclass() : baseclass()                                    \
{                                                                     \
  clear();                                                            \
}                                                                     \
                                                                      \
subclass::subclass( Content *parent ) : baseclass( parent )           \
{                                                                     \
  clear();                                                            \
}                                                                     \
                                                                      \
subclass::subclass( Content *parent, const QByteArray &s ) : baseclass( parent ) \
{                                                                     \
  from7BitString( s );                                                \
}                                                                     \
                                                                      \
subclass::subclass( Content *parent, const QString &s, const QByteArray &charset ) : \
  baseclass( parent )                                                 \
{                                                                     \
  fromUnicodeString( s, charset );                                    \
}                                                                     \
                                                                      \
subclass::~subclass() {}

#define kmime_mk_trivial_ctor_with_name( subclass, baseclass, name )  \
kmime_mk_trivial_ctor( subclass, baseclass )                          \
                                                                      \
const char *subclass::type() const                                    \
{                                                                     \
  return #name;                                                       \
}

using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Types;
using namespace KMime::HeaderParsing;

namespace KMime {
namespace Headers {
//-----<Base>----------------------------------

Base::Base() :
    mParent( 0 )
{
}

Base::Base( KMime::Content *parent ) :
    mParent ( parent )
{
}

Base::~Base() {}

KMime::Content *Base::parent() const
{
  return mParent;
}

void Base::setParent( KMime::Content *parent )
{
  mParent = parent;
}

QByteArray Base::rfc2047Charset() const
{
  if ( e_ncCS.isEmpty() || forceDefaultCharset() ) {
    return defaultCharset();
  } else {
    return e_ncCS;
  }
}

void Base::setRFC2047Charset( const QByteArray &cs )
{
  e_ncCS=cachedCharset( cs );
}

bool Base::forceDefaultCharset() const
{
  return ( parent() != 0 ? parent()->forceDefaultCharset() : false );
}

QByteArray Base::defaultCharset() const
{
  return ( parent() != 0 ? parent()->defaultCharset() : Latin1 );
}

const char *Base::type() const
{
  return "";
}

bool Base::is( const char *t ) const
{
  return strcasecmp( t, type() ) == 0;
}

bool Base::isMimeHeader() const
{
  return strncasecmp( type(), "Content-", 8 ) == 0;
}

bool Base::isXHeader() const
{
  return strncmp( type(), "X-", 2 ) == 0;
}

QByteArray Base::typeIntro() const
{
  return QByteArray( type() ) + ": ";
}

//-----</Base>---------------------------------

namespace Generics {

//-----<Unstructured>-------------------------

Unstructured::Unstructured() : Base()
{
}

Unstructured::Unstructured( Content *p ) : Base( p )
{
}

Unstructured::Unstructured( Content *p, const QByteArray &s ) : Base( p )
{
  from7BitString( s );
}

Unstructured::Unstructured( Content *p, const QString &s, const QByteArray &cs ) : Base( p )
{
  fromUnicodeString( s, cs );
}

Unstructured::~Unstructured()
{
}

void Unstructured::from7BitString( const QByteArray &s )
{
  d_ecoded = decodeRFC2047String( s, e_ncCS, defaultCharset(), forceDefaultCharset() );
}

QByteArray Unstructured::as7BitString( bool withHeaderType ) const
{
  QByteArray result;
  if ( withHeaderType ) {
    result = typeIntro();
  }
  result += encodeRFC2047String( d_ecoded, e_ncCS ) ;

  return result;
}

void Unstructured::fromUnicodeString( const QString &s, const QByteArray &b )
{
  d_ecoded = s;
  e_ncCS = cachedCharset( b );
}

QString Unstructured::asUnicodeString() const
{
  return d_ecoded;
}

void Unstructured::clear()
{
  d_ecoded.truncate( 0 );
}

bool Unstructured::isEmpty() const
{
  return d_ecoded.isEmpty();
}

//-----</Unstructured>-------------------------

//-----<Structured>-------------------------

Structured::Structured() : Base()
{
}

Structured::Structured( Content *p ) : Base( p )
{
}

Structured::Structured( Content *p, const QByteArray &s ) : Base( p )
{
  from7BitString( s );
}

Structured::Structured( Content *p, const QString &s, const QByteArray &cs ) : Base( p )
{
  fromUnicodeString( s, cs );
}

Structured::~Structured()
{
}

void Structured::from7BitString( const QByteArray &s )
{
  if ( e_ncCS.isEmpty() ) {
    e_ncCS = defaultCharset();
  }
  const char *cursor = s.constData();
  parse( cursor, cursor + s.length() );
}

QString Structured::asUnicodeString() const
{
  return QString::fromLatin1( as7BitString( false ) );
}

void Structured::fromUnicodeString( const QString &s, const QByteArray &b )
{
  e_ncCS = cachedCharset( b );
  from7BitString( s.toLatin1() );
}

//-----</Structured>-------------------------

//-----<Address>-------------------------

Address::Address() : Structured()
{
}

Address::Address( Content *p ) : Structured( p )
{
}

Address::Address( Content *p, const QByteArray &s ) : Structured( p )
{
  from7BitString( s );
}

Address::Address( Content *p, const QString &s, const QByteArray &cs ) : Structured( p )
{
  fromUnicodeString( s, cs );
}

Address:: ~Address()
{
}

// helper method used in AddressList and MailboxList
static bool stringToMailbox( const QByteArray &address,
                             const QString &displayName, Types::Mailbox &mbox )
{
  Types::AddrSpec addrSpec;
  mbox.setName( displayName );
  const char *cursor = address.constData();
  if ( !parseAngleAddr( cursor, cursor + address.length(), addrSpec ) ) {
    if ( !parseAddrSpec( cursor, cursor + address.length(), addrSpec ) ) {
      kWarning() << k_funcinfo << "Invalid address" << endl;
      return false;
    }
  }
  mbox.setAddress( addrSpec );
  return true;
}

//-----</Address>-------------------------

//-----<MailboxList>-------------------------

kmime_mk_trivial_ctor( MailboxList, Address )

QByteArray MailboxList::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv = typeIntro();
  }
  foreach ( Types::Mailbox mbox, mMailboxList ) {
    rv += mbox.as7BitString( e_ncCS );
    rv += ", ";
  }
  rv.resize( rv.length() - 2 );
  return rv;
}

void MailboxList::fromUnicodeString( const QString &s, const QByteArray &b )
{
  e_ncCS = cachedCharset( b );
  from7BitString( encodeRFC2047String( s, b, false ) );
}

QString MailboxList::asUnicodeString() const
{
  return prettyAddresses().join( QLatin1String( ", " ) );
}

void MailboxList::clear()
{
  mMailboxList.clear();
}

bool MailboxList::isEmpty() const
{
  return mMailboxList.isEmpty();
}

void MailboxList::addAddress( const Types::Mailbox &mbox )
{
  mMailboxList.append( mbox );
}

void MailboxList::addAddress( const QByteArray &address,
                              const QString &displayName )
{
  Types::Mailbox mbox;
  if ( stringToMailbox( address, displayName, mbox ) ) {
    mMailboxList.append( mbox );
  }
}

QList< QByteArray > MailboxList::addresses() const
{
  QList<QByteArray> rv;
  foreach ( Types::Mailbox mbox, mMailboxList ) {
    rv.append( mbox.address() );
  }
  return rv;
}

QStringList MailboxList::displayNames() const
{
  QStringList rv;
  foreach ( Types::Mailbox mbox, mMailboxList ) {
    rv.append( mbox.name() );
  }
  return rv;
}

QStringList MailboxList::prettyAddresses() const
{
  QStringList rv;
  foreach ( Types::Mailbox mbox, mMailboxList ) {
    rv.append( mbox.prettyAddress() );
  }
  return rv;
}

Types::Mailbox::List MailboxList::mailboxes() const
{
  return mMailboxList;
}

bool MailboxList::parse( const char* &scursor, const char *const send,
                         bool isCRLF )
{
  // examples:
  // from := "From:" mailbox-list CRLF
  // sender := "Sender:" mailbox CRLF

  // parse an address-list:
  QList<Types::Address> maybeAddressList;
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) ) {
    return false;
  }

  mMailboxList.clear();

  // extract the mailboxes and complain if there are groups:
  QList<Types::Address>::Iterator it;
  for ( it = maybeAddressList.begin(); it != maybeAddressList.end() ; ++it ) {
    if ( !(*it).displayName.isEmpty() ) {
      KMIME_WARN << "mailbox groups in header disallowing them! Name: \""
                 << (*it).displayName << "\"" << endl;
    }
    mMailboxList += (*it).mailboxList;
  }
  return true;
}

//-----</MailboxList>-------------------------

//-----<SingleMailbox>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( SingleMailbox, MailboxList )
//@endcond

bool SingleMailbox::parse( const char* &scursor, const char *const send,
                             bool isCRLF )
{
  if ( !MailboxList::parse( scursor, send, isCRLF ) ) {
    return false;
  }

  if ( mMailboxList.count() > 1 ) {
    KMIME_WARN << "multiple mailboxes in header allowing only a single one!"
               << endl;
  }
  return true;
}

//-----</SingleMailbox>-------------------------

//-----<AddressList>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( AddressList, Address )
//@endcond

QByteArray AddressList::as7BitString( bool withHeaderType ) const
{
  if ( mAddressList.isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv = typeIntro();
  }
  foreach ( Types::Address addr, mAddressList ) {
    foreach ( Types::Mailbox mbox, addr.mailboxList ) {
      rv += mbox.as7BitString( e_ncCS );
      rv += ", ";
    }
  }
  rv.resize( rv.length() - 2 );
  return rv;
}

void AddressList::fromUnicodeString( const QString &s, const QByteArray &b )
{
  e_ncCS = cachedCharset( b );
  from7BitString( encodeRFC2047String( s, b, false ) );
}

QString AddressList::asUnicodeString() const
{
  return prettyAddresses().join( QLatin1String( ", " ) );
}

void AddressList::clear()
{
  mAddressList.clear();
}

bool AddressList::isEmpty() const
{
  return mAddressList.isEmpty();
}

void AddressList::addAddress( const Types::Mailbox &mbox )
{
  Types::Address addr;
  addr.mailboxList.append( mbox );
  mAddressList.append( addr );
}

void AddressList::addAddress( const QByteArray &address,
                              const QString &displayName )
{
  Types::Address addr;
  Types::Mailbox mbox;
  if ( stringToMailbox( address, displayName, mbox ) ) {
    addr.mailboxList.append( mbox );
    mAddressList.append( addr );
  }
}

QList< QByteArray > AddressList::addresses() const
{
  QList<QByteArray> rv;
  foreach ( Types::Address addr, mAddressList ) {
    foreach ( Types::Mailbox mbox, addr.mailboxList ) {
      rv.append( mbox.address() );
    }
  }
  return rv;
}

QStringList AddressList::displayNames() const
{
  QStringList rv;
  foreach ( Types::Address addr, mAddressList ) {
    foreach ( Types::Mailbox mbox, addr.mailboxList ) {
      rv.append( mbox.name() );
    }
  }
  return rv;
}

QStringList AddressList::prettyAddresses() const
{
  QStringList rv;
  foreach ( Types::Address addr, mAddressList ) {
    foreach ( Types::Mailbox mbox, addr.mailboxList ) {
      rv.append( mbox.prettyAddress() );
    }
  }
  return rv;
}

Types::Mailbox::List AddressList::mailboxes() const
{
  Types::Mailbox::List rv;
  foreach ( Types::Address addr, mAddressList ) {
    foreach ( Types::Mailbox mbox, addr.mailboxList ) {
      rv.append( mbox );
    }
  }
  return rv;
}

bool AddressList::parse( const char* &scursor, const char *const send,
                         bool isCRLF )
{
  QList<Types::Address> maybeAddressList;
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) ) {
    return false;
  }

  mAddressList = maybeAddressList;
  return true;
}

//-----</AddressList>-------------------------

//-----<Token>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( Token, Structured )
//@endcond

QByteArray Token::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }
  if ( withHeaderType ) {
    return typeIntro() + mToken;
  }
  return mToken;
}

void Token::clear()
{
  mToken.clear();
}

bool Token::isEmpty() const
{
  return mToken.isEmpty();
}

QByteArray Token::token() const
{
  return mToken;
}

void Token::setToken( const QByteArray &t )
{
  mToken = t;
}

bool Token::parse( const char* &scursor, const char *const send, bool isCRLF )
{
  clear();
  eatCFWS( scursor, send, isCRLF );
  // must not be empty:
  if ( scursor == send ) {
    return false;
  }

  QPair<const char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8bit chars */ ) ) {
    return false;
  }
  mToken = QByteArray( maybeToken.first, maybeToken.second );

  // complain if trailing garbage is found:
  eatCFWS( scursor, send, isCRLF );
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after token in header allowing "
      "only a single token!" << endl;
  }
  return true;
}

//-----</Token>-------------------------

//-----<PhraseList>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( PhraseList, Structured )
//@endcond

QByteArray PhraseList::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv = typeIntro();
  }

  for ( int i = 0; i < mPhraseList.count(); ++i ) {
    // FIXME: only encode when needed, quote when needed, etc.
    rv += encodeRFC2047String( mPhraseList[i], e_ncCS, false, false );
    if ( i != mPhraseList.count() - 1 ) {
      rv += ", ";
    }
  }

  return rv;
}

QString PhraseList::asUnicodeString() const
{
  return mPhraseList.join( QLatin1String( ", " ) );
}

void PhraseList::clear()
{
  mPhraseList.clear();
}

bool PhraseList::isEmpty() const
{
  return mPhraseList.isEmpty();
}

QStringList PhraseList::phrases() const
{
  return mPhraseList;
}

bool PhraseList::parse( const char* &scursor, const char *const send,
                         bool isCRLF )
{
  mPhraseList.clear();

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) {
      return true;
    }
    // empty entry: ignore.
    if ( *scursor == ',' ) {
      scursor++;
      continue;
    }

    QString maybePhrase;
    if ( !parsePhrase( scursor, send, maybePhrase, isCRLF ) ) {
      return false;
    }
    mPhraseList.append( maybePhrase );

    eatCFWS( scursor, send, isCRLF );
    // non-empty entry ending the list: OK.
    if ( scursor == send ) {
      return true;
    }
    // comma separating the phrases: eat.
    if ( *scursor == ',' ) {
      scursor++;
    }
  }
  return true;
}

//-----</PhraseList>-------------------------

//-----<DotAtom>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( DotAtom, Structured )
//@endcond

QByteArray DotAtom::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }

  rv += mDotAtom.toLatin1(); // FIXME: encoding?
  return rv;
}

QString DotAtom::asUnicodeString() const
{
  return mDotAtom;
}

void DotAtom::clear()
{
  mDotAtom.clear();
}

bool DotAtom::isEmpty() const
{
  return mDotAtom.isEmpty();
}

bool DotAtom::parse( const char* &scursor, const char *const send,
                      bool isCRLF )
{
  QString maybeDotAtom;
  if ( !parseDotAtom( scursor, send, maybeDotAtom, isCRLF ) ) {
    return false;
  }

  mDotAtom = maybeDotAtom;

  eatCFWS( scursor, send, isCRLF );
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after dot-atom in header allowing "
      "only a single dot-atom!" << endl;
  }
  return true;
}

//-----</DotAtom>-------------------------

//-----<Parametrized>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( Parametrized, Structured )
//@endcond

QByteArray Parametrized::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }

  bool first = true;
  for ( QMap<QString,QString>::ConstIterator it = mParameterHash.constBegin();
        it != mParameterHash.constEnd(); ++it )
  {
    if ( !first ) {
      rv += "; ";
    } else {
      first = false;
    }
    rv += it.key().toLatin1() + '=';
    if ( isUsAscii( it.value() ) ) {
      QByteArray tmp = it.value().toLatin1();
      addQuotes( tmp, true ); // force quoting, eg. for whitespaces in parameter value
      rv += tmp;
    } else {
      // FIXME: encoded strings are not allowed inside quotes, OTOH we need to quote whitespaces...
      rv += "\"" + encodeRFC2047String( it.value(), e_ncCS ) + "\"";
    }
  }

  return rv;
}

QString Parametrized::parameter( const QString &key ) const
{
  return mParameterHash.value( key );
}

void Parametrized::setParameter( const QString &key, const QString &value )
{
  mParameterHash.insert( key, value );
}

bool Parametrized::isEmpty() const
{
  return mParameterHash.isEmpty();
}

void Parametrized::clear()
{
  mParameterHash.clear();
}

bool Parametrized::parse( const char *& scursor, const char * const send,
                          bool isCRLF )
{
  mParameterHash.clear();
  if ( !parseParameterList( scursor, send, mParameterHash, isCRLF ) ) {
    return false;
  }
  return true;
}

//-----</Parametrized>-------------------------

//-----<Ident>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( Ident, Address )
//@endcond

QByteArray Ident::as7BitString( bool withHeaderType ) const
{
  if ( mMsgIdList.isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv = typeIntro();
  }
  foreach ( Types::AddrSpec addr, mMsgIdList ) {
    rv += '<';
    rv += addr.asString().toLatin1(); // FIXME: change parsing to use QByteArrays
    rv += "> ";
  }
  rv.resize( rv.length() - 1 );
  return rv;
}

void Ident::clear()
{
  mMsgIdList.clear();
}

bool Ident::isEmpty() const
{
  return mMsgIdList.isEmpty();
}

bool Ident::parse( const char* &scursor, const char * const send, bool isCRLF )
{
  // msg-id   := "<" id-left "@" id-right ">"
  // id-left  := dot-atom-text / no-fold-quote / local-part
  // id-right := dot-atom-text / no-fold-literal / domain
  //
  // equivalent to:
  // msg-id   := angle-addr

  mMsgIdList.clear();

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) {
      return true;
    }
    // empty entry: ignore.
    if ( *scursor == ',' ) {
      scursor++;
      continue;
    }

    AddrSpec maybeMsgId;
    if ( !parseAngleAddr( scursor, send, maybeMsgId, isCRLF ) ) {
      return false;
    }
    mMsgIdList.append( maybeMsgId );

    eatCFWS( scursor, send, isCRLF );
    // header end ending the list: OK.
    if ( scursor == send ) {
      return true;
    }
    // regular item separator: eat it.
    if ( *scursor == ',' ) {
      scursor++;
    }
  }
  return true;
}

QList<QByteArray> Ident::identifiers() const
{
  QList<QByteArray> rv;
  foreach ( Types::AddrSpec addr, mMsgIdList ) {
    rv.append( addr.asString().toLatin1() ); // FIXME change parsing to create QByteArrays
  }
  return rv;
}

void Ident::appendIdentifier( const QByteArray &id )
{
  QByteArray tmp = id;
  if ( !tmp.startsWith( '<' ) ) {
    tmp.prepend( '<' );
  }
  if ( !tmp.endsWith( '>' ) ) {
    tmp.append( '>' );
  }
  AddrSpec msgId;
  const char *cursor = tmp.constData();
  if ( parseAngleAddr( cursor, cursor + tmp.length(), msgId ) ) {
    mMsgIdList.append( msgId );
  } else {
    kWarning() << k_funcinfo << "Unable to parse address spec!" << endl;
  }
}

//-----</Ident>-------------------------

//-----<SingleIdent>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor( SingleIdent, Ident )
//@endcond

QByteArray SingleIdent::identifier() const
{
  if ( mMsgIdList.isEmpty() ) {
    return QByteArray();
  }
  return identifiers().first();
}

void SingleIdent::setIdentifier( const QByteArray &id )
{
  mMsgIdList.clear();
  appendIdentifier( id );
}

bool SingleIdent::parse( const char* &scursor, const char * const send,
                         bool isCRLF )
{
  if ( !Ident::parse( scursor, send, isCRLF ) ) {
    return false;
  }

  if ( mMsgIdList.count() > 1 ) {
    KMIME_WARN << "more than one msg-id in header "
               << "allowing only a single one!" << endl;
  }
  return true;
}

//-----</SingleIdent>-------------------------

} // namespace Generics

//-----<ReturnPath>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( ReturnPath, Generics::Address, Return-Path )
//@endcond

QByteArray ReturnPath::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }
  rv += mMailbox.as7BitString( e_ncCS );
  return rv;
}

void ReturnPath::clear()
{
  mMailbox.setAddress( Types::AddrSpec() );
  mMailbox.setName( QString() );
}

bool ReturnPath::isEmpty() const
{
  return !mMailbox.hasAddress() && !mMailbox.hasName();
}

bool ReturnPath::parse( const char* &scursor, const char * const send,
                        bool isCRLF )
{
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  const char * oldscursor = scursor;

  Mailbox maybeMailbox;
  if ( !parseMailbox( scursor, send, maybeMailbox, isCRLF ) ) {
    // mailbox parsing failed, but check for empty brackets:
    scursor = oldscursor;
    if ( *scursor != '<' ) {
      return false;
    }
    scursor++;
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send || *scursor != '>' ) {
      return false;
    }
    scursor++;

    // prepare a Null mailbox:
    AddrSpec emptyAddrSpec;
    maybeMailbox.setName( QString() );
    maybeMailbox.setAddress( emptyAddrSpec );
  } else {
    // check that there was no display-name:
    if ( maybeMailbox.hasName() ) {
      KMIME_WARN << "display-name \"" << maybeMailbox.name()
                 << "\" in Return-Path!" << endl;
    }
  }

  // see if that was all:
  eatCFWS( scursor, send, isCRLF );
  // and warn if it wasn't:
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after angle-addr in Return-Path!" << endl;
  }
  return true;
}

//-----</ReturnPath>-------------------------

//-----<Generic>-------------------------------

Generic::Generic() : Generics::Unstructured(), t_ype( 0 )
{
}

Generic::Generic( const char *t ) : Generics::Unstructured(), t_ype( 0 )
{
  setType( t );
}

Generic::Generic( const char *t, Content *p )
  : Generics::Unstructured( p ), t_ype( 0 )
{
  setType( t );
}

Generic::Generic( const char *t, Content *p, const QByteArray &s )
  : Generics::Unstructured( p, s ), t_ype( 0 )
{
  setType( t );
}

Generic::Generic( const char *t, Content *p, const QString &s, const QByteArray &cs )
  : Generics::Unstructured( p, s, cs ), t_ype( 0 )
{
  setType( t );
}

Generic::~Generic()
{
  delete[] t_ype;
}

void Generic::clear()
{
  delete[] t_ype;
  Unstructured::clear();
}

bool Generic::isEmpty() const
{
  return t_ype == 0 || Unstructured::isEmpty();
}

const char *Generic::type() const
{
  return t_ype;
}

void Generic::setType( const char *type )
{
  if ( t_ype ) {
    delete[] t_ype;
  }
  if ( type ) {
    t_ype = new char[strlen( type )+1];
    strcpy( t_ype, type );
  } else {
    t_ype = 0;
  }
}

//-----<Generic>-------------------------------

//-----<MessageID>-----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( MessageID, Generics::SingleIdent, Message-Id )
//@endcond

void MessageID::generate( const QByteArray &fqdn )
{
  setIdentifier( uniqueString() + '@' + fqdn + '>' );
}

//-----</MessageID>----------------------------

//-----<Control>-------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( Control, Generics::Structured, Control )
//@endcond

QByteArray Control::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }

  rv += mName;
  if ( !mParameter.isEmpty() ) {
    rv += ' ' + mParameter;
  }
  return rv;
}

void Control::clear()
{
  mName.clear();
  mParameter.clear();
}

bool Control::isEmpty() const
{
  return mName.isEmpty();
}

QByteArray Control::controlType() const
{
  return mName;
}

QByteArray Control::parameter() const
{
  return mParameter;
}

bool Control::isCancel() const
{
  return mName.toLower() == "cancel";
}

void Control::setCancel( const QByteArray &msgid )
{
  mName = "cancel";
  mParameter = msgid;
}

bool Control::parse( const char* &scursor, const char *const send, bool isCRLF )
{
  clear();
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }
  const char *start = scursor;
  while ( scursor != send && !isspace( *scursor ) ) {
    ++scursor;
  }
  mName = QByteArray( start, scursor - start );
  eatCFWS( scursor, send, isCRLF );
  mParameter = QByteArray( scursor, send - scursor );
  return true;
}

//-----</Control>------------------------------

//-----<MailCopiesTo>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( MailCopiesTo,
                                 Generics::AddressList, Mail-Copies-To )
//@endcond

QByteArray MailCopiesTo::as7BitString( bool withHeaderType ) const
{
  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }
  if ( !AddressList::isEmpty() ) {
    rv += AddressList::as7BitString( false );
  } else {
    if ( mAlwaysCopy ) {
      rv += "poster";
    } else if ( mNeverCopy ) {
      rv += "nobody";
    }
  }
  return rv;
}

QString MailCopiesTo::asUnicodeString() const
{
  if ( !AddressList::isEmpty() ) {
    return AddressList::asUnicodeString();
  }
  if ( mAlwaysCopy ) {
    return QLatin1String( "poster" );
  }
  if ( mNeverCopy ) {
    return QLatin1String( "nobody" );
  }
  return QString();
}

void MailCopiesTo::clear()
{
  AddressList::clear();
  mAlwaysCopy = false;
  mNeverCopy = false;
}

bool MailCopiesTo::isEmpty() const
{
  return AddressList::isEmpty() && !(mAlwaysCopy || mNeverCopy);
}

bool MailCopiesTo::alwaysCopy() const
{
  return !AddressList::isEmpty() || mAlwaysCopy;
}

void MailCopiesTo::setAlwaysCopy()
{
  clear();
  mAlwaysCopy = true;
}

bool MailCopiesTo::neverCopy() const
{
  return mNeverCopy;
}

void MailCopiesTo::setNeverCopy()
{
  clear();
  mNeverCopy = true;
}

bool MailCopiesTo::parse( const char *& scursor, const char * const send,
                          bool isCRLF )
{
  clear();
  if ( send - scursor == 5 ) {
    if ( qstrnicmp( "never", scursor, 5 ) == 0 ) {
      mNeverCopy = true;
      return true;
    }
  }
  if ( send - scursor == 6 ) {
    if ( qstrnicmp( "always", scursor, 6 ) == 0 || qstrnicmp( "poster", scursor, 6 ) == 0 ) {
      mAlwaysCopy = true;
      return true;
    }
    if ( qstrnicmp( "nobody", scursor, 6 ) == 0 ) {
      mAlwaysCopy = true;
      return true;
    }
  }
  return AddressList::parse( scursor, send, isCRLF );
}

//-----</MailCopiesTo>-------------------------

//-----<Date>----------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( Date, Generics::Structured, Date )
//@endcond

QByteArray Date::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }
  rv += mDateTime.toString( KDateTime::RFCDateDay ).toLatin1();
  return rv;
}

void Date::clear()
{
  mDateTime = KDateTime();
}

bool Date::isEmpty() const
{
  return mDateTime.isNull() || !mDateTime.isValid();
}

KDateTime Date::dateTime() const
{
  return mDateTime;
}

void Date::setDateTime( const KDateTime &dt )
{
  mDateTime = dt;
}

int Date::ageInDays() const
{
  QDate today = QDate::currentDate();
  return dateTime().date().daysTo(today);
}

bool Date::parse( const char* &scursor, const char *const send, bool isCRLF )
{
  return parseDateTime( scursor, send, mDateTime, isCRLF );
}

//-----</Date>---------------------------------

//-----<Newsgroups>----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( Newsgroups, Generics::Structured, Newsgroups )
kmime_mk_trivial_ctor_with_name( FollowUpTo, Newsgroups, Followup-To )
//@endcond

QByteArray Newsgroups::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }

  for ( int i = 0; i < mGroups.count(); ++i ) {
    rv += mGroups[ i ];
    if ( i != mGroups.count() - 1 ) {
      rv += ',';
    }
  }
  return rv;
}

void Newsgroups::fromUnicodeString( const QString &s, const QByteArray &b )
{
  Q_UNUSED( b );
  from7BitString( s.toUtf8() );
  e_ncCS = cachedCharset( "UTF-8" );
}

QString Newsgroups::asUnicodeString() const
{
  return QString::fromUtf8( as7BitString( false ) );
}

void Newsgroups::clear()
{
  mGroups.clear();
}

bool Newsgroups::isEmpty() const
{
  return mGroups.isEmpty();
}

QList<QByteArray> Newsgroups::groups() const
{
  return mGroups;
}

void Newsgroups::setGroups( const QList<QByteArray> &groups )
{
  mGroups = groups;
}

bool Newsgroups::isCrossposted() const
{
  return mGroups.count() >= 2;
}

bool Newsgroups::parse( const char* &scursor, const char *const send, bool isCRLF )
{
  clear();
  forever {
    eatCFWS( scursor, send, isCRLF );
    if ( scursor != send && *scursor == ',' ) {
      ++scursor;
    }
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return true;
    }
    const char *start = scursor;
    while ( scursor != send && !isspace( *scursor ) && *scursor != ',' ) {
      ++scursor;
    }
    QByteArray group( start, scursor - start );
    mGroups.append( group );
  }
  return true;
}

//-----</Newsgroups>---------------------------

//-----<Lines>---------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( Lines, Generics::Structured, Lines )
//@endcond

QByteArray Lines::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray num;
  num.setNum( mLines );

  if ( withHeaderType ) {
    return typeIntro() + num;
  }
  return num;
}

QString Lines::asUnicodeString() const
{
  if ( isEmpty() ) {
    return QString();
  }
  return QString::number( mLines );
}

void Lines::clear()
{
  mLines = -1;
}

bool Lines::isEmpty() const
{
  return mLines == -1;
}

int Lines::numberOfLines() const
{
  return mLines;
}

void Lines::setNumberOfLines( int lines )
{
  mLines = lines;
}

bool Lines::parse( const char* &scursor, const char* const send, bool isCRLF )
{
  eatCFWS( scursor, send, isCRLF );
  if ( parseDigits( scursor, send, mLines )  == 0 ) {
    clear();
    return false;
  }
  return true;
}

//-----</Lines>--------------------------------

//-----<Content-Type>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( ContentType, Generics::Parametrized,
                                 Content-Type )
//@endcond

bool ContentType::isEmpty() const
{
  return mMimeType.isEmpty();
}

void ContentType::clear()
{
  c_ategory = CCsingle;
  mMimeType.clear();
  mMimeSubType.clear();
  Parametrized::clear();
}

QByteArray ContentType::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }

  rv += mimeType();
  if ( !Parametrized::isEmpty() ) {
    rv += "; " + Parametrized::as7BitString( false );
  }

  return rv;
}

QByteArray ContentType::mimeType() const
{
  return mMimeType + '/' + mMimeSubType;
}

QByteArray ContentType::mediaType() const
{
  return mMimeType;
}

QByteArray ContentType::subType() const
{
  return mMimeSubType;
}

void ContentType::setMimeType( const QByteArray &mimeType )
{
  int pos = mimeType.indexOf( '/' );
  if ( pos < 0 ) {
    mMimeType = mimeType;
    mMimeSubType.clear();
  } else {
    mMimeType = mimeType.left( pos );
    mMimeSubType = mimeType.mid( pos + 1 );
  }
  Parametrized::clear();

  if ( isMultipart() ) {
    c_ategory = CCcontainer;
  } else {
    c_ategory = CCsingle;
  }
}

bool ContentType::isMediatype( const char *mediatype ) const
{
  return strncasecmp( mediaType().constData(), mediatype, strlen( mediatype ) ) == 0;
}

bool ContentType::isSubtype( const char *subtype ) const
{
  return strncasecmp( subType().constData(), subtype, strlen( subtype ) ) == 0;
}

bool ContentType::isText() const
{
  return strncasecmp( mediaType().constData(), "text", 4 ) == 0;
}

bool ContentType::isPlainText() const
{
  return strcasecmp( mimeType().constData(), "text/plain" ) == 0;
}

bool ContentType::isHTMLText() const
{
  return strcasecmp( mimeType().constData(), "text/html" ) == 0;
}

bool ContentType::isImage() const
{
  return strncasecmp( mediaType().constData(), "image", 5 ) == 0;
}

bool ContentType::isMultipart() const
{
  return strncasecmp( mediaType().constData(), "multipart", 9 ) == 0;
}

bool ContentType::isPartial() const
{
  return strcasecmp( mimeType().constData(), "message/partial" ) == 0;
}

QByteArray ContentType::charset() const
{
  QByteArray ret = parameter( "charset" ).toLatin1();
  if ( ret.isEmpty() || forceDefaultCharset() ) {
    //return the default-charset if necessary
    ret = defaultCharset();
  }
  return ret;
}

void ContentType::setCharset( const QByteArray &s )
{
  setParameter( "charset", QString::fromLatin1( s ) );
}

QByteArray ContentType::boundary() const
{
  return parameter( "boundary" ).toLatin1();
}

void ContentType::setBoundary( const QByteArray &s )
{
  setParameter( "boundary", QString::fromLatin1( s ) );
}

QString ContentType::name() const
{
  return parameter( "name" );
}

void ContentType::setName( const QString &s, const QByteArray &cs )
{
  e_ncCS = cs;
  setParameter( "name", s );
}

QByteArray ContentType::id() const
{
  return parameter( "id" ).toLatin1();
}

void ContentType::setId( const QByteArray &s )
{
  setParameter( "id", s );
}

int ContentType::partialNumber() const
{
  QByteArray p = parameter( "number" ).toLatin1();
  if ( !p.isEmpty() ) {
    return p.toInt();
  } else {
    return -1;
  }
}

int ContentType::partialCount() const
{
  QByteArray p = parameter( "total" ).toLatin1();
  if ( !p.isEmpty() ) {
    return p.toInt();
  } else {
    return -1;
  }
}

contentCategory ContentType::category() const
{
  return c_ategory;
}

void ContentType::setCategory( contentCategory c )
{
  c_ategory = c;
}

void ContentType::setPartialParams( int total, int number )
{
  setParameter( "number", QString::number( number ) );
  setParameter( "total", QString::number( total ) );
}

bool ContentType::parse( const char* &scursor, const char * const send,
                         bool isCRLF )
{
  // content-type: type "/" subtype *(";" parameter)

  clear();
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false; // empty header
  }

  // type
  QPair<const char*,int> maybeMimeType;
  if ( !parseToken( scursor, send, maybeMimeType, false /* no 8Bit */ ) ) {
    return false;
  }
  mMimeType = QByteArray( maybeMimeType.first, maybeMimeType.second ).toLower();

  // subtype
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '/' ) {
    return false;
  }
  scursor++;
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  QPair<const char*,int> maybeSubType;
  if ( !parseToken( scursor, send, maybeSubType, false /* no 8bit */ ) ) {
    return false;
  }
  mMimeSubType = QByteArray( maybeSubType.first, maybeSubType.second ).toLower();

  // parameter list
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    goto success; // no parameters
  }

  if ( *scursor != ';' ) {
    return false;
  }
  scursor++;

  if ( !Parametrized::parse( scursor, send, isCRLF ) ) {
    return false;
  }

  // adjust category
success:
  if ( isMultipart() ) {
    c_ategory = CCcontainer;
  } else {
    c_ategory = CCsingle;
  }
  return true;
}

//-----</Content-Type>-------------------------

//-----<ContentTransferEncoding>----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( ContentTransferEncoding,
                                 Generics::Token, Content-Transfer-Encoding )
//@endcond

typedef struct { const char *s; int e; } encTableType;

static const encTableType encTable[] =
{
  { "7Bit", CE7Bit },
  { "8Bit", CE8Bit },
  { "quoted-printable", CEquPr },
  { "base64", CEbase64 },
  { "x-uuencode", CEuuenc },
  { "binary", CEbinary },
  { 0, 0}
};

void ContentTransferEncoding::clear()
{
  d_ecoded = true;
  c_te = CE7Bit;
  Token::clear();
}

contentEncoding ContentTransferEncoding::encoding() const
{
  return c_te;
}

void ContentTransferEncoding::setEncoding( contentEncoding e )
{
  c_te = e;

  for ( int i = 0; encTable[i].s != 0; ++i ) {
    if ( c_te == encTable[i].e ) {
      setToken( encTable[i].s );
      break;
    }
  }
}

bool ContentTransferEncoding::decoded() const
{
  return d_ecoded;
}

void ContentTransferEncoding::setDecoded( bool d )
{
  d_ecoded = d;
}

bool ContentTransferEncoding::needToEncode() const
{
  return d_ecoded && (c_te == CEquPr || c_te == CEbase64);
}

bool ContentTransferEncoding::parse( const char *& scursor,
                                     const char * const send, bool isCRLF )
{
  clear();
  if ( !Token::parse( scursor, send, isCRLF ) ) {
    return false;
  }

  // TODO: error handling in case of an unknown encoding?
  for ( int i = 0; encTable[i].s != 0; ++i ) {
    if ( strcasecmp( token().constData(), encTable[i].s ) == 0 ) {
      c_te = ( contentEncoding )encTable[i].e;
      break;
    }
  }
  d_ecoded = ( c_te == CE7Bit || c_te == CE8Bit );
  return true;
}

//-----</ContentTransferEncoding>---------------------------

//-----<ContentDisposition>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( ContentDisposition,
                                 Generics::Parametrized, Content-Disposition )
//@endcond

QByteArray ContentDisposition::as7BitString( bool withHeaderType ) const
{
  if ( isEmpty() ) {
    return QByteArray();
  }

  QByteArray rv;
  if ( withHeaderType ) {
    rv += typeIntro();
  }

  if ( mDisposition == CDattachment ) {
    rv += "attachment";
  } else if ( mDisposition == CDinline ) {
    rv += "inline";
  } else {
    return QByteArray();
  }

  if ( !Parametrized::isEmpty() ) {
    rv += "; " + Parametrized::as7BitString( false );
  }

  return rv;
}

bool ContentDisposition::isEmpty() const
{
  return mDisposition == CDInvalid;
}

void ContentDisposition::clear()
{
  mDisposition = CDInvalid;
  Parametrized::clear();
}

contentDisposition ContentDisposition::disposition() const
{
  return mDisposition;
}

void ContentDisposition::setDisposition( contentDisposition d )
{
  mDisposition = d;
}

QString KMime::Headers::ContentDisposition::filename() const
{
  return parameter( "filename" );
}

void ContentDisposition::setFilename( const QString &filename )
{
  setParameter( "filename", filename );
}

bool ContentDisposition::parse( const char *& scursor, const char * const send,
                                bool isCRLF )
{
  clear();

  // token
  QByteArray token;
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  QPair<const char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8Bit */ ) ) {
    return false;
  }

  token = QByteArray( maybeToken.first, maybeToken.second ).toLower();
  if ( token == "inline" ) {
    mDisposition = CDinline;
  } else if ( token == "attachment" ) {
    mDisposition = CDattachment;
  } else {
    return false;
  }

  // parameter list
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return true; // no parameters
  }

  if ( *scursor != ';' ) {
    return false;
  }
  scursor++;

  return Parametrized::parse( scursor, send, isCRLF );
}

//-----</ContentDisposition>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( Subject, Generics::Unstructured, Subject )
//@endcond

bool Subject::isReply() const
{
  return asUnicodeString().indexOf( QLatin1String( "Re:" ), 0, Qt::CaseInsensitive ) == 0;
}

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name( ContentDescription,
                                 Generics::Unstructured, Content-Description )
kmime_mk_trivial_ctor_with_name( From, Generics::MailboxList, From )
kmime_mk_trivial_ctor_with_name( Sender, Generics::SingleMailbox, Sender )
kmime_mk_trivial_ctor_with_name( To, Generics::AddressList, To )
kmime_mk_trivial_ctor_with_name( Cc, Generics::AddressList, Cc )
kmime_mk_trivial_ctor_with_name( Bcc, Generics::AddressList, Bcc )
kmime_mk_trivial_ctor_with_name( ReplyTo, Generics::AddressList, Reply-To )
kmime_mk_trivial_ctor_with_name( Keywords, Generics::PhraseList, Keywords )
kmime_mk_trivial_ctor_with_name( MIMEVersion, Generics::DotAtom, MIME-Version )
kmime_mk_trivial_ctor_with_name( ContentID, Generics::SingleIdent, Content-ID )
kmime_mk_trivial_ctor_with_name( Supersedes, Generics::SingleIdent, Supersedes )
kmime_mk_trivial_ctor_with_name( InReplyTo, Generics::Ident, In-Reply-To )
kmime_mk_trivial_ctor_with_name( References, Generics::Ident, References )
kmime_mk_trivial_ctor_with_name( Organization, Generics::Unstructured, Organization )
kmime_mk_trivial_ctor_with_name( UserAgent, Generics::Unstructured, User-Agent )
//@endcond

} // namespace Headers

} // namespace KMime
