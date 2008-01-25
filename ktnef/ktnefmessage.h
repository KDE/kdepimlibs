/*
    ktnefmessage.h

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
 * defines the KTNEFMessage class.
 *
 * @author Michael Goffioul
 */

#ifndef KTNEFMESSAGE_H
#define KTNEFMESSAGE_H

#include <QtCore/QList>

#include "ktnefpropertyset.h"
#include "ktnef_export.h"

namespace KTnef { class KTNEFAttach; }

namespace KTnef {

/**
 * @brief
 * Represents a @acronym TNEF message.
 */
class KTNEF_EXPORT KTNEFMessage : public KTNEFPropertySet
{
  public:
    /**
     * Creates a KTNEFMessage message object.
     */
    KTNEFMessage();

    /**
     * Destroys a KTNEFMessage message object.
     */
    ~KTNEFMessage();

    /**
     * Return a QList containing all the message's attachments.
     */
    const QList<KTNEFAttach *> &attachmentList() const;

    /**
     * Find the attachment associated to the specified file name.
     *
     * @param filename is a QString containing the file to search for in the
     * list of message attachments.
     *
     * @return A pointer to KTNEFAttach object, or 0 if the search fails.
     */
    KTNEFAttach *attachment( const QString &filename ) const;

    /**
     * Append an attachment to the message.
     * @param attach is a pointer to a KTNEFAttach object to be attached.
     */
    void addAttachment( KTNEFAttach *attach );

    /**
     * Clear the attachments list.
     */
    void clearAttachments();

    /**
     * Returns the Rich Text Format (@acronym RTF) data contained in the message.
     * @return A QString containing the @acronym RTF data.
     */
    QString rtfString() const;

  private:
    //@cond PRIVATE
    class MessagePrivate;
    MessagePrivate *const d;
    //@endcond

    Q_DISABLE_COPY( KTNEFMessage )
};

}
#endif
