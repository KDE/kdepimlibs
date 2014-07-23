/*
    ktnefproperty.h

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
 * defines the KTNEFProperty class.
 *
 * @author Michael Goffioul
 */

#ifndef KTNEFPROPERTY_H
#define KTNEFPROPERTY_H

#include <QtCore/QVariant>
#include <QtCore/QString>
#include "ktnef_export.h"

namespace KTnef
{

/**
 * @brief
 * Interface for setting @acronym MAPI properties.
 */
class KTNEF_EXPORT KTNEFProperty
{
public:
    /**
     * The different @acronym MAPI types.
     */
    enum MAPIType {
        UInt16  = 0x0002, /**< 16-bit unsigned integer */
        ULong   = 0x0003, /**< unsigned long integer */
        Float   = 0x0004, /**< single precision floating point */
        Double  = 0x0005, /**< double precision floating point */
        Boolean = 0x000B, /**< a boolean value */
        Object  = 0x000D, /**< an object */
        Time    = 0x0040, /**< a time value */
        String8 = 0x001E, /**< a string of 8 characters */
        UString = 0x001F, /**< a string of characters */
        Binary  = 0x0102  /**< a binary value */
    };

    /**
     * Constructs a @acronym TNEF property.
     */
    KTNEFProperty();

    /**
     * Constructs a @acronym TNEF property initialized with specified settings.
     *
     * @param key_ is the property key.
     * @param type_ is the property type.
     * @param value_ is the property value.
     * @param name_ is the property name.
     */
    KTNEFProperty(int key_, int type_, const QVariant &value_,
                  const QVariant &name_ = QVariant());

    /**
     * Constructs a @acronym TNEF property with settings from another property.
     *
     * @param p is a #KTNEFProperty.
     */
    KTNEFProperty(const KTNEFProperty &p);

    /**
     * Destroys the property.
     */
    ~KTNEFProperty();

    KTNEFProperty &operator=(const KTNEFProperty &other);

    /**
     * Returns the key string of the property.
     *
     * @return the key string.
     */
    QString keyString() const;

    /**
     * Returns the value string of the property.
     *
     * @return the value string.
     */
    QString valueString() const;

    /**
     * Creates a formatted string from the value of the property.
     *
     * @param v is the property value.
     * @param beautify if true uses a prettier format
     *
     * @return the formatted value string.
     */
    static QString formatValue(const QVariant &v, bool beautify = true);

    /**
     * Returns the integer key of the property.
     *
     * @return the property key.
     */
    int key() const;

    /**
     * Returns the integer type of the property.
     *
     * @return the property type.
     */
    int type() const;

    /**
     * Returns the value of the property.
     *
     * @return the property value.
     */
    QVariant value() const;

    /**
     * Returns the name of the property.
     *
     * @return the property name.
     */
    QVariant name() const;

    /**
     * Determines if the property is a vector type.
     *
     * @returns true if the property is a vector type; otherwise false.
     */
    bool isVector() const;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}
#endif
