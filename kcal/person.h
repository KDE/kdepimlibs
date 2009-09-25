/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
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

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCAL_PERSON_H
#define KCAL_PERSON_H

#include <QtCore/QString>
#include <QtCore/QHash>

#include "kcal_export.h"

namespace KCal {

/**
  @brief
  Represents a person, by name ane email address.

  This class represents a person, with a name and an email address.
  It supports the "FirstName LastName\ <mail@domain\>" format.
*/
class KCAL_EXPORT Person
{
  public:
    /**
      Constructs a blank person.
    */
    Person();

    /**
      Constructs a person with name and email address taken from @p fullName.

      @param fullName is the name and email of the person in
      the form "FirstName LastName \<mail@domain\>".
    */
    static Person fromFullName( const QString &fullName );

    /**
      Constructs a person with name and email address taken from @p fullName.

      @param fullName is the name and email of the person in
      the form "FirstName LastName \<mail@domain\>".

      @deprecated use fromFullName() instead.
    */
    KDE_CONSTRUCTOR_DEPRECATED explicit Person( const QString &fullName );

    /**
      Constructs a person with the name @p name and email address @p email.

      @param name is the name of this person.
      @param email is the email address of this person.
    */
    Person( const QString &name, const QString &email );

    /**
       Constructs a person as a copy of another person object.

       @param person is the person to copy.
     */
    Person( const Person &person );

    /**
      Destroys a person.
    */
    ~Person();

    /**
      Returns true if the person name and email address are empty.
    */
    bool isEmpty() const;

    /**
      Returns the full name of this person.
    */
    QString fullName( ) const;

    /**
      Sets the name of the person to @p name.

      @param name is the name of this person.

      @see name()
    */
    void setName( const QString &name );

    /**
      Returns the person name string.

      @see setName()
    */
    QString name() const;

    /**
      Sets the email address for this person to @p email.

      @param email is the email address for this person.

      @see email()
    */
    void setEmail( const QString &email );

    /**
      Returns the email address for this person.

      @see setEmail()
    */
    QString email() const;

    /**
      Compares this with @p person for equality.

      @param person is the person to compare.
    */
    //KDE5: make const for all
#if defined(Q_CC_MSVC)
    bool operator==( const Person &person ) const;
#else
    bool operator==( const Person &person ); //krazy:exclude=operators
#endif
    /**
      Sets this person equal to @p person.

      @param person is the person to copy.
    */
    Person &operator=( const Person &person );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
   //@endcond
};

}

/*
  Return a hash value for a Person argument.
  @param key is a Person.
  @since 4.4
*/
inline uint qHash( const KCal::Person &key ) //krazy:exclude=inline
{
  return qHash( key.fullName() );
}

#endif
