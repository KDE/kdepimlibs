/*
    kmime_headers.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
    See file AUTHORS for details

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

//-----</Address>-------------------------

//-----<MailboxList>-------------------------

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
      if ( !mbox.hasName() ) {
        rv += mbox.address();
      } else {
        if ( isUsAscii( mbox.name() ) ) {
          QByteArray tmp = mbox.name().toLatin1();
          addQuotes( tmp, false );
          rv += tmp;
        } else {
          rv += encodeRFC2047String( mbox.name(), e_ncCS, true );
        }
        if ( mbox.hasAddress() )
          rv += " <" + mbox.address() + '>';
      }
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
  Types::AddrSpec addrSpec;
  mbox.setName( displayName );
  const char* cursor = address.constData();
  if ( !parseAngleAddr( cursor, cursor + address.length(), addrSpec ) ) {
    if ( !parseAddrSpec( cursor, cursor + address.length(), addrSpec ) ) {
      kWarning() << k_funcinfo << "Invalid address" << endl;
      return;
    }
  }
  mbox.setAddress( addrSpec );
  addr.mailboxList.append( mbox );
  mAddressList.append( addr );
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

bool AddressList::parse( const char* &scursor, const char *const send, bool isCRLF )
{
  QList<Types::Address> maybeAddressList;
  if ( !parseAddressList( scursor, send, maybeAddressList, isCRLF ) )
    return false;

  mAddressList = maybeAddressList;
  return true;
}

//-----</AddressList>-------------------------

//-----<GToken>-------------------------

bool GToken::parse( const char* &scursor, const char *const send,
		    bool isCRLF )
{

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

//-----</GToken>-------------------------

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

//-----<GParametrized>-------------------------

//-----</GParametrized>-------------------------

//-----</GContentType>-------------------------

bool GContentType::parse( const char* &scursor, const char * const send,
			  bool isCRLF ) {

  // content-type: type "/" subtype *(";" parameter)

  mMimeType = 0;
  mMimeSubType = 0;
  mParameterHash.clear();

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    // empty header
    return false;
  }

  //
  // type
  //

  QPair<const char*,int> maybeMimeType;
  if ( !parseToken( scursor, send, maybeMimeType, false /* no 8Bit */ ) )
    return false;

  mMimeType = QByteArray( maybeMimeType.first, maybeMimeType.second ).toLower();

  //
  // subtype
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '/' ) return false;
  scursor++;
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  QPair<const char*,int> maybeSubType;
  if ( !parseToken( scursor, send, maybeSubType, false /* no 8bit */ ) )
    return false;

  mMimeSubType = QByteArray( maybeSubType.first, maybeSubType.second ).toLower();

  //
  // parameter list
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return true; // no parameters

  if ( *scursor != ';' ) return false;
  scursor++;

  if ( !parseParameterList( scursor, send, mParameterHash, isCRLF ) )
    return false;

  return true;
}

//-----</GContentType>-------------------------

//-----<GTokenWithParameterList>-------------------------

bool GCISTokenWithParameterList::parse( const char* &scursor,
					const char * const send, bool isCRLF ) {

  mToken = 0;
  mParameterHash.clear();

  //
  // token
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return false;

  QPair<const char*,int> maybeToken;
  if ( !parseToken( scursor, send, maybeToken, false /* no 8Bit */ ) )
    return false;

  mToken = QByteArray( maybeToken.first, maybeToken.second ).toLower();

  //
  // parameter list
  //

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) return true; // no parameters

  if ( *scursor != ';' ) return false;
  scursor++;

  if ( !parseParameterList( scursor, send, mParameterHash, isCRLF ) )
    return false;

  return true;
}

//-----</GTokenWithParameterList>-------------------------

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

