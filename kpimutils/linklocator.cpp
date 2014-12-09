/*
  Copyright (c) 2002 Dave Corrie <kde@davecorrie.com>

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
  This file is part of the KDEPIM Utilities library and provides the
  LinkLocator class.

  @brief
  Identifies URLs and email addresses embedded in plaintext.

  @author Dave Corrie \<kde@davecorrie.com\>
*/
#include "linklocator.h"

#include <KEmoticons>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QTextDocument>

#include <climits>

using namespace KPIMUtils;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KPIMUtils::LinkLocator::Private
{
  public:
    int mMaxUrlLen;
    int mMaxAddressLen;
};
//@endcond

// Use a static for this as calls to the KEmoticons constructor are expensive.
K_GLOBAL_STATIC( KEmoticons, sEmoticons )

LinkLocator::LinkLocator( const QString &text, int pos )
  : mText( text ), mPos( pos ), d( new KPIMUtils::LinkLocator::Private )
{
  d->mMaxUrlLen = 4096;
  d->mMaxAddressLen = 255;

  // If you change either of the above values for maxUrlLen or
  // maxAddressLen, then please also update the documentation for
  // setMaxUrlLen()/setMaxAddressLen() in the header file AND the
  // default values used for the maxUrlLen/maxAddressLen parameters
  // of convertToHtml().
}

LinkLocator::~LinkLocator()
{
  delete d;
}

void LinkLocator::setMaxUrlLen( int length )
{
  d->mMaxUrlLen = length;
}

int LinkLocator::maxUrlLen() const
{
  return d->mMaxUrlLen;
}

void LinkLocator::setMaxAddressLen( int length )
{
  d->mMaxAddressLen = length;
}

int LinkLocator::maxAddressLen() const
{
  return d->mMaxAddressLen;
}

QString LinkLocator::getUrl()
{
  QString url;
  if ( atUrl() ) {
    // NOTE: see http://tools.ietf.org/html/rfc3986#appendix-A and especially appendix-C
    // Appendix-C mainly says, that when extracting URLs from plain text, line breaks shall
    // be allowed and should be ignored when the URI is extracted.

    // This implementation follows this recommendation and
    // allows the URL to be enclosed within different kind of brackets/quotes
    // If an URL is enclosed, whitespace characters are allowed and removed, otherwise
    // the URL ends with the first whitespace
    // Also, if the URL is enclosed in brackets, the URL itself is not allowed
    // to contain the closing bracket, as this would be detected as the end of the URL

    QChar beforeUrl, afterUrl;

    // detect if the url has been surrounded by brackets or quotes
    if ( mPos > 0 ) {
      beforeUrl = mText[mPos - 1];

      /*if ( beforeUrl == '(' ) {
        afterUrl = ')';
      } else */if ( beforeUrl == QLatin1Char('[') ) {
        afterUrl = QLatin1Char(']');
      } else if ( beforeUrl == QLatin1Char('<') ) {
        afterUrl = QLatin1Char('>');
      } else if ( beforeUrl == QLatin1Char('>') ) { // for e.g. <link>http://.....</link>
        afterUrl = QLatin1Char('<');
      } else if ( beforeUrl == QLatin1Char('"') ) {
        afterUrl = QLatin1Char('"');
      }
    }

    url.reserve( maxUrlLen() );  // avoid allocs
    int start = mPos;
    while ( ( mPos < (int)mText.length() ) &&
            ( mText[mPos].isPrint() || mText[mPos].isSpace() ) &&
            ( ( afterUrl.isNull() && !mText[mPos].isSpace() ) ||
              ( !afterUrl.isNull() && mText[mPos] != afterUrl ) ) ) {
      if ( !mText[mPos].isSpace() ) {   // skip whitespace
        url.append( mText[mPos] );
        if ( url.length() > maxUrlLen() ) {
          break;
        }
      }

      mPos++;
    }

    if ( isEmptyUrl( url ) || ( url.length() > maxUrlLen() ) ) {
      mPos = start;
      url.clear();
    } else {
      --mPos;
    }
  }

  // HACK: This is actually against the RFC. However, most people don't properly escape the URL in
  //       their text with "" or <>. That leads to people writing an url, followed immediatley by
  //       a dot to finish the sentence. That would lead the parser to include the dot in the url,
  //       even though that is not wanted. So work around that here.
  //       Most real-life URLs hopefully don't end with dots or commas.
  QList<QChar> wordBoundaries;
  wordBoundaries << QLatin1Char('.') << QLatin1Char(',') << QLatin1Char(':') << QLatin1Char('!') << QLatin1Char('?') << QLatin1Char(')') << QLatin1Char('>');
  if ( url.length() > 1 ) {
    do {
      if ( wordBoundaries.contains( url.at( url.length() - 1 ) ) ) {
        url.chop( 1 );
        --mPos;
      } else {
        break;
      }
    } while( url.length() > 1 );
  }

  return url;
}

