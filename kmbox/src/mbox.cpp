/*
  Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
  Copyright (c) 2009 Bertjan Broeksema <broeksema@kde.org>

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

  NOTE: Most of the code inside here is an slightly adjusted version of
  kdepim/kmail/kmfoldermbox.cpp. This is why I added a line for Stefan Taferner.

  Bertjan Broeksema, april 2009
*/

#include "mbox.h"
#include "mbox_p.h"
#include "mboxentry_p.h"

#include <QDebug>
#include <QStandardPaths>
#include <QUrl>

#include <QtCore/QBuffer>
#include <QtCore/QProcess>

using namespace KMBox;

/// public methods.

MBox::MBox()
  : d( new MBoxPrivate( this ) )
{
  // Set some sane defaults
  d->mFileLocked = false;
  d->mLockType = None;

  d->mUnlockTimer.setInterval( 0 );
  d->mUnlockTimer.setSingleShot( true );
}

MBox::~MBox()
{
  if ( d->mFileLocked ) {
    unlock();
  }

  d->close();

  delete d;
}

// Appended entries works as follows: When an mbox file is loaded from disk,
// d->mInitialMboxFileSize is set to the file size at that moment. New entries
// are stored in memory (d->mAppendedEntries). The initial file size and the size
// of the buffer determine the offset for the next message to append.
MBoxEntry MBox::appendMessage( const KMime::Message::Ptr &entry )
{
  // It doesn't make sense to add entries when we don't have an reference file.
  Q_ASSERT( !d->mMboxFile.fileName().isEmpty() );

  const QByteArray rawEntry = MBoxPrivate::escapeFrom( entry->encodedContent() );

  if ( rawEntry.size() <= 0 ) {
    qDebug() << "Message added to folder `" << d->mMboxFile.fileName()
             << "' contains no data. Ignoring it.";
    return MBoxEntry();
  }

  int nextOffset = d->mAppendedEntries.size(); // Offset of the appended message

  // Make sure the byte array is large enough to check for an end character.
  // Then check if the required newlines are there.
  if ( nextOffset < 1 && d->mMboxFile.size() > 0 ) { // Empty, add one empty line
    d->mAppendedEntries.append( "\n" );
    ++nextOffset;
  } else if ( nextOffset == 1 && d->mAppendedEntries.at( 0 ) != '\n' ) {
    // This should actually not happen, but catch it anyway.
    if ( d->mMboxFile.size() < 0 ) {
      d->mAppendedEntries.append( "\n" );
      ++nextOffset;
    }
  } else if ( nextOffset >= 2 ) {
    if ( d->mAppendedEntries.at( nextOffset - 1 ) != '\n' ) {
      if ( d->mAppendedEntries.at( nextOffset ) != '\n' ) {
        d->mAppendedEntries.append( "\n\n" );
        nextOffset += 2;
      } else {
        d->mAppendedEntries.append( "\n" );
        ++nextOffset;
      }
    }
  }

  const QByteArray separator = MBoxPrivate::mboxMessageSeparator( rawEntry );
  d->mAppendedEntries.append( separator );
  d->mAppendedEntries.append( rawEntry );
  if ( rawEntry[rawEntry.size() - 1] != '\n' ) {
    d->mAppendedEntries.append( "\n\n" );
  } else {
    d->mAppendedEntries.append( "\n" );
  }

  MBoxEntry resultEntry;
  resultEntry.d->mOffset = d->mInitialMboxFileSize + nextOffset;
  resultEntry.d->mMessageSize = rawEntry.size();
  resultEntry.d->mSeparatorSize = separator.size();
  d->mEntries << resultEntry;

  return resultEntry;
}

MBoxEntry::List MBox::entries( const MBoxEntry::List &deletedEntries ) const
{
  if ( deletedEntries.isEmpty() ) {
    // fast path
    return d->mEntries;
  }

  MBoxEntry::List result;

  foreach ( const MBoxEntry &entry, d->mEntries ) {
    if ( !deletedEntries.contains( entry ) ) {
      result << entry;
    }
  }

  return result;
}

QString MBox::fileName() const
{
  return d->mMboxFile.fileName();
}

