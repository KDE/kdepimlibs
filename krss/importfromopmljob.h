/*
 * This file is part of the krss library
 *
 * Copyright (C) 2012 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KRSS_IMPORTFROMOPMLJOB_H
#define KRSS_IMPORTFROMOPMLJOB_H

#include "krss_export.h"

#include "akonadi/collection.h"
#include <KJob>

namespace KRss {
  
class KRSS_EXPORT ImportFromOpmlJob : public KJob {
    Q_OBJECT
public:
    explicit ImportFromOpmlJob( QObject* parent=0 );
    ~ImportFromOpmlJob();

    void start();

    QString inputFile() const;
    void setInputFile( const QString& path );

    QString opmlTitle() const;

    Akonadi::Collection::List collections() const;

    Akonadi::Collection parentFolder() const;
    void setParentFolder( const Akonadi::Collection& parentFolder );

private:
    class Private;
    Private* const d;
    Q_PRIVATE_SLOT(d, void doStart())
};

}

#endif
