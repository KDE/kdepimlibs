/*
    ktnefattach.h

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
 * defines the KTNEFAttach class.
 *
 * @author Michael Goffioul
 */

#ifndef KTNEFATTACH_H
#define KTNEFATTACH_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "ktnefpropertyset.h"
#include "ktnef_export.h"

namespace KTnef { class KTNEFProperty; }

namespace KTnef {

/**
 * @brief
 * Represents a @acronym TNEF attachment.
 */
class KTNEF_EXPORT KTNEFAttach : public KTNEFPropertySet
{
  public:
    /**
     * The different attachment parsed states.
     */
    enum ParseState {
      Unparsed = 0x0000,    /**< Unparsed */
      TitleParsed = 0x0001, /**< The title is parsed */
      DataParsed = 0x0002,  /**< The data is parsed */
      InfoParsed = 0x0004   /**< The info is parsed */
    };

    /**
     * Constructs a @acronym TNEF attachment.
     */
    KTNEFAttach();

    /**
     * Destroys the @acronym TNEF attachment.
     */
    ~KTNEFAttach();

    /**
     * Sets the #TitleParsed flag for this attachment.
     */
    void setTitleParsed();

    /**
     * Sets the #DataParsed flag for this attachment.
     */
    void setDataParsed();

    /**
     * Unsets the #DataParsed flag for this attachment.
     */
    void unsetDataParser();

    /**
     * Sets the #InfoParsed flag for this attachment.
     */
    void setInfoParsed();

    /**
     * Returns true if the #TitleParsed flag is set; else returns false.
     */
    bool titleParsed() const;

    /**
     * Returns true if the ParseState::DataParsed flag is set; else returns false.
     */
    bool dataParsed() const;

    /**
     * Returns true if the #InfoParsed flag is set; else returns false.
     */
    bool infoParsed() const;

    /**
     * Sets/Unsets the attachment state according to the @p state flag
     * must be a #ParseState type.
     *
     * @param state a #ParseState type.
     * @return true if the state is turned-on; else returns false.
     */
    bool checkState( int state ) const;

    /**
     * Sets the offset value of this attachment to @p offset.
     *
     * @param offset is the attachment offset to set.
     */
    void setOffset( int offset );

    /**
     * Returns the offset value of the attachment.
     */
    int offset() const;

    /**
     * Sets the size of the attachment to @p size.
     *
     * @param size is the attachment size to set.
     */
    void setSize( int size );

    /**
     * Returns the size of the attachment.
     */
    int size() const;

    /**
     * Sets the display size of the attachment to @p size.
     *
     * @param size is the attachment display size to set.
     */
    void setDisplaySize( int size );

    /**
     * Returns the display size of the attachment.
     */
    int displaySize() const;

    /**
     * Sets the name of this attachment to @p str.
     *
     * @param str is attachment name to set.
     */
    void setName( const QString &str );

    /**
     * Returns the name of the attachment.
     */
    QString name() const;

    /**
     * Sets the index of this attachment to @p indx.
     *
     * @param indx is the attachment index to set.
     */
    void setIndex( int indx );

    /**
     * Returns the index of the attachment.
     */
    int index() const;

    /**
     * Sets the filename of this attachment to @p str.
     *
     * @param str is the attachment filename to set.
     */
    void setFileName( const QString &str );

    /**
     * Returns the filename of the attachment.
     */
    QString fileName() const;

    /**
     * Sets the display name of this attachment to @p str.
     *
     * @param str is the attachment display name to set.
     */
    void setDisplayName( const QString &str );

    /**
     * Returns the display name of the attachment.
     */
    QString displayName() const;

    /**
     * Sets the @acronym MIME tag of this attachment to @p str.
     *
     * @param str is the attachment @acronym MIME tag to set.
     */
    void setMimeTag( const QString &str );

    /**
     * Returns the @acronym MIME tag of the attachment.
     */
    QString mimeTag() const;

    /**
     * Sets the filename extension of this attachment to @p str.
     *
     * @param str is the attachment filename extension to set.
     */
    void setExtension( const QString &str );

    /**
     * Returns the filename extension of the attachment.
     */
    QString extension() const;

  private:
    //@cond PRIVATE
    class AttachPrivate;
    AttachPrivate *const d;
    //@endcond

    Q_DISABLE_COPY( KTNEFAttach )
};

}
#endif
