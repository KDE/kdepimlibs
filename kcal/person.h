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

  @author Cornelius Schumacher
*/

#ifndef KCAL_PERSON_H
#define KCAL_PERSON_H

#include <QString>

#include "kcal.h"

namespace KCal {

/**
  @brief
  Represents a person.

  This class represents a person, with a name and an email address.
  It supports the "FirstName LastName <emailaddress>" format.
*/
class KCAL_EXPORT Person
{
  public:
    /**
      Constructs a person
    */
    Person();

    /**
      Constructs a person with name and email taken 
      from @p fullName. 

      @param fullName the name and email of this #Person in
      the form FirstName LastName <email>
    */
    Person( const QString &fullName );

    /**
      Constructs a person with the name @p name and 
      the email @p email

      @param name the name of this #Person
      @param email the email of this #Person
    */
    Person( const QString &name, const QString &email );

    /**
      Returns true if the name and email is empty
    */
    bool isEmpty() const;

    /**
      Returns the full name of this #Person
    */
    QString fullName( ) const;

    /**
      Sets the name of this #Person to @p name

      @param name the name for this #Person

      @see name()
    */
    void setName(const QString &name);

    /**
      Returns the name of this #Person

      @see setName()
    */
    QString name() const;

    /**
      Sets the email address for this #Person to @p email

      @param email the email for this #Person

      @see email()
    */   
    void setEmail(const QString &email);

    /**
      Returns the email for this #Person

      @see setEmail()
    */
    QString email() const;

  private:
    class Private;
    Private *d;
};

/**
  Compares @p p1 and @p p2 for equality
  
  @param p1 the first person to compare
  @param p2 the second person to compare
*/
bool operator==( const Person& p1, const Person& p2 );

}

#endif