bool MBox::load( const QString &fileName )
{
  if ( d->mFileLocked ) {
    return false;
  }

  d->initLoad( fileName );

  if ( !lock() ) {
    qDebug() << "Failed to lock";
    return false;
  }

  d->mInitialMboxFileSize = d->mMboxFile.size();  // AFTER the file has been locked

  QByteArray line;
  QByteArray prevSeparator;
  quint64 offs = 0; // The offset of the next message to read.

  while ( !d->mMboxFile.atEnd() ) {
    quint64 pos = d->mMboxFile.pos();

    line = d->mMboxFile.readLine();

    // if atEnd, use mail only if there was a separator line at all,
    // otherwise it's not a valid mbox
    if ( d->isMBoxSeparator( line ) ||
         ( d->mMboxFile.atEnd() && ( prevSeparator.size() != 0 ) ) ) {

      // if we are the at the file end, update pos to not forget the last line
      if ( d->mMboxFile.atEnd() ) {
        pos = d->mMboxFile.pos();
      }

      // Found the separator or at end of file, the message starts at offs
      quint64 msgSize = pos - offs;

      if ( pos > 0 ) {
        // This is not the separator of the first mail in the file. If pos == 0
        // than we matched the separator of the first mail in the file.
        MBoxEntry entry;
        entry.d->mOffset = offs;
        entry.d->mSeparatorSize = prevSeparator.size();
        entry.d->mMessageSize = msgSize - 1;

        // Don't add the separator size and the newline up to the message size.
        entry.d->mMessageSize -= prevSeparator.size() + 1;

        d->mEntries << entry;
      }

      if ( d->isMBoxSeparator( line ) ) {
        prevSeparator = line;
      }

      offs += msgSize; // Mark the beginning of the next message.
    }
  }

  // FIXME: What if unlock fails?
  // if no separator was found, the file is still valid if it is empty
  return unlock() && ( ( prevSeparator.size() != 0 ) || ( d->mMboxFile.size() == 0 ) );
}

bool MBox::lock()
{
  if ( d->mMboxFile.fileName().isEmpty() ) {
    return false; // We cannot lock if there is no file loaded.
  }

  // We can't load another file when the mbox currently is locked so if d->mFileLocked
  // is true atm just return true.
  if ( locked() ) {
    return true;
  }

  if ( d->mLockType == None ) {
    d->mFileLocked = true;
    if ( d->open() ) {
      d->startTimerIfNeeded();
      return true;
    }

    d->mFileLocked = false;
    return false;
  }

  QStringList args;
  int rc = 0;

  switch ( d->mLockType ) {
  case ProcmailLockfile:
    args << QLatin1String( "-l20" ) << QLatin1String( "-r5" );
    if ( !d->mLockFileName.isEmpty() ) {
      args << QString::fromLocal8Bit( QFile::encodeName( d->mLockFileName ) );
    } else {
      args << QString::fromLocal8Bit( QFile::encodeName( d->mMboxFile.fileName() +
                                                         QLatin1String( ".lock" ) ) );
    }

    rc = QProcess::execute( QLatin1String( "lockfile" ), args );
    if ( rc != 0 ) {
      qDebug() << "lockfile -l20 -r5 " << d->mMboxFile.fileName()
               << ": Failed (" << rc << ") switching to read only mode";
      d->mReadOnly = true; // In case the MBox object was created read/write we
      // set it to read only when locking failed.
    } else {
      d->mFileLocked = true;
    }
    break;

  case MuttDotlock:
    args << QString::fromLocal8Bit( QFile::encodeName( d->mMboxFile.fileName() ) );
    rc = QProcess::execute( QLatin1String( "mutt_dotlock" ), args );

    if ( rc != 0 ) {
      qDebug() << "mutt_dotlock " << d->mMboxFile.fileName()
               << ": Failed (" << rc << ") switching to read only mode";
      d->mReadOnly = true; // In case the MBox object was created read/write we
      // set it to read only when locking failed.
    } else {
      d->mFileLocked = true;
    }
    break;

  case MuttDotlockPrivileged:
    args << QLatin1String( "-p" )
         << QString::fromLocal8Bit( QFile::encodeName( d->mMboxFile.fileName() ) );
    rc = QProcess::execute( QLatin1String( "mutt_dotlock" ), args );

    if ( rc != 0 ) {
      qDebug() << "mutt_dotlock -p " << d->mMboxFile.fileName() << ":"
               << ": Failed (" << rc << ") switching to read only mode";
      d->mReadOnly = true;
    } else {
      d->mFileLocked = true;
    }
    break;

  case None:
    d->mFileLocked = true;
    break;
  default:
    break;
  }

  if ( d->mFileLocked ) {
    if ( !d->open() ) {
      const bool unlocked = unlock();
      Q_ASSERT( unlocked ); // If this fails we're in trouble.
      Q_UNUSED( unlocked );
    }
  }

  d->startTimerIfNeeded();
  return d->mFileLocked;
}

