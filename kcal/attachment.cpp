/*
    This file is part of the kcal library.

    Copyright (c) 2002 Michael Brade <brade@kde.org>

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
  This file is part of the API for handling calendar data and
  defines the Attachment class.

  @author Michael Brade
*/

#include "attachment.h"

#include <QtCore/QByteArray>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Attachment::Private
{
  public:
    QByteArray mDataCache;
    uint mSize;
    QString mMimeType;
    QString mData;
    bool mBinary;
    bool mShowInline;
    bool mLocal;
    QString mLabel;
};
//@endcond

Attachment::Attachment( const Attachment &attachment )
  : d( new Attachment::Private )
{
  d->mMimeType = attachment.d->mMimeType;
  d->mData = attachment.d->mData;
  d->mBinary = attachment.d->mBinary;
  d->mShowInline = attachment.d->mShowInline;
  d->mLabel = attachment.d->mLabel;
  d->mLocal = attachment.d->mLocal;
}

Attachment::Attachment( const QString &uri, const QString &mime )
  : d( new Attachment::Private )
{
  d->mMimeType = mime;
  d->mData = uri;
  d->mBinary = false;
  d->mShowInline = false;
  d->mLocal = false;
  d->mLabel.clear();
}

Attachment::Attachment( const char *base64, const QString &mime )
  : d( new Attachment::Private )
{
  d->mMimeType = mime;
  d->mData = QString::fromUtf8( base64 );
  d->mBinary = true;
  d->mShowInline = false;
  d->mLabel.clear();
}

Attachment::~Attachment()
{
  delete d;
}

bool Attachment::isUri() const
{
  return !d->mBinary;
}

QString Attachment::uri() const
{
  if ( !d->mBinary ) {
    return d->mData;
  } else {
    return QString();
  }
}

void Attachment::setUri( const QString &uri )
{
  d->mData = uri;
  d->mBinary = false;
}

bool Attachment::isBinary() const
{
  return d->mBinary;
}

char *Attachment::data() const
{
  if ( d->mBinary ) {
    return d->mData.toUtf8().data();
  } else {
    return 0;
  }
}

QByteArray &Attachment::decodedData() const
{
  if ( d->mDataCache.isNull() ) {
    d->mDataCache = QByteArray::fromBase64( d->mData.toUtf8() );
  }

  return d->mDataCache;
}

void Attachment::setDecodedData( const QByteArray &data )
{
  setData( data.toBase64() );
  d->mDataCache = data;
}

void Attachment::setData( const char *base64 )
{
  d->mData = QString::fromUtf8( base64 );
  d->mBinary = true;
  d->mDataCache = QByteArray();
  d->mSize = 0;
}

uint Attachment::size() const
{
  if ( isUri() ) {
    return 0;
  }
  if ( !d->mSize ) {
    d->mSize = decodedData().size();
  }

  return d->mSize;
}

QString Attachment::mimeType() const
{
  return d->mMimeType;
}

void Attachment::setMimeType( const QString &mime )
{
  d->mMimeType = mime;
}

bool Attachment::showInline() const
{
  return d->mShowInline;
}

void Attachment::setShowInline( bool showinline )
{
  d->mShowInline = showinline;
}

QString Attachment::label() const
{
  return d->mLabel;
}

void Attachment::setLabel( const QString &label )
{
  d->mLabel = label;
}

bool Attachment::isLocal() const
{
  return d->mLocal;
}

void Attachment::setLocal( bool local )
{
  d->mLocal = local;
}
