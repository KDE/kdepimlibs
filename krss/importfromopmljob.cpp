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

#include "importfromopmljob.h"
#include "opmlparser.h"
#include "feedcollection.h"

#include <Akonadi/EntityDisplayAttribute>

#include <KLocalizedString>
#include <KRandom>

#include <QFile>

using namespace Akonadi;
using namespace boost;
using namespace KRss;

static QString mimeType()
{
    return QLatin1String("application/rss+xml");
}

class KRss::ImportFromOpmlJob::Private {
    ImportFromOpmlJob* const q;
public:
    explicit Private( ImportFromOpmlJob* qq ) : q( qq ) {}

    void doStart();

    QString inputFile;
    Collection parentFolder;
    QString opmlTitle;
    Collection::List collections;
};

ImportFromOpmlJob::ImportFromOpmlJob( QObject* parent )
    : KJob( parent )
    , d( new Private( this ) )
{}


ImportFromOpmlJob::~ImportFromOpmlJob() {
    delete d;
}

void ImportFromOpmlJob::start() {
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

QString ImportFromOpmlJob::inputFile() const {
    return d->inputFile;
}

void ImportFromOpmlJob::setInputFile( const QString& path ) {
    d->inputFile = path;
}

Collection ImportFromOpmlJob::parentFolder() const {
    return d->parentFolder;
}

void ImportFromOpmlJob::setParentFolder( const Collection& parentFolder ) {
    d->parentFolder = parentFolder;
}

Collection::List ImportFromOpmlJob::collections() const {
    return d->collections;
}

QString ImportFromOpmlJob::opmlTitle() const {
    return d->opmlTitle;
}

static Collection::List buildCollectionTree( const QString& opmlPath, const QList<shared_ptr<const ParsedNode> >& listOfNodes, const Collection &parent)
{
    Collection::List list;
    list << parent;

    foreach(const shared_ptr<const ParsedNode> parsedNode, listOfNodes) {
        if (!parsedNode->isFolder()) {
            Collection c = (static_pointer_cast<const ParsedFeed>(parsedNode))->toAkonadiCollection();
            c.attribute<Akonadi::EntityDisplayAttribute>( Collection::AddIfMissing )->setDisplayName( parsedNode->title() );
            c.setParent( parent );

            //it customizes the collection with an rss icon
            c.attribute<Akonadi::EntityDisplayAttribute>( Collection::AddIfMissing )->setIconName( QString("application-rss+xml") );

            list << c;
        } else {
            shared_ptr<const ParsedFolder> parsedFolder = static_pointer_cast<const ParsedFolder>(parsedNode);
            KRss::FeedCollection folder;
            folder.setParent( parent );
            folder.setName( parsedFolder->title() + KRandom::randomString( 8 ) );
            folder.attribute<Akonadi::EntityDisplayAttribute>( Collection::AddIfMissing )->setDisplayName( parsedFolder->title() );
            folder.setRemoteId( opmlPath + parsedFolder->title() );
            folder.setIsFolder( true );
            folder.setContentMimeTypes( QStringList() << Collection::mimeType() << mimeType() );
            list << buildCollectionTree( opmlPath, parsedFolder->children(), folder );
        }
    }

    return list;
}

void ImportFromOpmlJob::Private::doStart() {

    QFile file( inputFile );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        q->setError( KJob::UserDefinedError );
        q->setErrorText( i18n("Could not open %1: %2", inputFile, file.errorString()) );
        q->emitResult();
        return;
    }

    QXmlStreamReader reader( &file );

    OpmlReader parser;

    while ( !reader.atEnd() ) {
        reader.readNext();

        if ( reader.isStartElement() ) {
        //check if the file is formatted opml, before parsing it
        //TODO: move this checking to inside the parser.readOpml implementation
            if ( reader.name().toString().toLower() == QLatin1String("opml") ) {
                parser.readOpml( reader );
            }
            else {
                reader.raiseError( i18n ( "The file is not a valid OPML document." ) );
            }
        }
    }

    if (reader.hasError()) {
        q->setError( KJob::UserDefinedError );
        q->setErrorText( i18n("Could not parse OPML from %1: %2", inputFile, reader.errorString() ) );
        q->emitResult();
        return;
    }

    opmlTitle = parser.titleOpml();
    QList<shared_ptr<const ParsedNode> > parsedNodes = parser.topLevelNodes();
    collections = buildCollectionTree(inputFile, parsedNodes, parentFolder);
    q->emitResult();
}

#include "importfromopmljob.moc"
