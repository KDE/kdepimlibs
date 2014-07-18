/*
    ktnefparser.h

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
 * defines the KTNEFParser class.
 *
 * @author Michael Goffioul
 */

#ifndef KTNEFPARSER_H
#define KTNEFPARSER_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QIODevice>
#include "ktnef_export.h"

namespace KTnef {
class KTNEFAttach;
class KTNEFMessage;
class KTNEFProperty;
}

namespace KTnef {

/**
 * @brief
 * Provides an @acronym TNEF parser.
 */
class KTNEF_EXPORT KTNEFParser
{
public:
    /**
      Constructs a @acronym TNEF parser object.
    */
    KTNEFParser();

    /**
      Destroys the @acronym TNEF parser object.
     */
    ~KTNEFParser();

    /**
      Opens the @p filename for parsing.

      @param filename is the name of the file to open.
      @return true if the open succeeded; otherwise false.
    */
    bool openFile( const QString &filename ) const;

    /**
      Opens the #QIODevice @p device for parsing.

      @param device is the #QIODevice to open.
      @return true if the open succeeded; otherwise false.
    */
    bool openDevice( QIODevice *device );

    /**
      Extracts a @acronym TNEF attachment having filename @p filename
      into the default directory.

      @param filename is the name of the file to extract the attachment into.
      @return true if the extraction succeeds; otherwise false.
    */
    bool extractFile( const QString &filename ) const;

    /**
      Extracts a @acronym TNEF attachment having filename @p filename
      into the directory @p dirname.

      @param filename is the name of the file to extract the attachment into.
      @param dirname is the name of the directory where the @p filename
      should be written.

      @return true if the extraction succeeds; otherwise false.
    */
    bool extractFileTo( const QString &filename, const QString &dirname ) const;

    /**
      Extracts all @acronym TNEF attachments into the default directory.

      @return true if the extraction succeeds; otherwise false.
    */
    bool extractAll();

    /**
      Sets the default extraction directory to @p dirname.

      @param dirname is the name of the default extraction directory.
    */
    void setDefaultExtractDir( const QString &dirname );

    /**
      Returns the KTNEFMessage used in the parsing process.

      @return a pointer to a KTNEFMessage object.
    */
    KTNEFMessage *message() const;

private:
    //@cond PRIVATE
    class ParserPrivate;
    ParserPrivate *const d;
    //@endcond

    Q_DISABLE_COPY( KTNEFParser )
};

}
#endif
