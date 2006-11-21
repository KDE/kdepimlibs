/*
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
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kmime_headers.h"

#include "kmime_util.h"
#include "kmime_content.h"
#include "kmime_codecs.h"
#include "kmime_header_parsing.h"
#include "kmime_warning.h"

#include <QTextCodec>
#include <QString>
#include <QStringList>

#include <kglobal.h>
#include <kcharsets.h>
#include <krfcdate.h>

#include <assert.h>

// macro to generate a default constructor implementation
#define kmime_mk_trivial_ctor( subclass, baseclass ) \
subclass::subclass() : baseclass() \
{ \
  clear(); \
} \
\
subclass::subclass( Content *parent ) : baseclass( parent ) \
{ \
  clear(); \
} \
\
subclass::subclass( Content *parent, const QByteArray &s ) : baseclass( parent ) \
{ \
  from7BitString( s ); \
} \
\
subclass::subclass( Content *parent, const QString &s, const QByteArray &charset ) : \
  baseclass( parent ) \
{ \
  fromUnicodeString( s, charset ); \
} \
\
subclass::~subclass() {}

#define kmime_mk_trivial_ctor_with_name( subclass, baseclass, name ) \
kmime_mk_trivial_ctor( subclass, baseclass ) \
\
const char* subclass::type() const \
{ \
  return #name; \
}


using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Types;
using namespace KMime::HeaderParsing;

namespace KMime {
namespace Headers {
//-----<Base>----------------------------------

QByteArray Base::rfc2047Charset()
{
  if ( e_ncCS.isEmpty() || forceCS() ) {
    return defaultCS();
  } else {
    return e_ncCS;
  }
}

void Base::setRFC2047Charset( const QByteArray &cs )
{
  e_ncCS=cachedCharset( cs );
}

bool Base::forceCS()
{
  return ( p_arent != 0 ? p_arent->forceDefaultCharset() : false );
}

QByteArray Base::defaultCS()
{
  return ( p_arent != 0 ? p_arent->defaultCharset() : Latin1 );
}

//-----</Base>---------------------------------

namespace Generics {

//-----<Unstructured>-------------------------

void Unstructured::from7BitString( const QByteArray &str )
{
  d_ecoded = decodeRFC2047String( str, e_ncCS, defaultCS(), forceCS() );
}

QByteArray Unstructured::as7BitString( bool withHeaderType )
{
  QByteArray result;
  if ( withHeaderType ) {
    result = typeIntro();
  }
  result += encodeRFC2047String( d_ecoded, e_ncCS ) ;

  return result;
}

void Unstructured::fromUnicodeString( const QString &str,
				       const QByteArray &suggestedCharset )
{
  d_ecoded = str;
  e_ncCS = cachedCharset( suggestedCharset );
}

QString Unstructured::asUnicodeString()
{
  return d_ecoded;
}

//-----</Unstructured>-------------------------

//-----<Structured>-------------------------

void Structured::from7BitString(const QByteArray & str)
{
  if ( e_ncCS.isEmpty() )
    e_ncCS = defaultCS();
  const char *cursor = str.constData();
  parse( cursor, cursor + str.length() );
}

void Structured::fromUnicodeString(const QString & s, const QByteArray & b)
{
  e_ncCS = cachedCharset( b );
  from7BitString( s.toLatin1() );
}

//-----</Structured>-------------------------

//-----<Address>-------------------------

// helper method used in AddressList and MailboxList
static bool stringToMailbox( const QByteArray &address, const QString &displayName, Types::Mailbox &mbox )
{
  Types::AddrSpec addrSpec;
  mbox.setName( displayName );
  const char* cursor = address.constData();
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

QByteArray MailboxList::as7BitString(bool withHeaderType)
{
  if ( isEmpty() )
    return QByteArray();

  QByteArray rv;
  if ( withHeaderType )
    rv = typeIntro();
  foreach ( Types::Mailbox mbox, mMailboxList ) {
    rv += mbox.as7BitString( e_ncCS );
    rv += ", ";
  }
  rv.resize( rv.length() - 2 );
  return rv;
}

void MailboxList::fromUnicodeString(const QString & s, const QByteArray & b)
{
  e_ncCS = cachedCharset( b );
  from7BitString( encodeRFC2047String( s, b, false ) );
}

QString MailboxList::asUnicodeString()
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

void MailboxList::addAddress(const Types::Mailbox & mbox)
{
  mMailboxList.append( mbox );
}

void MailboxList::addAddress(const QByteArray & address, const QString & displayName)
{
  Types::Mailbox mbox;
  if ( stringToMailbox( address, displayName, mbox ) )
    mMailboxList.append( mbox );
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
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) )
    return false;

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

kmime_mk_trivial_ctor( SingleMailbox, MailboxList )

bool SingleMailbox::parse( const char* &scursor, const char *const send,
			   bool isCRLF )
{
  if ( !MailboxList::parse( scursor, send, isCRLF ) ) return false;

  if ( mMailboxList.count() > 1 ) {
    KMIME_WARN << "multiple mailboxes in header allowing only a single one!"
	       << endl;
  }
  return true;
}

//-----</SingleMailbox>-------------------------

//-----<AddressList>-------------------------

QByteArray AddressList::as7BitString(bool withHeaderType)
{
  if ( mAddressList.isEmpty() )
    return QByteArray();

  QByteArray rv;
  if ( withHeaderType )
    rv = typeIntro();
  foreach ( Types::Address addr, mAddressList ) {
    foreach ( Types::Mailbox mbox, addr.mailboxList ) {
      rv += mbox.as7BitString( e_ncCS );
      rv += ", ";
    }
  }
  rv.resize( rv.length() - 2 );
  return rv;
}

void AddressList::fromUnicodeString(const QString & s, const QByteArray & b)
{
  e_ncCS = cachedCharset( b );
  from7BitString( encodeRFC2047String( s, b, false ) );
}

QString AddressList::asUnicodeString()
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

void AddressList::addAddress(const QByteArray & address, const QString & displayName)
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

bool AddressList::parse( const char* &scursor, const char *const send, bool isCRLF )
{
  QList<Types::Address> maybeAddressList;
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) )
    return false;

  mAddressList = maybeAddressList;
  return true;
}

//-----</AddressList>-------------------------

//-----<Token>-------------------------

kmime_mk_trivial_ctor( Token, Structured )

QByteArray Token::as7BitString(bool withHeaderType)
{
  if ( isEmpty() )
    return QByteArray();
  if ( withHeaderType )
    return typeIntro() + mToken;
  return mToken;
}

void Token::clear()
{
  mToken.clear();
  Structured::clear();
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
  if ( scursor == send ) return false;

  QPair<const char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8bit chars */ ) )
    return false;
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

