/*
  This file is part of the kpimutils library.
  Copyright (c) 2004 Matt Douhan <matt@fruitsalad.org>

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
/**
  @file
  This file is part of the KDEPIM Utilities library and provides
  static methods for email address validation.

  @author Matt Douhan \<matt@fruitsalad.org\>
*/
#include "email.h"

#include <kmime/kmime_util.h>

#include <KDebug>
#include <KLocale>
#include <KUrl>

#include <QtCore/QRegExp>
#include <QtCore/QByteArray>

#include <kglobal.h>

static const KCatalogLoader loader( "libkpimutils" );

using namespace KPIMUtils;

//-----------------------------------------------------------------------------
QStringList KPIMUtils::splitAddressList( const QString &aStr )
{
  // Features:
  // - always ignores quoted characters
  // - ignores everything (including parentheses and commas)
  //   inside quoted strings
  // - supports nested comments
  // - ignores everything (including double quotes and commas)
  //   inside comments

  QStringList list;

  if ( aStr.isEmpty() ) {
    return list;
  }

  QString addr;
  uint addrstart = 0;
  int commentlevel = 0;
  bool insidequote = false;

  for ( int index = 0; index<aStr.length(); index++ ) {
    // the following conversion to latin1 is o.k. because
    // we can safely ignore all non-latin1 characters
    switch ( aStr[index].toLatin1() ) {
    case '"' : // start or end of quoted string
      if ( commentlevel == 0 ) {
        insidequote = !insidequote;
      }
      break;
    case '(' : // start of comment
      if ( !insidequote ) {
        commentlevel++;
      }
      break;
    case ')' : // end of comment
      if ( !insidequote ) {
        if ( commentlevel > 0 ) {
          commentlevel--;
        } else {
          return list;
        }
      }
      break;
    case '\\' : // quoted character
      index++; // ignore the quoted character
      break;
    case ',' :
    case ';' :
      if ( !insidequote && ( commentlevel == 0 ) ) {
        addr = aStr.mid( addrstart, index - addrstart );
        if ( !addr.isEmpty() ) {
          list += addr.simplified();
        }
        addrstart = index + 1;
      }
      break;
    }
  }
  // append the last address to the list
  if ( !insidequote && ( commentlevel == 0 ) ) {
    addr = aStr.mid( addrstart, aStr.length() - addrstart );
    if ( !addr.isEmpty() ) {
      list += addr.simplified();
    }
  }

  return list;
}