bool MBox::locked() const
{
  return d->mFileLocked;
}

static bool lessThanByOffset( const MBoxEntry &left, const MBoxEntry &right )
{
  return left.messageOffset() < right.messageOffset();
}

bool MBox::purge( const MBoxEntry::List &deletedEntries, QList<MBoxEntry::Pair> *movedEntries )
{
  if ( d->mMboxFile.fileName().isEmpty() ) {
    return false; // No file loaded yet.
  }

  if ( deletedEntries.isEmpty() ) {
    return true; // Nothing to do.
  }

  if ( !lock() ) {
    return false;
  }

  foreach ( const MBoxEntry &entry, deletedEntries ) {
    d->mMboxFile.seek( entry.messageOffset() );
    const QByteArray line = d->mMboxFile.readLine();

    if ( !d->isMBoxSeparator( line ) ) {
      qDebug() << "Found invalid separator at:" << entry.messageOffset();
      unlock();
      return false; // The file is messed up or the index is incorrect.
    }
  }

  // All entries are deleted, so just resize the file to a size of 0.
  if ( deletedEntries.size() == d->mEntries.size() ) {
    d->mEntries.clear();
    d->mMboxFile.resize( 0 );
    qDebug() << "Purge comleted successfully, unlocking the file.";
    return unlock();
  }

  qSort( d->mEntries.begin(), d->mEntries.end(), lessThanByOffset );
  quint64 writeOffset = 0;
  bool writeOffSetInitialized = false;
  MBoxEntry::List resultingEntryList;
  QList<MBoxEntry::Pair> tmpMovedEntries;

  quint64 origFileSize = d->mMboxFile.size();

  QListIterator<MBoxEntry> i( d->mEntries );
  while ( i.hasNext() ) {
    MBoxEntry entry = i.next();

    if ( deletedEntries.contains( entry ) && !writeOffSetInitialized ) {
      writeOffset = entry.messageOffset();
      writeOffSetInitialized = true;
    } else if ( writeOffSetInitialized &&
                writeOffset < entry.messageOffset() &&
                !deletedEntries.contains( entry ) ) {
      // The current message doesn't have to be deleted, but must be moved.
      // First determine the size of the entry that must be moved.
      quint64 entrySize = 0;
      if ( i.hasNext() ) {
        entrySize = i.next().messageOffset() - entry.messageOffset();
        i.previous(); // Go back to make sure that we also handle the next entry.
      } else {
        entrySize = origFileSize - entry.messageOffset();
      }

      Q_ASSERT( entrySize > 0 ); // MBox entries really cannot have a size <= 0;

      // we map the whole area of the file starting at the writeOffset up to the
      // message that have to be moved into memory. This includes eventually the
      // messages that are the deleted between the first deleted message
      // encountered and the message that has to be moved.
      quint64 mapSize = entry.messageOffset() + entrySize - writeOffset;

      // Now map writeOffSet + mapSize into mem.
      uchar *memArea = d->mMboxFile.map( writeOffset, mapSize );

      // Now read the entry that must be moved to writeOffset.
      quint64 startOffset = entry.messageOffset() - writeOffset;
      memmove( memArea, memArea + startOffset, entrySize );

      d->mMboxFile.unmap( memArea );

      MBoxEntry resultEntry;
      resultEntry.d->mOffset = writeOffset;
      resultEntry.d->mSeparatorSize = entry.separatorSize();
      resultEntry.d->mMessageSize = entry.messageSize();

      resultingEntryList << resultEntry;
      tmpMovedEntries << MBoxEntry::Pair( MBoxEntry( entry.messageOffset() ),
                                          MBoxEntry( resultEntry.messageOffset() ) );
      writeOffset += entrySize;
    } else if ( !deletedEntries.contains( entry ) ) {
      // Unmoved and not deleted entry, can only ocure before the first deleted
      // entry.
      Q_ASSERT( !writeOffSetInitialized );
      resultingEntryList << entry;
    }
  }

  // Chop off remaining entry bits.
  d->mMboxFile.resize( writeOffset );
  d->mEntries = resultingEntryList;

  qDebug() << "Purge comleted successfully, unlocking the file.";
  if ( movedEntries ) {
    *movedEntries = tmpMovedEntries;
  }
  return unlock(); // FIXME: What if this fails? It will return false but the
                   // file has changed.
}