//-----<GPhraseList>-------------------------

bool GPhraseList::parse( const char* &scursor, const char *const send,
			 bool isCRLF )
{
  mPhraseList.clear();

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) return true;
    // empty entry: ignore.
    if ( *scursor != ',' ) { scursor++; continue; }

    QString maybePhrase;
    if ( !parsePhrase( scursor, send, maybePhrase, isCRLF ) )
      return false;
    mPhraseList.append( maybePhrase );

    eatCFWS( scursor, send, isCRLF );
    // non-empty entry ending the list: OK.
    if ( scursor == send ) return true;
    // comma separating the phrases: eat.
    if ( *scursor != ',' ) scursor++;
  }
  return true;
}

//-----</GPhraseList>-------------------------

//-----<GDotAtom>-------------------------

bool GDotAtom::parse( const char* &scursor, const char *const send,
		      bool isCRLF )
{
  QString maybeDotAtom;
  if ( !parseDotAtom( scursor, send, maybeDotAtom, isCRLF ) )
    return false;

  mDotAtom = maybeDotAtom;

  eatCFWS( scursor, send, isCRLF );
  if ( scursor != send ) {
    KMIME_WARN << "trailing garbage after dot-atom in header allowing "
      "only a single dot-atom!" << endl;
  }
  return true;
}

//-----</GDotAtom>-------------------------

//-----<Parametrized>-------------------------

kmime_mk_trivial_ctor( Parametrized, Structured )

