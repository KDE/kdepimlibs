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

#include <akonadi/collectioncreatejob.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/session.h>

#include <KLocalizedString>
#include <KRandom>

#include <QFile>
#include <QPointer>

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
    explicit Private( ImportFromOpmlJob* qq ) : q( qq ), session( 0 ), createCollections( true ) {}

    void doStart();
    void collectionsCreated( KJob* );
    void createNext();

    QString inputFile;
    Collection parentFolder;
    QString opmlTitle;
    Collection::List collections;
    Collection::List creationQueue;
    Collection currentlyCreated;
    QPointer<Session> session;
    QMap<Collection, QString> errors;
    bool createCollections;
};

ImportFromOpmlJob::ImportFromOpmlJob( QObject* parent )
    : KJob( parent )
    , d( new Private( this ) )
{}


ImportFromOpmlJob::~ImportFromOpmlJob() {
    delete d;
}

Session* ImportFromOpmlJob::session() const {
    return d->session;
}

void ImportFromOpmlJob::setSession( Akonadi::Session* session ) {
    d->session = session;
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
            c.setParentCollection( parent );

            //it customizes the collection with an rss icon
            c.attribute<Akonadi::EntityDisplayAttribute>( Collection::AddIfMissing )->setIconName( QString("application-rss+xml") );
            c.setRights( Collection::CanChangeCollection | Collection::CanDeleteCollection |
                         Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem );

            list << c;
        } else {
            shared_ptr<const ParsedFolder> parsedFolder = static_pointer_cast<const ParsedFolder>(parsedNode);
            KRss::FeedCollection folder;
            folder.setParentCollection( parent );
            folder.setName( parsedFolder->title() + KRandom::randomString( 8 ) );
            folder.attribute<Akonadi::EntityDisplayAttribute>( Collection::AddIfMissing )->setDisplayName( parsedFolder->title() );
            folder.setRemoteId( opmlPath + parsedFolder->title() );
            folder.setIsFolder( true );
            folder.setContentMimeTypes( QStringList() << Collection::mimeType() << mimeType() );
            folder.setRights( Collection::CanCreateCollection | Collection::CanChangeCollection | Collection::CanDeleteCollection );
            list << buildCollectionTree( opmlPath, parsedFolder->children(), folder );
        }
    }

    return list;
}

void ImportFromOpmlJob::Private::doStart() {

    QFile file( inputFile );
    if ( !file.open( QIODevice::ReadOnly ) ) {
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
    const QList<shared_ptr<const ParsedNode> > parsedNodes = parser.topLevelNodes();
    collections = buildCollectionTree( inputFile, parsedNodes, parentFolder );

    if ( !createCollections ) {
        q->emitResult();
        return;
    }

    std::swap( creationQueue, collections ); //put all collection in queue, clear collections
    Q_ASSERT( collections.isEmpty() );

    createNext();
}

void ImportFromOpmlJob::Private::collectionsCreated( KJob* j ) {
    const CollectionCreateJob* const job = qobject_cast<CollectionCreateJob*>( j );
    Q_ASSERT( job );
    if ( job->error() != KJob::NoError )
        errors.insert( currentlyCreated, job->errorString() );
    else
        collections.append( job->collection() );
    createNext();
}

void ImportFromOpmlJob::Private::createNext() {
    if ( creationQueue.isEmpty() ) {
        if ( !errors.isEmpty() ) {
            q->setError( KJob::UserDefinedError );
            QStringList lines;
            QMap<Collection,QString>::ConstIterator it = errors.constBegin();
            for ( ; it != errors.constEnd(); ++it )
                lines += i18nc( "feed title: reason why feed could not be imported", "%1: %2", it.key().attribute<Akonadi::EntityDisplayAttribute>()->displayName(), it.value() );
            q->setErrorText( i18n("The import of the following feeds and folders failed: %1", lines.join( QLatin1String("\n") ) ) );
        }
        q->emitResult();
        return;
    }

    currentlyCreated = creationQueue.takeFirst();
    CollectionCreateJob* job = new CollectionCreateJob( currentlyCreated, session );
    job->connect( job, SIGNAL(result(KJob*)), q, SLOT(collectionsCreated(KJob*)) );
    job->start();
}

bool ImportFromOpmlJob::createCollections() const {
    return d->createCollections;
}

void ImportFromOpmlJob::setCreateCollections( bool create ) {
    d->createCollections = create;
}

#include "importfromopmljob.moc"