QByteArray MBox::readRawMessage( const MBoxEntry &entry )
{
  const bool wasLocked = locked();
  if ( !wasLocked ) {
    if ( !lock() ) {
      return QByteArray();
    }
  }

  // TODO: Add error handling in case locking failed.

  quint64 offset = entry.messageOffset();

  Q_ASSERT( d->mFileLocked );
  Q_ASSERT( d->mMboxFile.isOpen() );
  Q_ASSERT( ( d->mInitialMboxFileSize + d->mAppendedEntries.size() ) > offset );

  QByteArray message;

  if ( offset < d->mInitialMboxFileSize ) {
    d->mMboxFile.seek( offset );

    QByteArray line = d->mMboxFile.readLine();

    if ( !d->isMBoxSeparator( line ) ) {
      qDebug() << "[MBox::readEntry] Invalid entry at:" << offset;
      if ( !wasLocked ) {
        unlock();
      }
      return QByteArray(); // The file is messed up or the index is incorrect.
    }

    line = d->mMboxFile.readLine();
    while ( !d->isMBoxSeparator( line ) ) {
      message += line;
      if ( d->mMboxFile.atEnd() ) {
        break;
      }
      line = d->mMboxFile.readLine();
    }
  } else {
    offset -= d->mInitialMboxFileSize;
    if ( offset > static_cast<quint64>( d->mAppendedEntries.size() ) ) {
      if ( !wasLocked ) {
        unlock();
      }
      return QByteArray();
    }

    QBuffer buffer( &( d->mAppendedEntries ) );
    buffer.open( QIODevice::ReadOnly );
    buffer.seek( offset );

    QByteArray line = buffer.readLine();

    if ( !d->isMBoxSeparator( line ) ) {
      qDebug() << "[MBox::readEntry] Invalid appended entry at:" << offset;
      if ( !wasLocked ) {
        unlock();
      }
      return QByteArray(); // The file is messed up or the index is incorrect.
    }

    line = buffer.readLine();
    while ( !d->isMBoxSeparator( line ) && !buffer.atEnd() ) {
      message += line;
      line = buffer.readLine();
    }
  }

  // Remove te last '\n' added by writeEntry.
  if ( message.endsWith( '\n' ) ) {
    message.chop( 1 );
  }

  MBoxPrivate::unescapeFrom( message.data(), message.size() );

  if ( !wasLocked ) {
    if ( !d->startTimerIfNeeded() ) {
      const bool unlocked = unlock();
      Q_ASSERT( unlocked );
      Q_UNUSED( unlocked );
    }
  }

  return message;
}

KMime::Message *MBox::readMessage( const MBoxEntry &entry )
{
  const QByteArray message = readRawMessage( entry );
  if ( message.isEmpty() ) {
    return 0;
  }

  KMime::Message *mail = new KMime::Message();
  mail->setContent( KMime::CRLFtoLF( message ) );
  mail->parse();

  return mail;
}

QByteArray MBox::readMessageHeaders( const MBoxEntry &entry )
{
  const bool wasLocked = d->mFileLocked;
  if ( !wasLocked ) {
    if (!lock()) {
       qDebug() << "Failed to lock";
       return QByteArray();
    }
  }

  const quint64 offset = entry.messageOffset();

  Q_ASSERT( d->mFileLocked );
  Q_ASSERT( d->mMboxFile.isOpen() );
  Q_ASSERT( ( d->mInitialMboxFileSize + d->mAppendedEntries.size() ) > offset );

  QByteArray headers;
  if ( offset < d->mInitialMboxFileSize ) {
    d->mMboxFile.seek( offset );
    QByteArray line = d->mMboxFile.readLine();

    while ( line[0] != '\n' && !d->mMboxFile.atEnd() ) {
      headers += line;
      line = d->mMboxFile.readLine();
    }
  } else {
    QBuffer buffer( &( d->mAppendedEntries ) );
    buffer.open( QIODevice::ReadOnly );
    buffer.seek( offset - d->mInitialMboxFileSize );
    QByteArray line = buffer.readLine();

    while ( line[0] != '\n' && !buffer.atEnd() ) {
      headers += line;
      line = buffer.readLine();
    }
  }

  if ( !wasLocked ) {
    unlock();
  }

  return headers;
}