QByteArray Parametrized::as7BitString( bool withHeaderType )
{
  if ( isEmpty() )
    return QByteArray();

  QByteArray rv;
  if ( withHeaderType )
    rv += typeIntro();

  bool first = true;
  for ( QMap<QString,QString>::ConstIterator it = mParameterHash.constBegin();
        it != mParameterHash.constEnd(); ++it )
  {
    if ( !first )
      rv += "; ";
    else
      first = false;
    rv += it.key().toLatin1() + "=";
    if ( isUsAscii( it.value() ) ) {
      QByteArray tmp = it.value().toLatin1();
      addQuotes( tmp, false );
      rv += tmp;
    } else {
      // FIXME: encoded strings are not allowed inside quotes, OTOH we need to quote whitespaces...
      rv += "\"" + encodeRFC2047String( it.value(), e_ncCS ) + "\"";
    }
  }

  return rv;
}

QString Parametrized::parameter(const QString & key) const
{
  return mParameterHash.value( key );
}

void Parametrized::setParameter(const QString & key, const QString & value)
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

bool Parametrized::parse(const char *& scursor, const char * const send, bool isCRLF)
{
  mParameterHash.clear();
  if ( !parseParameterList( scursor, send, mParameterHash, isCRLF ) )
    return false;
  return true;
}

//-----</Parametrized>-------------------------

//-----<Ident>-------------------------

QByteArray Ident::as7BitString(bool withHeaderType)
{
  if ( mMsgIdList.isEmpty() )
    return QByteArray();

  QByteArray rv;
  if ( withHeaderType )
    rv = typeIntro();
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
    if ( scursor == send ) return true;
    // empty entry: ignore.
    if ( *scursor == ',' ) { scursor++; continue; }

    AddrSpec maybeMsgId;
    if ( !parseAngleAddr( scursor, send, maybeMsgId, isCRLF ) )
      return false;
    mMsgIdList.append( maybeMsgId );

    eatCFWS( scursor, send, isCRLF );
    // header end ending the list: OK.
    if ( scursor == send ) return true;
    // regular item separator: eat it.
    if ( *scursor == ',' ) scursor++;
  }
  return true;
}

QList<QByteArray> Ident::identifiers() const
{
  QList<QByteArray> rv;
  foreach ( Types::AddrSpec addr, mMsgIdList )
    rv.append( addr.asString().toLatin1() ); // FIXME change parsing to create QByteArrays
  return rv;
}

void Ident::appendIdentifier(const QByteArray & id)
{
  QByteArray tmp = id;
  if ( !tmp.startsWith( '<' ) )
    tmp.prepend( '<' );
  if ( !tmp.endsWith( '>' ) )
    tmp.append( '>' );
  AddrSpec msgId;
  const char* cursor = tmp.constData();
  if ( parseAngleAddr( cursor, cursor + tmp.length(), msgId ) )
    mMsgIdList.append( msgId );
  else
    kWarning() << k_funcinfo << "Unable to parse address spec!" << endl;
}


//-----</Ident>-------------------------

//-----<SingleIdent>-------------------------

QByteArray SingleIdent::identifier() const
{
  if ( mMsgIdList.isEmpty() )
    return QByteArray();
  return identifiers().first();
}

void SingleIdent::setIdentifier(const QByteArray & id)
{
  mMsgIdList.clear();
  appendIdentifier( id );
}

bool SingleIdent::parse( const char* &scursor, const char * const send, bool isCRLF )
{
  if ( !Ident::parse( scursor, send, isCRLF ) )
    return false;

  if ( mMsgIdList.count() > 1 ) {
    KMIME_WARN << "more than one msg-id in header "
      "allowing only a single one!" << endl;
  }
  return true;
}

//-----</SingleIdent>-------------------------

} // namespace Generics

//-----<ReturnPath>-------------------------