// keep this in sync with KMMainWin::slotUrlClicked()
bool LinkLocator::atUrl() const
{
  // the following characters are allowed in a dot-atom (RFC 2822):
  // a-z A-Z 0-9 . ! # $ % & ' * + - / = ? ^ _ ` { | } ~
  static const QString allowedSpecialChars = QLatin1String( ".!#$%&'*+-/=?^_`{|}~" );

  // the character directly before the URL must not be a letter, a number or
  // any other character allowed in a dot-atom (RFC 2822).
  if ( ( mPos > 0 ) &&
       ( mText[mPos-1].isLetterOrNumber() ||
         ( allowedSpecialChars.indexOf( mText[mPos-1] ) != -1 ) ) ) {
    return false;
  }

  QChar ch = mText[mPos];
  return
    ( ch == QLatin1Char('h') && ( mText.mid( mPos, 7 ) == QLatin1String( "http://" ) ||
                     mText.mid( mPos, 8 ) == QLatin1String( "https://" ) ) ) ||
    ( ch == QLatin1Char('v') && mText.mid( mPos, 6 ) == QLatin1String( "vnc://" ) ) ||
    ( ch == QLatin1Char('f') && ( mText.mid( mPos, 7 ) == QLatin1String( "fish://" ) ||
                     mText.mid( mPos, 6 ) == QLatin1String( "ftp://" ) ||
                     mText.mid( mPos, 7 ) == QLatin1String( "ftps://" ) ) ) ||
    ( ch == QLatin1Char('s') && ( mText.mid( mPos, 7 ) == QLatin1String( "sftp://" ) ||
                     mText.mid( mPos, 6 ) == QLatin1String( "smb://" ) ) ) ||
    ( ch == QLatin1Char('m') && mText.mid( mPos, 7 ) == QLatin1String( "mailto:" ) ) ||
    ( ch == QLatin1Char('w') && mText.mid( mPos, 4 ) == QLatin1String( "www." ) ) ||
    ( ch == QLatin1Char('f') && ( mText.mid( mPos, 4 ) == QLatin1String( "ftp." ) ||
                     mText.mid( mPos, 7 ) == QLatin1String( "file://" ) ) )||
    ( ch == QLatin1Char('n') && mText.mid( mPos, 5 ) == QLatin1String( "news:" ) );
}

bool LinkLocator::isEmptyUrl( const QString &url ) const
{
  return url.isEmpty() ||
    url == QLatin1String( "http://" ) ||
    url == QLatin1String( "https://" ) ||
    url == QLatin1String( "fish://" ) ||
    url == QLatin1String( "ftp://" ) ||
    url == QLatin1String( "ftps://" ) ||
    url == QLatin1String( "sftp://" ) ||
    url == QLatin1String( "smb://" ) ||
    url == QLatin1String( "vnc://" ) ||
    url == QLatin1String( "mailto" ) ||
    url == QLatin1String( "www" ) ||
    url == QLatin1String( "ftp" ) ||
    url == QLatin1String( "news" ) ||
    url == QLatin1String( "news://" );
}

