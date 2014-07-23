/*
    ktnefmessage.cpp

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

#include "ktnefmessage.h"
#include "ktnefattach.h"
#include "lzfu.h"

#include <QtCore/QBuffer>
#include <QtCore/QList>

using namespace KTnef;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
//@cond PRIVATE
class KTnef::KTNEFMessage::MessagePrivate
{
public:
    MessagePrivate() {}
    ~MessagePrivate();

    void clearAttachments();

    QList<KTNEFAttach *>attachments_;
};

KTNEFMessage::MessagePrivate::~MessagePrivate()
{
    clearAttachments();
}

void KTNEFMessage::MessagePrivate::clearAttachments()
{
    while (!attachments_.isEmpty()) {
        delete attachments_.takeFirst();
    }
}
//@endcond

KTNEFMessage::KTNEFMessage() : d(new KTnef::KTNEFMessage::MessagePrivate)
{
}

KTNEFMessage::~KTNEFMessage()
{
    delete d;
}

const QList<KTNEFAttach *> &KTNEFMessage::attachmentList() const
{
    return d->attachments_;
}

KTNEFAttach *KTNEFMessage::attachment(const QString &filename) const
{
    QList<KTNEFAttach *>::const_iterator it = d->attachments_.constBegin();
    QList<KTNEFAttach *>::const_iterator end = d->attachments_.constEnd();
    for (; it != end; ++it) {
        if ((*it)->name() == filename) {
            return *it;
        }
    }
    return 0;
}

void KTNEFMessage::addAttachment(KTNEFAttach *attach)
{
    d->attachments_.append(attach);
}

void KTNEFMessage::clearAttachments()
{
    d->clearAttachments();
}

QString KTNEFMessage::rtfString() const
{
    QVariant prop = property(0x1009);
    if (prop.isNull() || prop.type() != QVariant::ByteArray) {
        return QString();
    } else {
        QByteArray rtf;
        QByteArray propArray(prop.toByteArray());
        QBuffer input(&propArray), output(&rtf);
        if (input.open(QIODevice::ReadOnly) &&
                output.open(QIODevice::WriteOnly)) {
            KTnef::lzfu_decompress(&input, &output);
        }
        return QString::fromLatin1(rtf);
    }
}