bool ReturnPath::parse( const char* &scursor, const char * const send, bool isCRLF )
{
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  const char * oldscursor = scursor;

  Mailbox maybeMailbox;
  if ( !parseMailbox( scursor, send, maybeMailbox, isCRLF ) ) {
    // mailbox parsing failed, but check for empty brackets:
    scursor = oldscursor;
    if ( *scursor != '<' ) return false;
    scursor++;
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send || *scursor != '>' ) return false;
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

void Generic::setType( const char *type )
{
  if ( t_ype )
    delete[] t_ype;
  if ( type ) {
    t_ype = new char[strlen( type )+1];
    strcpy( t_ype, type );
  } else {
    t_ype = 0;
  }
}

//-----<Generic>-------------------------------

//-----<MessageID>-----------------------------

kmime_mk_trivial_ctor_with_name( MessageID, Generics::SingleIdent, Message-Id )

void MessageID::generate( const QByteArray &fqdn )
{
  setIdentifier( uniqueString() + '@' + fqdn + '>' );
}

//-----</MessageID>----------------------------

//-----<Control>-------------------------------

void Control::from7BitString( const QByteArray &s )
{
  c_trlMsg = s;
}

QByteArray Control::as7BitString( bool incType )
{
  if ( incType ) {
    return ( typeIntro() + c_trlMsg );
  } else {
    return c_trlMsg;
  }
}

void Control::fromUnicodeString( const QString &s, const QByteArray& )
{
  c_trlMsg = s.toLatin1();
}

QString Control::asUnicodeString()
{
  return QString::fromLatin1( c_trlMsg );
}

//-----</Control>------------------------------

//-----<MailCopiesTo>--------------------------

kmime_mk_trivial_ctor_with_name( MailCopiesTo, Generics::AddressList, Mail-Copies-To )

QByteArray MailCopiesTo::as7BitString(bool withHeaderType)
{
  QByteArray rv;
  if ( withHeaderType )
    rv += typeIntro();
  if ( !AddressList::isEmpty() )
    rv += AddressList::as7BitString( false );
  else {
    if ( mAlwaysCopy )
      rv += "poster";
    else if ( mNeverCopy )
      rv += "nobody";
  }
  return rv;
}

QString MailCopiesTo::asUnicodeString()
{
  if ( !AddressList::isEmpty() )
    return AddressList::asUnicodeString();
  if ( mAlwaysCopy )
    return QLatin1String( "poster" );
  if ( mNeverCopy )
    return QLatin1String( "nobody" );
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

bool MailCopiesTo::parse(const char *& scursor, const char * const send, bool isCRLF)
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

void Date::from7BitString( const QByteArray &s )
{
  t_ime=KRFCDate::parseDate( s );
}

QByteArray Date::as7BitString( bool incType )
{
  if ( incType ) {
    return ( typeIntro() + KRFCDate::rfc2822DateString( t_ime ) );
  } else {
    return KRFCDate::rfc2822DateString( t_ime );
  }
}

void Date::fromUnicodeString( const QString &s, const QByteArray &barr )
{
  Q_UNUSED( barr );
  from7BitString( s.toLatin1() );
}

QString Date::asUnicodeString()
{
  return QString::fromLatin1( as7BitString( false ) );
}

QDateTime Date::qdt()
{
  QDateTime dt;
  dt.setTime_t( t_ime );
  return dt;
}

int Date::ageInDays()
{
  QDate today = QDate::currentDate();
  return ( qdt().date().daysTo(today) );
}

//-----</Date>---------------------------------

//-----<Newsgroups>----------------------------

void Newsgroups::from7BitString( const QByteArray &s )
{
  g_roups = s;
  e_ncCS=cachedCharset( "UTF-8" );
}

QByteArray Newsgroups::as7BitString( bool incType )
{
  if ( incType ) {
    return ( typeIntro() + g_roups );
  } else {
    return g_roups;
  }
}

void Newsgroups::fromUnicodeString( const QString &s, const QByteArray &barr )
{
  Q_UNUSED( barr );

  g_roups = s.toUtf8();
  e_ncCS = cachedCharset( "UTF-8" );
}

QString Newsgroups::asUnicodeString()
{
  return QString::fromUtf8( g_roups );
}

QByteArray Newsgroups::firstGroup()
{
  int pos = 0;
  if ( !g_roups.isEmpty() ) {
    pos = g_roups.indexOf( ',' );
    if ( pos == -1 ) {
      return g_roups;
    } else {
      return g_roups.left( pos );
    }
  } else {
    return QByteArray();
  }
}

QStringList Newsgroups::getGroups()
{
  QList<QByteArray> temp = g_roups.split( ',' );
  QStringList ret;
  QString s;

  foreach ( QByteArray group, temp ) {
    ret.append( group.simplified() );
  }

  return ret;
}

//-----</Newsgroups>---------------------------

//-----<Lines>---------------------------------

void Lines::from7BitString( const QByteArray &s )
{
  l_ines = s.toInt();
  e_ncCS = cachedCharset( Latin1 );
}

QByteArray Lines::as7BitString( bool incType )
{
  QByteArray num;
  num.setNum( l_ines );

  if ( incType ) {
    return ( typeIntro() + num );
  } else {
    return num;
  }
}

void Lines::fromUnicodeString( const QString &s, const QByteArray &barr )
{
  Q_UNUSED( barr );

  l_ines = s.toInt();
  e_ncCS = cachedCharset( Latin1 );
}

QString Lines::asUnicodeString()
{
  QString num;
  num.setNum( l_ines );

  return num;
}

//-----</Lines>--------------------------------

//-----<Content-Type>--------------------------

kmime_mk_trivial_ctor_with_name( ContentType, Generics::Parametrized, Content-Type )

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

QByteArray ContentType::as7BitString( bool incType )
{
  if ( isEmpty() )
    return QByteArray();

  QByteArray rv;
  if ( incType )
    rv += typeIntro();

  rv += mimeType();
  if ( !Parametrized::isEmpty() )
    rv += "; " + Parametrized::as7BitString( false );

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
  return ( strncasecmp( mediaType().constData(), mediatype, strlen( mediatype ) ) == 0 );
}

bool ContentType::isSubtype( const char *subtype ) const
{
  return ( strncasecmp( subType().constData(), subtype, strlen( subtype ) ) == 0 );
}

bool ContentType::isText() const
{
  return ( strncasecmp( mediaType().constData(), "text", 4 ) == 0 );
}

bool ContentType::isPlainText() const
{
  return ( strcasecmp( mimeType().constData(), "text/plain" ) == 0 );
}

bool ContentType::isHTMLText() const
{
  return ( strcasecmp( mimeType().constData(), "text/html" ) == 0 );
}

bool ContentType::isImage() const
{
  return ( strncasecmp( mediaType().constData(), "image", 5 ) == 0 );
}

bool ContentType::isMultipart() const
{
  return ( strncasecmp( mediaType().constData(), "multipart", 9 ) == 0 );
}

bool ContentType::isPartial() const
{
  return ( strcasecmp( mimeType().constData(), "message/partial" ) == 0 );
}

QByteArray ContentType::charset()
{
  QByteArray ret = parameter( "charset" ).toLatin1();
  if ( ret.isEmpty() || forceCS() ) //we return the default-charset if necessary
    ret = defaultCS();
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

void ContentType::setPartialParams( int total, int number )
{
  setParameter( "number", QString::number( number ) );
  setParameter( "total", QString::number( total ) );
}

bool ContentType::parse( const char* &scursor, const char * const send, bool isCRLF )
{
  // content-type: type "/" subtype *(";" parameter)

  clear();
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send )
    return false; // empty header

  // type
  QPair<const char*,int> maybeMimeType;
  if ( !parseToken( scursor, send, maybeMimeType, false /* no 8Bit */ ) )
    return false;
  mMimeType = QByteArray( maybeMimeType.first, maybeMimeType.second ).toLower();

  // subtype
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '/' )
    return false;
  scursor++;
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send )
    return false;

  QPair<const char*,int> maybeSubType;
  if ( !parseToken( scursor, send, maybeSubType, false /* no 8bit */ ) )
    return false;
  mMimeSubType = QByteArray( maybeSubType.first, maybeSubType.second ).toLower();

  // parameter list
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send )
    goto success; // no parameters

  if ( *scursor != ';' )
    return false;
  scursor++;

  if ( !Parametrized::parse( scursor, send, isCRLF ) )
    return false;

  // adjust category
  success:
  if ( isMultipart() )
    c_ategory = CCcontainer;
  else
    c_ategory = CCsingle;
  return true;
}

