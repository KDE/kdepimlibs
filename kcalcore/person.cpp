/*
  This file is part of the kcalcore library.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  defines the Person class.

  @brief
  Represents a person, by name and email address.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#include "person.h"

#include "kpimutils/email.h"

#include <kdebug.h>

#include <QtCore/QRegExp>

using namespace KCalCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalCore::Person::Private
{
  public:
    Private() : mCount( 0 ) {}
    QString mName;   // person name
    QString mEmail;  // person email address
    int mCount;      // person reference count
};
//@endcond

Person::Person() : d( new KCalCore::Person::Private )
{
}

/** static */
Person::Ptr Person::fromFullName( const QString &fullName )
{
  QString email, name;
  KPIMUtils::extractEmailAddressAndName( fullName, email, name );
  return Person::Ptr( new Person( name, email ) );
}

Person::Person( const QString &name, const QString &email )
  : d( new KCalCore::Person::Private )
{
  d->mName = name;
  d->mEmail = email;
}

Person::Person( const Person &person )
  : d( new KCalCore::Person::Private( *person.d ) )
{
}

Person::~Person()
{
  delete d;
}

bool KCalCore::Person::operator==( const Person &person ) const
{
  return
    d->mName == person.d->mName &&
    d->mEmail == person.d->mEmail;
}

Person &KCalCore::Person::operator=( const Person &person )
{
  // check for self assignment
  if ( &person == this ) {
    return *this;
  }

  *d = *person.d;
  return *this;
}

QString Person::fullName() const
{
  if ( d->mName.isEmpty() ) {
    return d->mEmail;
  } else {
    if ( d->mEmail.isEmpty() ) {
      return d->mName;
    } else {
      // Taken from KABC::Addressee::fullEmail
      QString name = d->mName;
      QRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
      bool weNeedToQuote = name.indexOf( needQuotes ) != -1;
      if ( weNeedToQuote ) {
        if ( name[0] != '"' ) {
          name.prepend( '"' );
        }
        if ( name[ name.length()-1 ] != '"' ) {
          name.append( '"' );
        }
      }
      return name + " <" + d->mEmail + '>';
    }
  }
}

QString Person::name() const
{
  return d->mName;
}

QString Person::email() const
{
  return d->mEmail;
}

bool Person::isEmpty() const
{
  return d->mEmail.isEmpty() && d->mName.isEmpty();
}

void Person::setName( const QString &name )
{
  d->mName = name;
}

void Person::setEmail( const QString &email )
{
  if ( email.startsWith( QLatin1String( "mailto:" ), Qt::CaseInsensitive ) ) {
    d->mEmail = email.mid( 7 );
  } else {
    d->mEmail = email;
  }
}

bool Person::isValidEmail( const QString &email )
{
// PENDING(kdab) Review
// can we replace this with a call to the email static functions in kpimutils? should we do that?
  int pos = email.lastIndexOf( "@" );
  return ( pos > 0 ) && ( email.lastIndexOf( "." ) > pos ) && ( ( email.length() - pos ) > 4 );
}

void Person::setCount( int count )
{
  d->mCount = count;
}

int Person::count() const
{
  return d->mCount;
}

uint qHash( const KCalCore::Person &key )
{
  return qHash( key.fullName() );
}

QDataStream& KCalCore::operator<<( QDataStream& stream, const KCalCore::Person& person )
{
  return stream << person.d->mName << person.d->mEmail << person.d->mCount;
}

QDataStream& KCalCore::operator>>( QDataStream& stream, Person& person )
{
  return stream >> person.d->mName >>person.d->mEmail >> person.d->mCount;
}