QString LinkLocator::getEmailAddress()
{
  QString address;

  if ( mText[mPos] == QLatin1Char('@') ) {
    // the following characters are allowed in a dot-atom (RFC 2822):
    // a-z A-Z 0-9 . ! # $ % & ' * + - / = ? ^ _ ` { | } ~
    static const QString allowedSpecialChars = QLatin1String( ".!#$%&'*+-/=?^_`{|}~" );

    // determine the local part of the email address
    int start = mPos - 1;
    while ( start >= 0 && mText[start].unicode() < 128 &&
            ( mText[start].isLetterOrNumber() ||
              mText[start] == QLatin1Char('@') || // allow @ to find invalid email addresses
              allowedSpecialChars.indexOf( mText[start] ) != -1 ) ) {
      if ( mText[start] == QLatin1Char('@') ) {
        return QString(); // local part contains '@' -> no email address
      }
      --start;
    }
    ++start;
    // we assume that an email address starts with a letter or a digit
    while ( ( start < mPos ) && !mText[start].isLetterOrNumber() ) {
      ++start;
    }
    if ( start == mPos ) {
      return QString(); // local part is empty -> no email address
    }

    // determine the domain part of the email address
    int dotPos = INT_MAX;
    int end = mPos + 1;
    while ( end < (int)mText.length() &&
            ( mText[end].isLetterOrNumber() ||
              mText[end] == QLatin1Char('@') || // allow @ to find invalid email addresses
              mText[end] == QLatin1Char('.') ||
              mText[end] == QLatin1Char('-') ) ) {
      if ( mText[end] == QLatin1Char('@') ) {
        return QString(); // domain part contains '@' -> no email address
      }
      if ( mText[end] == QLatin1Char('.') ) {
        dotPos = qMin( dotPos, end ); // remember index of first dot in domain
      }
      ++end;
    }
    // we assume that an email address ends with a letter or a digit
    while ( ( end > mPos ) && !mText[end - 1].isLetterOrNumber() ) {
      --end;
    }
    if ( end == mPos ) {
      return QString(); // domain part is empty -> no email address
    }
    if ( dotPos >= end ) {
      return QString(); // domain part doesn't contain a dot
    }

    if ( end - start > maxAddressLen() ) {
      return QString(); // too long -> most likely no email address
    }
    address = mText.mid( start, end - start );

    mPos = end - 1;
  }
  return address;
}

QString LinkLocator::convertToHtml( const QString &plainText, int flags,
                                    int maxUrlLen, int maxAddressLen )
{
  LinkLocator locator( plainText );
  locator.setMaxUrlLen( maxUrlLen );
  locator.setMaxAddressLen( maxAddressLen );

  QString str;
  QString result( (QChar*)0, (int)locator.mText.length() * 2 );
  QChar ch;
  int x;
  bool startOfLine = true;

  for ( locator.mPos = 0, x = 0; locator.mPos < (int)locator.mText.length();
        locator.mPos++, x++ ) {
    ch = locator.mText[locator.mPos];
    if ( flags & PreserveSpaces ) {
      if ( ch == QLatin1Char(' ') ) {
        if ( locator.mPos + 1 < locator.mText.length() ) {
          if ( locator.mText[locator.mPos + 1] != QLatin1Char(' ') ) {

            // A single space, make it breaking if not at the start or end of the line
            const bool endOfLine = locator.mText[locator.mPos + 1] == QLatin1Char('\n');
            if ( !startOfLine && !endOfLine ) {
              result += QLatin1Char(' ');
            } else {
              result += QLatin1String("&nbsp;");
            }
          } else {

            // Whitespace of more than one space, make it all non-breaking
            while ( locator.mPos < locator.mText.length() && locator.mText[locator.mPos] == QLatin1Char(' ') ) {
              result += QLatin1String("&nbsp;");
              locator.mPos++;
              x++;
            }

            // We incremented once to often, undo that
            locator.mPos--;
            x--;
          }
        } else {
          // Last space in the text, it is non-breaking
          result += QLatin1String("&nbsp;");
        }

        if ( startOfLine ) {
          startOfLine = false;
        }
        continue;
      } else if ( ch == QLatin1Char('\t') ) {
        do {
          result += QLatin1String("&nbsp;");
          x++;
        } while ( ( x & 7 ) != 0 );
        x--;
        startOfLine = false;
        continue;
      }
    }
    if ( ch == QLatin1Char('\n') ) {
      result += QLatin1String("<br />\n"); // Keep the \n, so apps can figure out the quoting levels correctly.
      startOfLine = true;
      x = -1;
      continue;
    }

    startOfLine = false;
    if ( ch == QLatin1Char('&') ) {
      result += QLatin1String("&amp;");
    } else if ( ch == QLatin1Char('"') ) {
      result += QLatin1String("&quot;");
    } else if ( ch == QLatin1Char('<') ) {
      result += QLatin1String("&lt;");
    } else if ( ch == QLatin1Char('>') ) {
      result += QLatin1String("&gt;");
    } else {
      const int start = locator.mPos;
      if ( !( flags & IgnoreUrls ) ) {
        str = locator.getUrl();
        if ( !str.isEmpty() ) {
          QString hyperlink;
          if ( str.left( 4 ) == QLatin1String("www.") ) {
            hyperlink = QLatin1String("http://") + str;
          } else if ( str.left( 4 ) == QLatin1String("ftp.") ) {
            hyperlink = QLatin1String("ftp://") + str;
          } else {
            hyperlink = str;
          }

          result += QLatin1String("<a href=\"") + hyperlink + QLatin1String("\">") + Qt::escape( str ) + QLatin1String("</a>");
          x += locator.mPos - start;
          continue;
        }
        str = locator.getEmailAddress();
        if ( !str.isEmpty() ) {
          // len is the length of the local part
          int len = str.indexOf( QLatin1Char('@') );
          QString localPart = str.left( len );

          // remove the local part from the result (as '&'s have been expanded to
          // &amp; we have to take care of the 4 additional characters per '&')
          result.truncate( result.length() -
                           len - ( localPart.count( QLatin1Char('&') ) * 4 ) );
          x -= len;

          result += QLatin1String("<a href=\"mailto:") + str + QLatin1String("\">") + str + QLatin1String("</a>");
          x += str.length() - 1;
          continue;
        }
      }
      if ( flags & HighlightText ) {
        str = locator.highlightedText();
        if ( !str.isEmpty() ) {
          result += str;
          x += locator.mPos - start;
          continue;
        }
      }
      result += ch;
    }
  }

  if ( flags & ReplaceSmileys ) {
    QStringList exclude;
    exclude << QLatin1String("(c)") << QLatin1String("(C)") << QLatin1String("&gt;:-(") << QLatin1String("&gt;:(") << QLatin1String("(B)") << QLatin1String("(b)") << QLatin1String("(P)") << QLatin1String("(p)");
    exclude << QLatin1String("(O)") << QLatin1String("(o)") << QLatin1String("(D)") << QLatin1String("(d)") << QLatin1String("(E)") << QLatin1String("(e)") << QLatin1String("(K)")<< QLatin1String("(k)");
    exclude << QLatin1String("(I)") << QLatin1String("(i)") << QLatin1String("(L)") << QLatin1String("(l)") << QLatin1String("(8)") << QLatin1String("(T)") << QLatin1String("(t)") << QLatin1String("(G)");
    exclude << QLatin1String("(g)") << QLatin1String("(F)") << QLatin1String("(f)") << QLatin1String("(H)");
    exclude << QLatin1String("8)") << QLatin1String("(N)") << QLatin1String("(n)") << QLatin1String("(Y)") << QLatin1String("(y)" )<< QLatin1String("(U)") << QLatin1String("(u)") << QLatin1String("(W)") << QLatin1String("(w)");
    static QString cachedEmoticonsThemeName;
    if ( cachedEmoticonsThemeName.isEmpty() ) {
      cachedEmoticonsThemeName = KEmoticons::currentThemeName();
    }
    result =
      sEmoticons->theme( cachedEmoticonsThemeName ).parseEmoticons(
        result, KEmoticonsTheme::StrictParse | KEmoticonsTheme::SkipHTML, exclude );
  }

  return result;
}