//-----</Content-Type>-------------------------

//-----<ContentTransferEncoding>----------------------------

kmime_mk_trivial_ctor_with_name( ContentTransferEncoding, Generics::Token, Content-Transfer-Encoding )

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

bool ContentTransferEncoding::parse(const char *& scursor, const char * const send, bool isCRLF)
{
  clear();
  if ( !Token::parse( scursor, send, isCRLF ) )
    return false;

  // TODO: error handling in case of an unknown encoding?
  for ( int i = 0; encTable[i].s != 0; ++i ) {
    if ( strcasecmp( token().constData(), encTable[i].s ) == 0 ) {
      c_te = (contentEncoding)encTable[i].e;
      break;
    }
  }
  d_ecoded = ( c_te == CE7Bit || c_te == CE8Bit );
  return true;
}

//-----</ContentTransferEncoding>---------------------------

//-----<ContentDisposition>--------------------------

kmime_mk_trivial_ctor_with_name( ContentDisposition, Generics::Parametrized, Content-Disposition )

QByteArray ContentDisposition::as7BitString( bool incType )
{
  if ( isEmpty() )
    return QByteArray();

  QByteArray rv;
  if ( incType )
    rv += typeIntro();

  if ( mDisposition == CDattachment )
    rv += "attachment";
  else if ( mDisposition == CDinline )
    rv += "inline";
  else
    return QByteArray();

  if ( !Parametrized::isEmpty() )
    rv += "; " + Parametrized::as7BitString( false );

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

void ContentDisposition::setDisposition(contentDisposition d)
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

bool ContentDisposition::parse(const char *& scursor, const char * const send, bool isCRLF)
{
  clear();

  // token
  QByteArray token;
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  QPair<const char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8Bit */ ) )
    return false;

  token = QByteArray( maybeToken.first, maybeToken.second ).toLower();
  if ( token == "inline" )
    mDisposition = CDinline;
  else if ( token == "attachment" )
    mDisposition = CDattachment;
  else
    return false;

  // parameter list
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return true; // no parameters

  if ( *scursor != ';' ) return false;
  scursor++;

  return Parametrized::parse( scursor, send, isCRLF );
}