//-----------------------------------------------------------------------------
// Used by KPIMUtils::splitAddress(...) and KPIMUtils::firstEmailAddress(...).
KPIMUtils::EmailParseResult splitAddressInternal( const QByteArray  address,
                                                  QByteArray &displayName,
                                                  QByteArray &addrSpec,
                                                  QByteArray &comment,
                                                  bool allowMultipleAddresses )
{
  //  kDebug() << "address";
  displayName = "";
  addrSpec = "";
  comment = "";

  if ( address.isEmpty() ) {
    return AddressEmpty;
  }

  // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
  // The purpose is to extract a displayable string from the mailboxes.
  // Comments in the addr-spec are not handled. No error checking is done.

  enum {
    TopLevel,
    InComment,
    InAngleAddress
  } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;
  bool stop = false;

  for ( const char *p = address.data(); *p && !stop; ++p ) {
    switch ( context ) {
    case TopLevel :
      {
      switch ( *p ) {
      case '"' :
        inQuotedString = !inQuotedString;
        displayName += *p;
        break;
      case '(' :
        if ( !inQuotedString ) {
          context = InComment;
          commentLevel = 1;
        } else {
          displayName += *p;
        }
        break;
      case '<' :
        if ( !inQuotedString ) {
          context = InAngleAddress;
        } else {
          displayName += *p;
        }
        break;
      case '\\' : // quoted character
        displayName += *p;
        ++p; // skip the '\'
        if ( *p ) {
          displayName += *p;
        } else {
          return UnexpectedEnd;
        }
        break;
      case ',' :
        if ( !inQuotedString ) {
          if ( allowMultipleAddresses ) {
            stop = true;
          } else {
            return UnexpectedComma;
          }
        } else {
          displayName += *p;
        }
        break;
      default :
        displayName += *p;
      }
      break;
      }
    case InComment :
      {
      switch ( *p ) {
      case '(' :
        ++commentLevel;
        comment += *p;
        break;
      case ')' :
        --commentLevel;
        if ( commentLevel == 0 ) {
          context = TopLevel;
          comment += ' '; // separate the text of several comments
        } else {
          comment += *p;
        }
        break;
      case '\\' : // quoted character
        comment += *p;
        ++p; // skip the '\'
        if ( *p ) {
          comment += *p;
        } else {
          return UnexpectedEnd;
        }
        break;
      default :
        comment += *p;
      }
      break;
      }
    case InAngleAddress :
      {
        switch ( *p ) {
      case '"' :
        inQuotedString = !inQuotedString;
        addrSpec += *p;
        break;
      case '>' :
        if ( !inQuotedString ) {
          context = TopLevel;
        } else {
          addrSpec += *p;
        }
        break;
      case '\\' : // quoted character
        addrSpec += *p;
        ++p; // skip the '\'
        if ( *p ) {
          addrSpec += *p;
        } else {
          return UnexpectedEnd;
        }
        break;
      default :
        addrSpec += *p;
      }
      break;
    }
    } // switch ( context )
  }
  // check for errors
  if ( inQuotedString ) {
    return UnbalancedQuote;
  }
  if ( context == InComment ) {
    return UnbalancedParens;
  }
  if ( context == InAngleAddress ) {
    return UnclosedAngleAddr;
  }

  displayName = displayName.trimmed();
  comment = comment.trimmed();
  addrSpec = addrSpec.trimmed();

  if ( addrSpec.isEmpty() ) {
    if ( displayName.isEmpty() ) {
      return NoAddressSpec;
    } else {
      addrSpec = displayName;
      displayName.truncate( 0 );
    }
  }
  /*
    kDebug() << "display-name : \"" << displayName << "\"";
    kDebug() << "comment      : \"" << comment << "\"";
    kDebug() << "addr-spec    : \"" << addrSpec << "\"";
  */
  return AddressOk;
}

//-----------------------------------------------------------------------------
EmailParseResult KPIMUtils::splitAddress( const QByteArray &address,
                                          QByteArray &displayName,
                                          QByteArray &addrSpec,
                                          QByteArray &comment )
{
  return splitAddressInternal( address, displayName, addrSpec, comment,
                               false/* don't allow multiple addresses */ );
}

//-----------------------------------------------------------------------------
EmailParseResult KPIMUtils::splitAddress( const QString &address,
                                          QString &displayName,
                                          QString &addrSpec,
                                          QString &comment )
{
  QByteArray d, a, c;
  // FIXME: toUtf8() is probably not safe here, what if the second byte of a multi-byte character
  //        has the same code as one of the ASCII characters that splitAddress uses as delimiters?
  EmailParseResult result = splitAddress( address.toUtf8(), d, a, c );

  if ( result == AddressOk ) {
    displayName = QString::fromUtf8( d );
    addrSpec = QString::fromUtf8( a );
    comment = QString::fromUtf8( c );
  }
  return result;
}