#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<AddressField>--------------------------
void AddressField::from7BitString( const QByteArray &s )
{
  int pos1=0, pos2=0, type=0;
  QByteArray n;

  //so what do we have here ?
  if ( QString( s ).contains(
       QRegExp( "*@*(*)", Qt::CaseInsensitive, QRegExp::Wildcard ) ) ) {
    type = 2;       // From: foo@bar.com (John Doe)
  } else if ( QString( s ).contains(
              QRegExp( "*<*@*>", Qt::CaseInsensitive, QRegExp::Wildcard ) ) ) {
    type = 1;  // From: John Doe <foo@bar.com>
  } else if ( QString( s ).contains(
              QRegExp( "*@*", Qt::CaseInsensitive, QRegExp::Wildcard ) ) ) {
    type = 0;     // From: foo@bar.com
  } else { //broken From header => just decode it
    n_ame = decodeRFC2047String( s, e_ncCS, defaultCS(), forceCS() );
    return;
  }

  switch( type )
  {
  case 0:
    e_mail = s;
    break;

  case 1:
    pos1 = 0;
    pos2 = s.indexOf( '<' );
    if ( pos2 != -1 ) {
      n = s.mid(pos1, pos2 - pos1).trimmed();
      pos1 = pos2 + 1;
      pos2 = s.indexOf( '>', pos1 );
      if ( pos2 != -1 )
        e_mail = s.mid( pos1, pos2 - pos1 ).trimmed();
    } else {
      return;
    }
    break;

  case 2:
    pos1 = 0;
    pos2 = s.indexOf( '(' );
    if ( pos2 != -1 ) {
      e_mail = s.mid( pos1, pos2 - pos1 ).trimmed();
      pos1 = pos2 + 1;
      pos2 = s.indexOf( ')', pos1 );
      if ( pos2 != -1 )
        n = s.mid( pos1, pos2 - pos1 ).trimmed();
    }
    break;

  default: break;
  }

  if ( !n.isEmpty() ) {
    removeQuots( n );
    n_ame = decodeRFC2047String( n, e_ncCS, defaultCS(), forceCS() );
  }
}

QByteArray AddressField::as7BitString( bool incType )
{
  QByteArray ret;

  if ( incType && type()[0] != '\0' )
    ret = typeIntro();

  if ( n_ame.isEmpty() ) {
    ret += e_mail;
  } else {
    if ( isUsAscii( n_ame ) ) {
      QByteArray tmp = n_ame.toLatin1();
      addQuotes( tmp, false );
      ret += tmp;
    } else {
      ret += encodeRFC2047String( n_ame, e_ncCS, true );
    }
    if ( !e_mail.isEmpty() )
      ret += " <" + e_mail + '>';
  }

  return ret;
}

void AddressField::fromUnicodeString( const QString &s, const QByteArray &cs )
{
  int pos1=0, pos2=0, type=0;
  QByteArray n;

  e_ncCS=cachedCharset( cs );

  //so what do we have here ?
  if ( s.contains(
       QRegExp( "*@*(*)", Qt::CaseInsensitive, QRegExp::Wildcard ) ) ) {
    type = 2;       // From: foo@bar.com (John Doe)
  } else if ( s.contains(
              QRegExp( "*<*@*>", Qt::CaseInsensitive, QRegExp::Wildcard ) ) ) {
    type = 1;  // From: John Doe <foo@bar.com>
  } else if ( s.contains(
              QRegExp( "*@*", Qt::CaseInsensitive, QRegExp::Wildcard ) ) ) {
    type = 0;     // From: foo@bar.com
  } else { //broken From header => just copy it
    n_ame = s;
    return;
  }

  switch( type )
  {
  case 0:
    e_mail = s.toLatin1();
    break;

  case 1:
    pos1 = 0;
    pos2 = s.indexOf( '<' );
    if ( pos2 != -1 ) {
      n_ame = s.mid( pos1, pos2 - pos1).trimmed();
      pos1 = pos2 + 1;
      pos2 = s.indexOf( '>', pos1 );
      if ( pos2 != -1 )
        e_mail = s.mid( pos1, pos2 - pos1 ).toLatin1();
    } else {
      return;
    }
    break;

  case 2:
    pos1 = 0;
    pos2 = s.indexOf( '(' );
    if ( pos2 != -1 ) {
      e_mail = s.mid( pos1, pos2 - pos1 ).trimmed().toLatin1();
      pos1 = pos2 + 1;
      pos2 = s.indexOf( ')', pos1 );
      if ( pos2 != -1 )
        n_ame = s.mid( pos1, pos2 - pos1 ).trimmed();
    }
    break;

  default: break;
  }

  if ( !n_ame.isEmpty() )
    removeQuots( n_ame );
}