//-----</ContentDisposition>-------------------------

kmime_mk_trivial_ctor_with_name( Subject, Generics::Unstructured, Subject )

bool Subject::isReply()
{
  return ( asUnicodeString().indexOf( QLatin1String( "Re:" ), 0, Qt::CaseInsensitive ) == 0 );
}

kmime_mk_trivial_ctor_with_name( ContentDescription, Generics::Unstructured, Content-Description )
kmime_mk_trivial_ctor_with_name( From, Generics::MailboxList, From )
kmime_mk_trivial_ctor_with_name( Sender, Generics::SingleMailbox, Sender )
kmime_mk_trivial_ctor_with_name( To, Generics::AddressList, To )
kmime_mk_trivial_ctor_with_name( Cc, Generics::AddressList, Cc )
kmime_mk_trivial_ctor_with_name( Bcc, Generics::AddressList, Bcc )
kmime_mk_trivial_ctor_with_name( ReplyTo, Generics::AddressList, Reply-To )
kmime_mk_trivial_ctor_with_name( Keywords, Generics::GPhraseList, Keywords )
kmime_mk_trivial_ctor_with_name( MIMEVersion, Generics::GDotAtom, MIME-Version )
kmime_mk_trivial_ctor_with_name( ContentID, Generics::SingleIdent, Content-ID )
kmime_mk_trivial_ctor_with_name( Supersedes, Generics::SingleIdent, Supersedes )
kmime_mk_trivial_ctor_with_name( InReplyTo, Generics::Ident, In-Reply-To )
kmime_mk_trivial_ctor_with_name( References, Generics::Ident, References )
kmime_mk_trivial_ctor_with_name( UserAgent, Generics::Unstructured, User-Agent )

} // namespace Headers

} // namespace KMime