QString LinkLocator::pngToDataUrl( const QString &iconPath )
{
  if ( iconPath.isEmpty() ) {
    return QString();
  }

  QFile pngFile( iconPath );
  if ( !pngFile.open( QIODevice::ReadOnly | QIODevice::Unbuffered ) ) {
    return QString();
  }

  QByteArray ba = pngFile.readAll();
  pngFile.close();
  return QString::fromLatin1( "data:image/png;base64,%1" ).arg( QLatin1String(ba.toBase64().constData()) );
}

QString LinkLocator::highlightedText()
{
  // formating symbols must be prepended with a whitespace
  if ( ( mPos > 0 ) && !mText[mPos-1].isSpace() ) {
    return QString();
  }

  const QChar ch = mText[mPos];
  if ( ch != QLatin1Char('/') && ch != QLatin1Char('*') && ch != QLatin1Char('_') && ch != QLatin1Char('-') ) {
    return QString();
  }

  QRegExp re =
    QRegExp( QString::fromLatin1( "\\%1((\\w+)([\\s-']\\w+)*( ?[,.:\\?!;])?)\\%2" ).arg( ch ).arg( ch ) );
  re.setMinimal( true );
  if ( re.indexIn( mText, mPos ) == mPos ) {
    int length = re.matchedLength();
    // there must be a whitespace after the closing formating symbol
    if ( mPos + length < mText.length() && !mText[mPos + length].isSpace() ) {
      return QString();
    }
    mPos += length - 1;
    switch ( ch.toLatin1() ) {
    case '*':
      return QLatin1String("<b>*") + re.cap( 1 ) + QLatin1String("*</b>");
    case '_':
      return QLatin1String("<u>_") + re.cap( 1 ) + QLatin1String("_</u>");
    case '/':
      return QLatin1String("<i>/") + re.cap( 1 ) + QLatin1String("/</i>");
    case '-':
      return QLatin1String("<strike>-") + re.cap( 1 ) + QLatin1String("-</strike>");
    }
  }
  return QString();
}