bool MBox::save( const QString &fileName )
{
  if ( !fileName.isEmpty() && QUrl( fileName ).toLocalFile() != d->mMboxFile.fileName() ) {
    if ( !d->mMboxFile.copy( fileName ) ) {
      return false;
    }

    if ( d->mAppendedEntries.size() == 0 ) {
      return true; // Nothing to do
    }

    QFile otherFile( fileName );
    Q_ASSERT( otherFile.exists() );
    if ( !otherFile.open( QIODevice::ReadWrite ) ) {
      return false;
    }

    otherFile.seek( d->mMboxFile.size() );
    otherFile.write( d->mAppendedEntries );

    // Don't clear mAppendedEntries and don't update mInitialFileSize. These
    // are still valid for the original file.
    return true;
  }

  if ( d->mAppendedEntries.size() == 0 ) {
    return true; // Nothing to do.
  }

  if ( !lock() ) {
    return false;
  }

  Q_ASSERT( d->mMboxFile.isOpen() );

  d->mMboxFile.seek( d->mMboxFile.size() );
  d->mMboxFile.write( d->mAppendedEntries );
  d->mAppendedEntries.clear();
  d->mInitialMboxFileSize = d->mMboxFile.size();

  return unlock();
}

bool MBox::setLockType( LockType ltype )
{
  if ( d->mFileLocked ) {
    qDebug() << "File is currently locked.";
    return false; // Don't change the method if the file is currently locked.
  }

  switch ( ltype ) {
    case ProcmailLockfile:
      if ( QStandardPaths::findExecutable( QLatin1String( "lockfile" ) ).isEmpty() ) {
        qDebug() << "Could not find the lockfile executable";
        return false;
      }
      break;
    case MuttDotlock: // fall through
    case MuttDotlockPrivileged:
      if ( QStandardPaths::findExecutable( QLatin1String( "mutt_dotlock" ) ).isEmpty() ) {
        qDebug() << "Could not find the mutt_dotlock executable";
        return false;
      }
      break;
    default:
      break; // We assume fcntl available and lock_none doesn't need a check.
  }

  d->mLockType = ltype;
  return true;
}

void MBox::setLockFile( const QString &lockFile )
{
  d->mLockFileName = lockFile;
}

void MBox::setUnlockTimeout( int msec )
{
  d->mUnlockTimer.setInterval( msec );
}

bool MBox::unlock()
{
  if ( d->mLockType == None && !d->mFileLocked ) {
    d->mFileLocked = false;
    d->mMboxFile.close();
    return true;
  }

  int rc = 0;
  QStringList args;

  switch ( d->mLockType ) {
    case ProcmailLockfile:
      // QFile::remove returns true on succes so negate the result.
      if ( !d->mLockFileName.isEmpty() ) {
        rc = !QFile( d->mLockFileName ).remove();
      } else {
        rc = !QFile( d->mMboxFile.fileName() + QLatin1String( ".lock" ) ).remove();
      }
      break;

    case MuttDotlock:
      args << QLatin1String( "-u" )
           << QString::fromLocal8Bit( QFile::encodeName( d->mMboxFile.fileName() ) );
      rc = QProcess::execute( QLatin1String( "mutt_dotlock" ), args );
      break;

    case MuttDotlockPrivileged:
      args << QLatin1String( "-u" ) << QLatin1String( "-p" )
           << QString::fromLocal8Bit( QFile::encodeName( d->mMboxFile.fileName() ) );
      rc = QProcess::execute( QLatin1String( "mutt_dotlock" ), args );
      break;

    case None: // Fall through.
    default:
      break;
  }

  if ( rc == 0 ) { // Unlocking succeeded
    d->mFileLocked = false;
  }

  d->mMboxFile.close();

  return !d->mFileLocked;
}
