/*
    ktnefpropertyset.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

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
 * @file
 * This file is part of the API for handling TNEF data and
 * defines the KTNEFPropertySet class.
 *
 * @author Michael Goffioul
 */

#ifndef KTNEFPROPERTYSET_H
#define KTNEFPROPERTYSET_H

#include <QtCore/QMap>
#include <QtCore/QVariant>
#include "ktnef_export.h"

namespace KTnef { class KTNEFProperty; }

namespace KTnef {

/**
 * @brief
 * Interface for setting @acronym MAPI properties and @acronym TNEF attributes.
 */
class KTNEF_EXPORT KTNEFPropertySet
{
  public:
    /**
      Constructor.
    */
    KTNEFPropertySet();

    /**
      Destructor.
    */
    ~KTNEFPropertySet();

    /**
      Adds a @acronym MAPI property.

      @param key is the property key.
      @param type is the property type.
      @param value is the property value.
      @param name is the property name.
      @param overwrite if true, then remove the property if it already exists.
    */
    void addProperty( int key, int type, const QVariant &value,
                      const QVariant &name=QVariant(), bool overwrite=false );

    /**
      Finds a property by @p key, returning a formatted value.

      @param key is the property key.
      @param fallback is the fallback formatted value to use if the @p key
      is not found.
      @param convertToUpper if true, then return the formatted value in all
      upper case characters.

      @return a formatted value string.
    */
    QString findProp( int key, const QString &fallback=QString(),
                      bool convertToUpper=false ) const;

    /**
      Finds a property by @p name, returning a formatted value.

      @param name is the property name.
      @param fallback is the fallback formatted value to use if the @p name
      is not found.
      @param convertToUpper if true, then return the formatted value in all
      upper case characters.

      @return a formatted value string.
    */
    QString findNamedProp( const QString &name,
                           const QString &fallback=QString(),
                           bool convertToUpper=false ) const;

    /**
      Returns a #QMap of all (key,@acronym MAPI) properties
    */
    QMap<int,KTNEFProperty*>& properties();

    /**
      Returns a #QMap of all (key,@acronym MAPI) properties
    */
    const QMap<int,KTNEFProperty*>& properties() const; //krazy:exclude=constref

    /**
      Returns the property associcated with the specified @p key.

      @param key is the property key.

      @return the property.q
    */
    QVariant property( int key ) const;

    /**
      Adds a @acronym TNEF attribute.

      @param key is the attribute key.
      @param type is the attribute type.
      @param value is the attribute value.
      @param overwrite if true, then remove the attribute if it already exists.
    */
    void addAttribute( int key, int type, const QVariant &value,
                       bool overwrite=false );

    /**
      Returns a #QMap of all (key,@acronym TNEF) attributes.
    */
    QMap<int,KTNEFProperty*>& attributes();

    /**
      Returns a #QMap of all (key,@acronym TNEF) attributes.
    */
    const QMap<int,KTNEFProperty*>& attributes() const; //krazy:exclude=constref

    /**
      Returns the attribute associcated with the specified @p key.

      @param key is the @acronym TNEF key.

      @return the attribute associated with the key.
    */
    QVariant attribute( int key ) const;

    /**
      Clears the @acronym MAPI and @acronym TNEF maps.

      @param deleteAll if true, delete the map memory as well.
    */
    void clear( bool deleteAll=false );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    Q_DISABLE_COPY( KTNEFPropertySet )
};

}
#endif
