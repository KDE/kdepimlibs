/* dataprovider.cpp
   Copyright (C) 2004 Klar�vdalens Datakonsult AB

   This file is part of QGPGME.

   QGPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   QGPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with QGPGME; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA. */

// -*- c++ -*-

#include <qgpgme/dataprovider.h>

#include <QIODevice>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

using namespace QGpgME;

//
//
// QByteArrayDataProvider
//
//

static bool resizeAndInit( QByteArray & ba, size_t newSize ) {
  const size_t oldSize = ba.size();
  ba.resize( newSize );
  const bool ok = ( newSize == static_cast<size_t>( ba.size() ) );
  if ( ok )
    memset( ba.data() + oldSize, 0, newSize - oldSize );
  return ok;
}

QByteArrayDataProvider::QByteArrayDataProvider()
  : GpgME::DataProvider(), mOff( 0 ) {}

QByteArrayDataProvider::QByteArrayDataProvider( const QByteArray & initialData )
  : GpgME::DataProvider(), mArray( initialData ), mOff( 0 ) {}

QByteArrayDataProvider::~QByteArrayDataProvider() {}

ssize_t QByteArrayDataProvider::read( void * buffer, size_t bufSize ) {
#ifndef NDEBUG
  //qDebug( "QByteArrayDataProvider::read( %p, %d )", buffer, bufSize );
#endif
  if ( bufSize == 0 )
    return 0;
  if ( !buffer ) {
    errno = EINVAL;
    return -1;
  }
  if ( mOff >= mArray.size() )
    return 0; // EOF
  size_t amount = qMin( bufSize, static_cast<size_t>( mArray.size() - mOff ) );
  assert( amount > 0 );
  memcpy( buffer, mArray.data() + mOff, amount );
  mOff += amount;
  return amount;
}

ssize_t QByteArrayDataProvider::write( const void * buffer, size_t bufSize ) {
#ifndef NDEBUG
    qDebug( "QByteArrayDataProvider::write( %p, %lu )", buffer, static_cast<unsigned long>( bufSize ) );
#endif
  if ( bufSize == 0 )
    return 0;
  if ( !buffer ) {
    errno = EINVAL;
    return -1;
  }
  if ( mOff >= mArray.size() )
    resizeAndInit( mArray, mOff + bufSize );
  if ( mOff >= mArray.size() ) {
    errno = EIO;
    return -1;
  }
  assert( bufSize <= static_cast<size_t>(mArray.size()) - mOff );
  memcpy( mArray.data() + mOff, buffer, bufSize );
  mOff += bufSize;
  return bufSize;
}

off_t QByteArrayDataProvider::seek( off_t offset, int whence ) {
#ifndef NDEBUG
  qDebug( "QByteArrayDataProvider::seek( %d, %d )", int(offset), whence );
#endif
  int newOffset = mOff;
  switch ( whence ) {
  case SEEK_SET:
    newOffset = offset;
    break;
  case SEEK_CUR:
    newOffset += offset;
    break;
  case SEEK_END:
    newOffset = mArray.size() + offset;
    break;
  default:
    errno = EINVAL;
    return (off_t)-1;
  }
  return mOff = newOffset;
}

void QByteArrayDataProvider::release() {
#ifndef NDEBUG
  qDebug( "QByteArrayDataProvider::release()" );
#endif
  mArray = QByteArray();
}


//
//
// QIODeviceDataProvider
//
//

QIODeviceDataProvider::QIODeviceDataProvider( const boost::shared_ptr<QIODevice> & io )
  : GpgME::DataProvider(),
    mIO( io )
{
  assert( mIO );
}

QIODeviceDataProvider::~QIODeviceDataProvider() {}

bool QIODeviceDataProvider::isSupported( Operation op ) const {
    switch ( op ) {
    case Read:    return mIO->isReadable();
    case Write:   return mIO->isWritable();
    case Seek:    return !mIO->isSequential();
    case Release: return true;
    default:      return false;
    }
}

ssize_t QIODeviceDataProvider::read( void * buffer, size_t bufSize ) {
#ifndef NDEBUG
  //qDebug( "QIODeviceDataProvider::read( %p, %d )", buffer, bufSize );
#endif
  if ( bufSize == 0 )
    return 0;
  if ( !buffer ) {
    errno = EINVAL;
    return -1;
  }
  return mIO->read( static_cast<char*>(buffer), bufSize );
}

ssize_t QIODeviceDataProvider::write( const void * buffer, size_t bufSize ) {
#ifndef NDEBUG
  qDebug( "QIODeviceDataProvider::write( %p, %lu )", buffer, static_cast<unsigned long>( bufSize ) );
#endif
  if ( bufSize == 0 )
    return 0;
  if ( !buffer ) {
     errno = EINVAL;
     return -1;
  }
  return mIO->write( static_cast<const char*>(buffer), bufSize );
}

off_t QIODeviceDataProvider::seek( off_t offset, int whence ) {
#ifndef NDEBUG
  qDebug( "QIODeviceDataProvider::seek( %d, %d )", int(offset), whence );
#endif
  if ( mIO->isSequential() ) {
    errno = ESPIPE;
    return (off_t)-1;
  }
  qint64 newOffset = mIO->pos();
  switch ( whence ) {
  case SEEK_SET:
    newOffset = offset;
    break;
  case SEEK_CUR:
    newOffset += offset;
    break;
  case SEEK_END:
    newOffset = mIO->size() + offset;
    break;
  default:
    errno = EINVAL;
    return (off_t)-1;
  }
  if ( !mIO->seek( newOffset ) ) {
    errno = EINVAL;
    return (off_t)-1;
  }
  return newOffset;
}

void QIODeviceDataProvider::release() {
#ifndef NDEBUG
  qDebug( "QIODeviceDataProvider::release()" );
#endif
  mIO->close();
}
