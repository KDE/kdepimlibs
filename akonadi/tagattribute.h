/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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
*/

#ifndef AKONADI_TAGATTRIBUTE_H
#define AKONADI_TAGATTRIBUTE_H

#include <QColor>
#include <QFont>

#include <akonadi/attribute.h>

class KIcon;

namespace Akonadi {

/**
 * @short Attribute that stores the properties that are used to display a tag.
 *
 * @since 4.13
 */
class AKONADI_EXPORT TagAttribute : public Attribute
{
  public:
    TagAttribute();

    ~TagAttribute();

    /**
     * Sets the @p name that should be used for display.
     */
    void setDisplayName( const QString &name );

    /**
     * Returns the name that should be used for display.
     * Users of this should fall back to Collection::name() if this is empty.
     */
    QString displayName() const;

    /**
     * Sets the icon @p name for the default icon.
     */
    void setIconName( const QString &name );

    /**
     * Returns the icon name of the icon returned by icon().
     */
    QString iconName() const;


    void setBackgroundColor( const QColor &color );
    QColor backgroundColor() const;
    void setTextColor( const QColor &color );
    QColor textColor() const;
    void setFont( const QFont &font );
    QFont font() const;
    void setInToolbar(bool);
    bool inToolbar();
    void setShortcut(const QString &);
    QString shortcut();

    /* reimpl */
    QByteArray type() const;
    TagAttribute* clone() const;
    QByteArray serialized() const;
    void deserialize( const QByteArray &data );

  private:
    TagAttribute(const TagAttribute &);
    TagAttribute &operator=(const TagAttribute &);
    //@cond PRIVATE
    class Private;
    Private* const d;
    //@endcond
};

}

#endif