//-----------------------------------------------------------------------------
EmailParseResult KPIMUtils::isValidAddress( const QString &aStr )
{
  // If we are passed an empty string bail right away no need to process
  // further and waste resources
  if ( aStr.isEmpty() ) {
    return AddressEmpty;
  }

  // count how many @'s are in the string that is passed to us
  // if 0 or > 1 take action
  // at this point to many @'s cannot bail out right away since
  // @ is allowed in qoutes, so we use a bool to keep track
  // and then make a judgment further down in the parser
  // FIXME count only @ not in double quotes

  bool tooManyAtsFlag = false;

  int atCount = aStr.count( '@' );
  if ( atCount > 1 ) {
    tooManyAtsFlag = true;
  } else if ( atCount == 0 ) {
    return TooFewAts;
  }

  // The main parser, try and catch all weird and wonderful
  // mistakes users and/or machines can create

  enum {
    TopLevel,
    InComment,
    InAngleAddress
  } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;

  unsigned int strlen = aStr.length();

  for ( unsigned int index = 0; index < strlen; index++ ) {
    switch ( context ) {
    case TopLevel :
      {
        switch ( aStr[index].toLatin1() ) {
        case '"' :
          inQuotedString = !inQuotedString;
          break;
        case '(' :
          if ( !inQuotedString ) {
            context = InComment;
            commentLevel = 1;
          }
          break;
        case '[' :
          if ( !inQuotedString ) {
            return InvalidDisplayName;
          }
          break;
        case ']' :
          if ( !inQuotedString ) {
            return InvalidDisplayName;
          }
          break;
        case ':' :
          if ( !inQuotedString ) {
            return DisallowedChar;
          }
          break;
        case '<' :
          if ( !inQuotedString ) {
            context = InAngleAddress;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if ( ( index + 1 ) > strlen ) {
            return UnexpectedEnd;
          }
          break;
        case ',' :
          if ( !inQuotedString ) {
            return UnexpectedComma;
          }
          break;
        case ')' :
          if ( !inQuotedString ) {
            return UnbalancedParens;
          }
          break;
        case '>' :
          if ( !inQuotedString ) {
            return UnopenedAngleAddr;
          }
          break;
        case '@' :
          if ( !inQuotedString ) {
            if ( index == 0 ) {  // Missing local part
              return MissingLocalPart;
            } else if ( index == strlen-1 ) {
              return MissingDomainPart;
            }
          } else if ( inQuotedString ) {
            --atCount;
            if ( atCount == 1 ) {
              tooManyAtsFlag = false;
            }
          }
          break;
        }
        break;
      }
    case InComment :
      {
        switch ( aStr[index].toLatin1() ) {
        case '(' :
          ++commentLevel;
          break;
        case ')' :
          --commentLevel;
          if ( commentLevel == 0 ) {
            context = TopLevel;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if ( ( index + 1 ) > strlen ) {
            return UnexpectedEnd;
          }
          break;
        }
        break;
      }

    case InAngleAddress :
      {
        switch ( aStr[index].toLatin1() ) {
        case ',' :
          if ( !inQuotedString ) {
            return UnexpectedComma;
          }
          break;
        case '"' :
          inQuotedString = !inQuotedString;
          break;
        case '@' :
          if ( inQuotedString ) {
            --atCount;
            if ( atCount == 1 ) {
              tooManyAtsFlag = false;
            }
          }
          break;
        case '>' :
          if ( !inQuotedString ) {
            context = TopLevel;
            break;
          }
          break;
        case '\\' : // quoted character
          ++index; // skip the '\'
          if ( ( index + 1 ) > strlen ) {
            return UnexpectedEnd;
          }
          break;
        }
        break;
      }
    }
  }

  if ( atCount == 0 && !inQuotedString ) {
    return TooFewAts;
  }

  if ( inQuotedString ) {
    return UnbalancedQuote;
  }

  if ( context == InComment ) {
    return UnbalancedParens;
  }

  if ( context == InAngleAddress ) {
    return UnclosedAngleAddr;
  }

  if ( tooManyAtsFlag ) {
    return TooManyAts;
  }

  return AddressOk;
}

//-----------------------------------------------------------------------------
KPIMUtils::EmailParseResult KPIMUtils::isValidAddressList( const QString &aStr,
                                                           QString &badAddr )
{
  if ( aStr.isEmpty() ) {
    return AddressEmpty;
  }

  const QStringList list = splitAddressList( aStr );

  QStringList::const_iterator it = list.begin();
  EmailParseResult errorCode = AddressOk;
  for ( it = list.begin(); it != list.end(); ++it ) {
    errorCode = isValidAddress( *it );
    if ( errorCode != AddressOk ) {
      badAddr = ( *it );
      break;
    }
  }
  return errorCode;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::emailParseResultToString( EmailParseResult errorCode )
{
  switch ( errorCode ) {
  case TooManyAts :
    return i18n( "The email address you entered is not valid because it "
                 "contains more than one @. "
                 "You will not create valid messages if you do not "
                 "change your address." );
  case TooFewAts :
    return i18n( "The email address you entered is not valid because it "
                 "does not contain a @. "
                 "You will not create valid messages if you do not "
                 "change your address." );
  case AddressEmpty :
    return i18n( "You have to enter something in the email address field." );
  case MissingLocalPart :
    return i18n( "The email address you entered is not valid because it "
                 "does not contain a local part." );
  case MissingDomainPart :
    return i18n( "The email address you entered is not valid because it "
                 "does not contain a domain part." );
  case UnbalancedParens :
    return i18n( "The email address you entered is not valid because it "
                 "contains unclosed comments/brackets." );
  case AddressOk :
    return i18n( "The email address you entered is valid." );
  case UnclosedAngleAddr :
    return i18n( "The email address you entered is not valid because it "
                 "contains an unclosed angle bracket." );
  case UnopenedAngleAddr :
    return i18n( "The email address you entered is not valid because it "
                 "contains too many closing angle brackets." );
  case UnexpectedComma :
    return i18n( "The email address you have entered is not valid because it "
                 "contains an unexpected comma." );
  case UnexpectedEnd :
    return i18n( "The email address you entered is not valid because it ended "
                 "unexpectedly. This probably means you have used an escaping "
                 "type character like a '\\' as the last character in your "
                 "email address." );
  case UnbalancedQuote :
    return i18n( "The email address you entered is not valid because it "
                 "contains quoted text which does not end." );
  case NoAddressSpec :
    return i18n( "The email address you entered is not valid because it "
                 "does not seem to contain an actual email address, i.e. "
                 "something of the form joe@example.org." );
  case DisallowedChar :
    return i18n( "The email address you entered is not valid because it "
                 "contains an illegal character." );
  case InvalidDisplayName :
    return i18n( "The email address you have entered is not valid because it "
                 "contains an invalid display name." );
  }
  return i18n( "Unknown problem with email address" );
}

//-----------------------------------------------------------------------------
bool KPIMUtils::isValidSimpleAddress( const QString &aStr )
{
  // If we are passed an empty string bail right away no need to process further
  // and waste resources
  if ( aStr.isEmpty() ) {
    return false;
  }

  int atChar = aStr.lastIndexOf( '@' );
  QString domainPart = aStr.mid( atChar + 1 );
  QString localPart = aStr.left( atChar );

  // Both of these parts must be non empty
  // after all we cannot have emails like:
  // @kde.org, or  foo@
  if ( localPart.isEmpty() || domainPart.isEmpty() ) {
    return false;
  }

  bool tooManyAtsFlag = false;
  bool inQuotedString = false;
  int atCount = localPart.count( '@' );

  unsigned int strlen = localPart.length();
  for ( unsigned int index = 0; index < strlen; index++ ) {
    switch ( localPart[ index ].toLatin1() ) {
    case '"' :
      inQuotedString = !inQuotedString;
      break;
    case '@' :
      if ( inQuotedString ) {
        --atCount;
        if ( atCount == 0 ) {
          tooManyAtsFlag = false;
        }
      }
      break;
    }
  }

  QString addrRx;

  if ( localPart[ 0 ] == '\"' || localPart[ localPart.length()-1 ] == '\"' ) {
    addrRx = "\"[a-zA-Z@]*[\\w.@-]*[a-zA-Z0-9@]\"@";
  } else {
    addrRx = "[a-zA-Z]*[~|{}`\\^?=/+*'&%$#!_\\w.-]*[~|{}`\\^?=/+*'&%$#!_a-zA-Z0-9-]@";
  }
  if ( domainPart[ 0 ] == '[' || domainPart[ domainPart.length()-1 ] == ']' ) {
    addrRx += "\\[[0-9]{,3}(\\.[0-9]{,3}){3}\\]";
  } else {
    addrRx += "[\\w-#]+(\\.[\\w-#]+)*";
  }
  QRegExp rx( addrRx );
  return  rx.exactMatch( aStr ) && !tooManyAtsFlag;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::simpleEmailAddressErrorMsg()
{
  return i18n( "The email address you entered is not valid because it "
               "does not seem to contain an actual email address, i.e. "
               "something of the form joe@example.org." );
}

//-----------------------------------------------------------------------------
QByteArray KPIMUtils::extractEmailAddress( const QByteArray &address )
{
  QByteArray dummy1, dummy2, addrSpec;
  EmailParseResult result =
    splitAddressInternal( address, dummy1, addrSpec, dummy2,
                          false/* don't allow multiple addresses */ );
  if ( result != AddressOk ) {
    addrSpec = QByteArray();
    if ( result != AddressEmpty ) {
      kDebug()
        << "Input:" << address << "\nError:"
        << emailParseResultToString( result );
    }
  }

  return addrSpec;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::extractEmailAddress( const QString &address )
{
  return QString::fromUtf8( extractEmailAddress( address.toUtf8() ) );
}

//-----------------------------------------------------------------------------
QByteArray KPIMUtils::firstEmailAddress( const QByteArray &addresses )
{
  QByteArray dummy1, dummy2, addrSpec;
  EmailParseResult result =
    splitAddressInternal( addresses, dummy1, addrSpec, dummy2,
                          true/* allow multiple addresses */ );
  if ( result != AddressOk ) {
    addrSpec = QByteArray();
    if ( result != AddressEmpty ) {
      kDebug()
        << "Input: aStr\nError:"
        << emailParseResultToString( result );
    }
  }

  return addrSpec;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::firstEmailAddress( const QString &addresses )
{
  return QString::fromUtf8( firstEmailAddress( addresses.toUtf8() ) );
}

//-----------------------------------------------------------------------------
bool KPIMUtils::extractEmailAddressAndName( const QString &aStr,
                                            QString &mail, QString &name )
{
  name.clear();
  mail.clear();

  const int len = aStr.length();
  const char cQuotes = '"';

  bool bInComment = false;
  bool bInQuotesOutsideOfEmail = false;
  int i = 0, iAd = 0, iMailStart = 0, iMailEnd = 0;
  QChar c;
  unsigned int commentstack = 0;

  // Find the '@' of the email address
  // skipping all '@' inside "(...)" comments:
  while ( i < len ) {
    c = aStr[i];
    if ( '(' == c ) {
      commentstack++;
    }
    if ( ')' == c ) {
      commentstack--;
    }
    bInComment = commentstack != 0;
    if ( '"' == c && !bInComment ) {
      bInQuotesOutsideOfEmail = !bInQuotesOutsideOfEmail;
    }

    if ( !bInComment && !bInQuotesOutsideOfEmail ) {
      if ( '@' == c ) {
        iAd = i;
        break; // found it
      }
    }
    ++i;
  }

  if ( !iAd ) {
    // We suppose the user is typing the string manually and just
    // has not finished typing the mail address part.
    // So we take everything that's left of the '<' as name and the rest as mail
    for ( i = 0; len > i; ++i ) {
      c = aStr[i];
      if ( '<' != c ) {
        name.append( c );
      } else {
        break;
      }
    }
    mail = aStr.mid( i + 1 );
    if ( mail.endsWith( '>' ) ) {
      mail.truncate( mail.length() - 1 );
    }

  } else {
    // Loop backwards until we find the start of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text before the leading '<'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    for ( i = iAd-1; 0 <= i; --i ) {
      c = aStr[i];
      if ( bInComment ) {
        if ( '(' == c ) {
          if ( !name.isEmpty() ) {
            name.prepend( ' ' );
          }
          bInComment = false;
        } else {
          name.prepend( c ); // all comment stuff is part of the name
        }
      } else if ( bInQuotesOutsideOfEmail ) {
        if ( cQuotes == c ) {
          bInQuotesOutsideOfEmail = false;
        } else if ( c != '\\' ) {
          name.prepend( c );
        }
      } else {
        // found the start of this addressee ?
        if ( ',' == c ) {
          break;
        }
        // stuff is before the leading '<' ?
        if ( iMailStart ) {
          if ( cQuotes == c ) {
            bInQuotesOutsideOfEmail = true; // end of quoted text found
          } else {
            name.prepend( c );
          }
        } else {
          switch ( c.toLatin1() ) {
          case '<':
            iMailStart = i;
            break;
          case ')':
            if ( !name.isEmpty() ) {
              name.prepend( ' ' );
            }
            bInComment = true;
            break;
          default:
            if ( ' ' != c ) {
              mail.prepend( c );
            }
          }
        }
      }
    }

    name = name.simplified();
    mail = mail.simplified();

    if ( mail.isEmpty() ) {
      return false;
    }

    mail.append( '@' );

    // Loop forward until we find the end of the string
    // or a ',' that is outside of a comment
    //          and outside of quoted text behind the trailing '>'.
    bInComment = false;
    bInQuotesOutsideOfEmail = false;
    int parenthesesNesting = 0;
    for ( i = iAd+1; len > i; ++i ) {
      c = aStr[i];
      if ( bInComment ) {
        if ( ')' == c ) {
          if ( --parenthesesNesting == 0 ) {
            bInComment = false;
            if ( !name.isEmpty() ) {
              name.append( ' ' );
            }
          } else {
            // nested ")", add it
            name.append( ')' ); // name can't be empty here
          }
        } else {
          if ( '(' == c ) {
            // nested "("
            ++parenthesesNesting;
          }
          name.append( c ); // all comment stuff is part of the name
        }
      } else if ( bInQuotesOutsideOfEmail ) {
        if ( cQuotes == c ) {
          bInQuotesOutsideOfEmail = false;
        } else if ( c != '\\' ) {
          name.append( c );
        }
      } else {
        // found the end of this addressee ?
        if ( ',' == c ) {
          break;
        }
        // stuff is behind the trailing '>' ?
        if ( iMailEnd ) {
          if ( cQuotes == c ) {
            bInQuotesOutsideOfEmail = true; // start of quoted text found
          } else {
            name.append( c );
          }
        } else {
          switch ( c.toLatin1() ) {
          case '>':
            iMailEnd = i;
            break;
          case '(':
            if ( !name.isEmpty() ) {
              name.append( ' ' );
            }
            if ( ++parenthesesNesting > 0 ) {
              bInComment = true;
            }
            break;
          default:
            if ( ' ' != c ) {
              mail.append( c );
            }
          }
        }
      }
    }
  }

  name = name.simplified();
  mail = mail.simplified();

  return ! ( name.isEmpty() || mail.isEmpty() );
}

//-----------------------------------------------------------------------------
bool KPIMUtils::compareEmail( const QString &email1, const QString &email2,
                              bool matchName )
{
  QString e1Name, e1Email, e2Name, e2Email;

  extractEmailAddressAndName( email1, e1Email, e1Name );
  extractEmailAddressAndName( email2, e2Email, e2Name );

  return e1Email == e2Email &&
    ( !matchName || ( e1Name == e2Name ) );
}

//-----------------------------------------------------------------------------
QString KPIMUtils::normalizedAddress( const QString &displayName,
                                      const QString &addrSpec,
                                      const QString &comment )
{
  const QString realDisplayName = KMime::removeBidiControlChars( displayName );
  if ( realDisplayName.isEmpty() && comment.isEmpty() ) {
    return addrSpec;
  } else if ( comment.isEmpty() ) {
    if ( !realDisplayName.startsWith( '\"' ) ) {
      return quoteNameIfNecessary( realDisplayName ) + " <" + addrSpec + '>';
    } else {
      return realDisplayName + " <" + addrSpec + '>';
    }
  } else if ( realDisplayName.isEmpty() ) {
    QString commentStr = comment;
    return quoteNameIfNecessary( commentStr ) + " <" + addrSpec + '>';
  } else {
    return realDisplayName + " (" + comment + ") <" + addrSpec + '>';
  }
}

//-----------------------------------------------------------------------------
QString KPIMUtils::fromIdn( const QString &addrSpec )
{
  const int atPos = addrSpec.lastIndexOf( '@' );
  if ( atPos == -1 ) {
    return addrSpec;
  }

  QString idn = KUrl::fromAce( addrSpec.mid( atPos + 1 ).toLatin1() );
  if ( idn.isEmpty() ) {
    return QString();
  }

  return addrSpec.left( atPos + 1 ) + idn;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::toIdn( const QString &addrSpec )
{
  const int atPos = addrSpec.lastIndexOf( '@' );
  if ( atPos == -1 ) {
    return addrSpec;
  }

  QString idn = KUrl::toAce( addrSpec.mid( atPos + 1 ) );
  if ( idn.isEmpty() ) {
    return addrSpec;
  }

  return addrSpec.left( atPos + 1 ) + idn;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::normalizeAddressesAndDecodeIdn( const QString &str )
{
  //  kDebug() << str;
  if ( str.isEmpty() ) {
    return str;
  }

  const QStringList addressList = splitAddressList( str );
  QStringList normalizedAddressList;

  QByteArray displayName, addrSpec, comment;

  for ( QStringList::ConstIterator it = addressList.begin();
        ( it != addressList.end() );
        ++it ) {
    if ( !( *it ).isEmpty() ) {
      if ( splitAddress( ( *it ).toUtf8(),
                         displayName, addrSpec, comment ) == AddressOk ) {

        displayName = KMime::decodeRFC2047String( displayName ).toUtf8();
        comment = KMime::decodeRFC2047String( comment ).toUtf8();

        normalizedAddressList
          << normalizedAddress( QString::fromUtf8( displayName ),
                                fromIdn( QString::fromUtf8( addrSpec ) ),
                                QString::fromUtf8( comment ) );
      }
    }
  }
  /*
    kDebug() << "normalizedAddressList: \""
             << normalizedAddressList.join( ", " )
             << "\"";
  */
  return normalizedAddressList.join( ", " );
}

//-----------------------------------------------------------------------------
QString KPIMUtils::normalizeAddressesAndEncodeIdn( const QString &str )
{
  //kDebug() << str;
  if ( str.isEmpty() ) {
    return str;
  }

  const QStringList addressList = splitAddressList( str );
  QStringList normalizedAddressList;

  QByteArray displayName, addrSpec, comment;

  for ( QStringList::ConstIterator it = addressList.begin();
        ( it != addressList.end() );
        ++it ) {
    if ( !( *it ).isEmpty() ) {
      if ( splitAddress( ( *it ).toUtf8(),
                         displayName, addrSpec, comment ) == AddressOk ) {

        normalizedAddressList << normalizedAddress( QString::fromUtf8( displayName ),
                                                    toIdn( QString::fromUtf8( addrSpec ) ),
                                                    QString::fromUtf8( comment ) );
      }
    }
  }

  /*
    kDebug() << "normalizedAddressList: \""
             << normalizedAddressList.join( ", " )
             << "\"";
  */
  return normalizedAddressList.join( ", " );
}

//-----------------------------------------------------------------------------
// Escapes unescaped doublequotes in str.
static QString escapeQuotes( const QString &str )
{
  if ( str.isEmpty() ) {
    return QString();
  }

  QString escaped;
  // reserve enough memory for the worst case ( """..."" -> \"\"\"...\"\" )
  escaped.reserve( 2 * str.length() );
  unsigned int len = 0;
  for ( int i = 0; i < str.length(); ++i, ++len ) {
    if ( str[i] == '"' ) { // unescaped doublequote
      escaped[len] = '\\';
      ++len;
    } else if ( str[i] == '\\' ) { // escaped character
      escaped[len] = '\\';
      ++len;
      ++i;
      if ( i >= str.length() ) { // handle trailing '\' gracefully
        break;
      }
    }
    escaped[len] = str[i];
  }
  escaped.truncate( len );
  return escaped;
}

//-----------------------------------------------------------------------------
QString KPIMUtils::quoteNameIfNecessary( const QString &str )
{
  QString quoted = str;

  QRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
  // avoid double quoting
  if ( ( quoted[0] == '"' ) && ( quoted[quoted.length() - 1] == '"' ) ) {
    quoted = "\"" + escapeQuotes( quoted.mid( 1, quoted.length() - 2 ) ) + "\"";
  } else if ( quoted.indexOf( needQuotes ) != -1 ) {
    quoted = "\"" + escapeQuotes( quoted ) + "\"";
  }

  return quoted;
}

KUrl KPIMUtils::encodeMailtoUrl( const QString &mailbox )
{
  const QByteArray encodedPath = KMime::encodeRFC2047String( mailbox, "utf-8" );
  KUrl mailtoUrl;
  mailtoUrl.setProtocol( "mailto" );
  mailtoUrl.setPath( encodedPath );
  return mailtoUrl;
}

QString KPIMUtils::decodeMailtoUrl( const KUrl &mailtoUrl )
{
  Q_ASSERT( mailtoUrl.protocol().toLower() == "mailto" );
  return KMime::decodeRFC2047String( mailtoUrl.path().toUtf8() );
}
