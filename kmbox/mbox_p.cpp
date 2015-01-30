/*
  Copyright (c) 2009 Bertjan Broeksema <broeksema@kde.org>

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

#include "mbox_p.h"

#include <KDebug>
#include <KUrl>
#include <QLocale>

using namespace KMBox;

MBoxPrivate::MBoxPrivate( MBox *mbox )
  : mInitialMboxFileSize( 0 ), mMBox( mbox ), mReadOnly( false ),
    mSeparatorMatcher( QLatin1String( "^From .*[0-9][0-9]:[0-9][0-9]" ) )
{
  connect( &mUnlockTimer, SIGNAL(timeout()), SLOT(unlockMBox()) );
}

MBoxPrivate::~MBoxPrivate()
{
  if ( mMboxFile.isOpen() ) {
    mMboxFile.close();
  }
}

bool MBoxPrivate::open()
{
  if ( mMboxFile.isOpen() ) {
    return true;  // already open
  }

  QIODevice::OpenMode mode = mReadOnly ? QIODevice::ReadOnly : QIODevice::ReadWrite;

  if ( !mMboxFile.open( mode ) ) { // messages file
    // failed to open readWrite -> try to open readOnly
    if ( !mMboxFile.open( QIODevice::ReadOnly ) ) {
      kDebug() << "Cannot open mbox file `" << mMboxFile.fileName() << "' FileError:"
               << mMboxFile.errorString();
      return false;
    } else {
      mReadOnly = true;
    }
  }

  return true;
}

void MBoxPrivate::close()
{
  if ( mMboxFile.isOpen() ) {
    mMboxFile.close();
  }

  mFileLocked = false;
}

void MBoxPrivate::initLoad( const QString &fileName )
{
  mMboxFile.setFileName( KUrl( fileName ).toLocalFile() );
  mAppendedEntries.clear();
  mEntries.clear();
}

bool MBoxPrivate::startTimerIfNeeded()
{
  if ( mUnlockTimer.interval() > 0 ) {
    mUnlockTimer.start();
    return true;
  }

  return false;
}

void MBoxPrivate::unlockMBox()
{
  mMBox->unlock();
}

QByteArray MBoxPrivate::mboxMessageSeparator( const QByteArray &msg )
{
  KMime::Message mail;
  QByteArray body, header;
  KMime::HeaderParsing::extractHeaderAndBody( KMime::CRLFtoLF( msg ), header, body );
  body.clear();
  mail.setHead( header );
  mail.parse();

  QByteArray separator = "From ";

  KMime::Headers::From *from = mail.from( false );
  if ( !from || from->addresses().isEmpty() ) {
    separator += "unknown@unknown.invalid";
  } else {
    separator += from->addresses().first() + ' ';
  }

  // format dateTime according to the mbox "standard" RFC4155
  KMime::Headers::Date *date = mail.date( false );
  QDateTime dateTime;
  if ( !date || date->isEmpty() ) {
    dateTime = QDateTime::currentDateTimeUtc();
  } else {
    dateTime = date->dateTime().toUtc().dateTime();
  }
  separator += QLocale::c().toString(dateTime, QLatin1String("ddd MMM dd HH:mm:ss yyyy")).toUtf8() + '\n';

  return separator;
}

#define STRDIM(x) (sizeof(x)/sizeof(*x)-1)

QByteArray MBoxPrivate::escapeFrom( const QByteArray &str )
{
  const unsigned int strLen = str.length();
  if ( strLen <= STRDIM( "From " ) ) {
    return str;
  }

  // worst case: \nFrom_\nFrom_\nFrom_... => grows to 7/6
  QByteArray result( int( strLen + 5 ) / 6 * 7 + 1, '\0' );

  const char *s = str.data();
  const char *const e = s + strLen - STRDIM( "From " );
  char *d = result.data();

  bool onlyAnglesAfterLF = false; // dont' match ^From_
  while ( s < e ) {
    switch ( *s ) {
      case '\n':
        onlyAnglesAfterLF = true;
        break;
      case '>':
        break;
      case 'F':
        if ( onlyAnglesAfterLF && qstrncmp( s+1, "rom ", STRDIM( "rom " ) ) == 0 ) {
          *d++ = '>';
        }
        // fall through
      default:
        onlyAnglesAfterLF = false;
        break;
    }
    *d++ = *s++;
  }

  while ( s < str.data() + strLen ) {
    *d++ = *s++;
  }

  result.truncate( d - result.data() );

  return result;
}

// performs (\n|^)>{n}From_ -> \1>{n-1}From_ conversion
void MBoxPrivate::unescapeFrom( char *str, size_t strLen )
{
  if ( !str ) {
    return;
  }

  if ( strLen <= STRDIM( ">From " ) ) {
    return;
  }

  // yes, *d++ = *s++ is a no-op as long as d == s (until after the
  // first >From_), but writes are cheap compared to reads and the
  // data is already in the cache from the read, so special-casing
  // might even be slower...
  const char *s = str;
  char *d = str;
  const char *const e = str + strLen - STRDIM( ">From " );

  while ( s < e ) {
    if ( *s == '\n' && *( s + 1 ) == '>' ) { // we can do the lookahead,
                                             // since e is 6 chars from the end!
      *d++ = *s++;  // == '\n'
      *d++ = *s++;  // == '>'

      while ( s < e && *s == '>' ) {
        *d++ = *s++;
      }

      if ( qstrncmp( s, "From ", STRDIM( "From " ) ) == 0 ) {
        --d;
      }
    }

    *d++ = *s++; // yes, s might be e here, but e is not the end :-)
  }
  // copy the rest:
  while ( s < str + strLen ) {
    *d++ = *s++;
  }

  if ( d < s ) { // only NUL-terminate if it's shorter
    *d = 0;
  }
}

bool MBoxPrivate::isMBoxSeparator( const QByteArray &line ) const
{
  if ( !line.startsWith( "From " ) ) {  //krazy:exclude=strings
    return false;
  }

  return mSeparatorMatcher.indexIn( QString::fromLatin1( line ) ) >= 0;
}

#undef STRDIM