QString AddressField::asUnicodeString()
{
  if ( n_ame.isEmpty() ) {
    return QString( e_mail );
  } else {
    QString s = n_ame;
    if ( !e_mail.isEmpty( ))
      s += " <" + e_mail + '>';
    return s;
  }
}

QByteArray AddressField::nameAs7Bit()
{
  return encodeRFC2047String( n_ame, e_ncCS );
}

void AddressField::setNameFrom7Bit( const QByteArray &s )
{
  n_ame = decodeRFC2047String( s, e_ncCS, defaultCS(), forceCS() );
}

//-----</AddressField>-------------------------
#endif

//-----<MailCopiesTo>--------------------------

bool MailCopiesTo::isValid()
{
  if ( hasEmail() )
    return true;

  if ( ( n_ame == "nobody" ) ||
       ( n_ame == "never" ) ||
       ( n_ame == "poster" ) ||
       ( n_ame == "always" ) ) {
    return true;
  } else {
    return false;
  }
}

bool MailCopiesTo::alwaysCopy()
{
  return ( hasEmail() || ( n_ame == "poster" ) || ( n_ame == "always" ) );
}

bool MailCopiesTo::neverCopy()
{
  return ( ( n_ame == "nobody" ) || ( n_ame == "never" ) );
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

//-----<UserAgent>-----------------------------

void UserAgent::from7BitString( const QByteArray &s )
{
  u_agent = s;
  e_ncCS = cachedCharset( Latin1 );
}

QByteArray UserAgent::as7BitString( bool incType )
{
  if ( incType ) {
    return ( typeIntro() + u_agent );
  } else {
    return u_agent;
  }
}

void UserAgent::fromUnicodeString( const QString &s, const QByteArray &barr )
{
  Q_UNUSED( barr );

  u_agent = s.toLatin1();
  e_ncCS = cachedCharset( Latin1 );
}

QString UserAgent::asUnicodeString()
{
  return QString::fromLatin1( u_agent );
}

//-----</UserAgent>----------------------------

#if !defined(KMIME_NEW_STYLE_CLASSTREE)
//-----<Content-Type>--------------------------

void ContentType::from7BitString( const QByteArray &s )
{
  int pos = s.indexOf( ';' );

  if ( pos == -1 ) {
    m_imeType = s.simplified();
  } else {
    m_imeType = s.left( pos ).simplified();
    p_arams = s.mid( pos, s.length() - pos ).simplified();
  }

  if ( isMultipart() ) {
    c_ategory = CCcontainer;
  } else {
    c_ategory = CCsingle;
  }

  e_ncCS = cachedCharset( Latin1 );
}

QByteArray ContentType::as7BitString( bool incType )
{
  if ( incType ) {
    return ( typeIntro() + m_imeType + p_arams );
  } else {
    return ( m_imeType + p_arams );
  }
}

void ContentType::fromUnicodeString( const QString &s, const QByteArray &barr )
{
  Q_UNUSED( barr );
  from7BitString( s.toLatin1() );
}

QString ContentType::asUnicodeString()
{
  return QString::fromLatin1( as7BitString( false ) );
}

QByteArray ContentType::mediaType()
{
  int pos = m_imeType.indexOf( '/' );
  if ( pos == -1 ) {
    return m_imeType;
  } else {
    return m_imeType.left( pos );
  }
}

QByteArray ContentType::subType()
{
  int pos = m_imeType.indexOf( '/' );
  if ( pos == -1 ) {
    return QByteArray();
  } else {
    return m_imeType.mid( pos, m_imeType.length() - pos );
  }
}

void ContentType::setMimeType( const QByteArray &s )
{
  p_arams.resize( 0 );
  m_imeType = s;

  if ( isMultipart() ) {
    c_ategory = CCcontainer;
  } else {
    c_ategory = CCsingle;
  }
}

bool ContentType::isMediatype( const char *s )
{
  return ( strncasecmp( m_imeType.data(), s, strlen( s ) ) );
}

bool ContentType::isSubtype( const char *s )
{
  char *c = strchr( m_imeType.data(), '/' );

  if ( ( c == 0 ) || ( *(c+1) == '\0' ) ) {
    return false;
  } else {
    return ( strcasecmp( c+1, s ) == 0 );
  }
}

bool ContentType::isText()
{
  return ( strncasecmp( m_imeType.data(), "text", 4 ) == 0 );
}

bool ContentType::isPlainText()
{
  return ( strcasecmp( m_imeType.data(), "text/plain" ) == 0 );
}

bool ContentType::isHTMLText()
{
  return ( strcasecmp( m_imeType.data(), "text/html" ) == 0 );
}

bool ContentType::isImage()
{
  return ( strncasecmp( m_imeType.data(), "image", 5 ) == 0 );
}

bool ContentType::isMultipart()
{
  return ( strncasecmp( m_imeType.data(), "multipart", 9 ) == 0 );
}

bool ContentType::isPartial()
{
  return ( strcasecmp( m_imeType.data(), "message/partial" ) == 0 );
}

QByteArray ContentType::charset()
{
  QByteArray ret = getParameter( "charset" );
  if ( ret.isEmpty() || forceCS() ) { //we return the default-charset if necessary
    ret = defaultCS();
  }
  return ret;
}

void ContentType::setCharset( const QByteArray &s )
{
  setParameter( "charset", s );
}

QByteArray ContentType::boundary()
{
  return getParameter( "boundary" );
}

void ContentType::setBoundary( const QByteArray &s )
{
  setParameter( "boundary", s, true );
}

QString ContentType::name()
{
  QByteArray dummy;
  return ( decodeRFC2047String(getParameter("name"), dummy, defaultCS(),
                               forceCS()) );
}

void ContentType::setName( const QString &s, const QByteArray &cs )
{
  e_ncCS = cs;

  if ( isUsAscii( s ) ) {
    QByteArray tmp = s.toLatin1();
    addQuotes( tmp, true );
    setParameter( "name", tmp, false );
  } else {
    // FIXME: encoded words can't be enclosed in quotes!!
    setParameter( "name", encodeRFC2047String( s, cs ), true );
  }
}

QByteArray ContentType::id()
{
  return ( getParameter( "id" ) );
}

void ContentType::setId( const QByteArray &s )
{
  setParameter( "id", s, true );
}

int ContentType::partialNumber()
{
  QByteArray p = getParameter( "number" );
  if ( !p.isEmpty() ) {
    return p.toInt();
  } else {
    return -1;
  }
}

int ContentType::partialCount()
{
  QByteArray p = getParameter( "total" );
  if ( !p.isEmpty() ) {
    return p.toInt();
  } else {
    return -1;
  }
}

void ContentType::setPartialParams( int total, int number )
{
  QByteArray num;
  num.setNum( number );
  setParameter( "number", num );
  num.setNum( total );
  setParameter( "total", num );
}

QByteArray ContentType::getParameter( const char *name )
{
  QByteArray ret;
  int pos1=0, pos2=0;

  pos1 = QString( p_arams ).indexOf( name, 0, Qt::CaseInsensitive );
  if ( pos1 != -1 ) {
    if ( ( pos2 = p_arams.indexOf( ';', pos1 ) ) == -1 ) {
      pos2 = p_arams.length();
    }
    pos1 += strlen( name ) + 1;
    ret = p_arams.mid( pos1, pos2 - pos1 );
    removeQuots( ret );
  }
  return ret;
}

void ContentType::setParameter( const QByteArray &name,
                                const QByteArray &value, bool doubleQuotes )
{
  int pos1=0, pos2=0;
  QByteArray param;

  if ( doubleQuotes ) {
    param = name + "=\"" + value + '\"';
  } else {
    param = name + '=' + value;
  }

  pos1 = QString( p_arams ).indexOf( name, 0, Qt::CaseInsensitive );
  if ( pos1 == -1 ) {
    p_arams += "; " + param;
  } else {
    pos2 = p_arams.indexOf( ';', pos1 );
    if ( pos2 == -1 )
      pos2 = p_arams.length();
    p_arams.remove( pos1, pos2 - pos1 );
    p_arams.insert( pos1, param );
  }
}

//-----</Content-Type>-------------------------

//-----<CTEncoding>----------------------------

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

void CTEncoding::from7BitString( const QByteArray &s )
{
  QByteArray stripped = s.simplified();
  c_te = CE7Bit;
  for ( int i=0; encTable[i].s!=0; i++ ) {
    if ( strcasecmp( stripped.data(), encTable[i].s ) == 0 ) {
      c_te = (contentEncoding)encTable[i].e;
      break;
    }
  }
  d_ecoded = ( c_te == CE7Bit || c_te == CE8Bit );

  e_ncCS = cachedCharset( Latin1 );
}

QByteArray CTEncoding::as7BitString( bool incType )
{
  QByteArray str;
  for ( int i=0; encTable[i].s!=0; i++ ) {
    if ( c_te == encTable[i].e ) {
      str = encTable[i].s;
      break;
    }
  }

  if ( incType ) {
    return ( typeIntro() + str );
  } else {
    return str;
  }
}

void CTEncoding::fromUnicodeString( const QString &s, const QByteArray &barr )
{
  Q_UNUSED( barr );
  from7BitString( s.toLatin1() );
}

QString CTEncoding::asUnicodeString()
{
  return QString::fromLatin1( as7BitString( false ) );
}

//-----</CTEncoding>---------------------------

//-----<CDisposition>--------------------------

void CDisposition::from7BitString( const QByteArray &s )
{
  if ( strncasecmp( s.data(), "attachment", 10 ) == 0 ) {
    d_isp = CDattachment;
  } else {
    d_isp = CDinline;
  }

  int pos = QString( s ).indexOf( "filename=", 0, Qt::CaseInsensitive );
  QByteArray fn;
  if ( pos > -1 ) {
    pos += 9;
    fn = s.mid( pos, s.length() - pos );
    removeQuots( fn );
    f_ilename = decodeRFC2047String( fn, e_ncCS, defaultCS(), forceCS() );
  }
}

QByteArray CDisposition::as7BitString( bool incType )
{
  QByteArray ret;
  if ( d_isp == CDattachment ) {
    ret = "attachment";
  } else {
    ret = "inline";
  }

  if ( !f_ilename.isEmpty() ) {
    if ( isUsAscii( f_ilename ) ) {
      QByteArray tmp = f_ilename.toLatin1();
      addQuotes( tmp, true );
      ret += "; filename=" + tmp;
    } else {
      // FIXME: encoded words can't be enclosed in quotes!!
      ret += "; filename=\"" + encodeRFC2047String(f_ilename, e_ncCS) + '\"';
    }
  }

  if ( incType ) {
    return ( typeIntro() + ret );
  } else {
    return ret;
  }
}

void CDisposition::fromUnicodeString( const QString &s, const QByteArray &cs )
{
  if ( strncasecmp( s.toLatin1(), "attachment", 10 ) == 0 ) {
    d_isp = CDattachment;
  } else {
    d_isp = CDinline;
  }

  int pos = s.indexOf( "filename=", 0, Qt::CaseInsensitive );
  if ( pos > -1 ) {
    pos += 9;
    f_ilename = s.mid( pos, s.length() - pos );
    removeQuots( f_ilename );
  }

  e_ncCS=cachedCharset( cs );
}

QString CDisposition::asUnicodeString()
{
  QString ret;
  if ( d_isp == CDattachment ) {
    ret = "attachment";
  } else {
    ret = "inline";
  }

  if ( !f_ilename.isEmpty() ) {
    ret += "; filename=\"" + f_ilename + '\"';
  }

  return ret;
}

//-----</CDisposition>-------------------------
#endif
} // namespace Headers

} // namespace KMime
