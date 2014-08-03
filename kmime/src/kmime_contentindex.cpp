/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the ContentIndex class.

  @brief
  Defines the ContentIndex class.

  @authors Volker Krause \<vkrause@kde.org\>
*/

#include "kmime_contentindex.h"

#include <QHash>
#include <QSharedData>
#include <QtCore/QStringList>

using namespace KMime;

class ContentIndex::Private : public QSharedData
{
public:
    Private() {}
    Private(const Private &other) : QSharedData(other)
    {
        index = other.index;
    }

    QList<unsigned int> index;
};

KMime::ContentIndex::ContentIndex() : d(new Private)
{
}

KMime::ContentIndex::ContentIndex(const QString &index) : d(new Private)
{
    const QStringList l = index.split(QLatin1Char('.'));
    foreach (const QString &s, l) {
        bool ok;
        unsigned int i = s.toUInt(&ok);
        if (!ok) {
            d->index.clear();
            break;
        }
        d->index.append(i);
    }
}

ContentIndex::ContentIndex(const ContentIndex &other) : d(other.d)
{
}

ContentIndex::~ContentIndex()
{
}

bool KMime::ContentIndex::isValid() const
{
    return !d->index.isEmpty();
}

unsigned int KMime::ContentIndex::pop()
{
    return d->index.takeFirst();
}

void KMime::ContentIndex::push(unsigned int index)
{
    d->index.prepend(index);
}

QString KMime::ContentIndex::toString() const
{
    QStringList l;
    foreach (unsigned int i, d->index) {
        l.append(QString::number(i));
    }
    return l.join(QLatin1String("."));
}

bool KMime::ContentIndex::operator ==(const ContentIndex &index) const
{
    return d->index == index.d->index;
}

bool KMime::ContentIndex::operator !=(const ContentIndex &index) const
{
    return d->index != index.d->index;
}

ContentIndex &ContentIndex::operator =(const ContentIndex &other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

uint qHash(const KMime::ContentIndex &index)
{
    return qHash(index.toString());
}
