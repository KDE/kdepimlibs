/*
    This file is part of the kcal library.

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

  @author Cornelius Schumacher
  @author Reinhold Kainhofer
*/

#include "person.h"

#include "kpimutils/email.h"

#include <QtCore/QRegExp>

#include <kdebug.h>
#include <klocale.h>

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::Person::Private
{
  public:
    QString mName;   // person name
    QString mEmail;  // person email address
};
//@endcond

Person::Person() : d( new KCal::Person::Private )
{
}

Person::Person( const QString &fullName ) : d( new KCal::Person::Private )
{
  KPIMUtils::extractEmailAddressAndName( fullName, d->mEmail, d->mName );
}

Person::Person( const QString &name, const QString &email )
  : d( new KCal::Person::Private )
{
  d->mName = name;
  d->mEmail = email;
}

Person::Person( const Person &person )
  : d( new KCal::Person::Private( *person.d ) )
{
}

Person::~Person()
{
  delete d;
}

bool KCal::Person::operator==( const Person &person )
{
  return
    d->mName == person.d->mName &&
    d->mEmail == person.d->mEmail;
}

Person &KCal::Person::operator=( const Person &person )
{
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
  if ( email.startsWith( "mailto:", Qt::CaseInsensitive ) ) {
    d->mEmail = email.mid( 7 );
  } else {
    d->mEmail = email;
  }
}
