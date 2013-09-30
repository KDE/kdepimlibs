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
#include <QPair>
#include <QPointer>
#include <QVector>

using namespace Akonadi;
using namespace boost;
using namespace KRss;

struct CreateInfo {
    CreateInfo()
        : relativeParentIndex( -1 )
        , error( false ) {
    }

    CreateInfo( const Collection& c, int pp )
        : collection( c )
        , relativeParentIndex( pp )
        , error( false )
    {}

    Collection collection;
    int relativeParentIndex;
    bool error;
    QString errorString;
};

class KRss::ImportFromOpmlJob::Private {
    ImportFromOpmlJob* const q;
public:
    explicit Private( ImportFromOpmlJob* qq ) : q( qq ), currentlyCreatedIndex( -1 ), session( 0 ), createCollections( true ) {}

    void doStart();
    void collectionCreated( KJob* );
    void createNext();

    QString inputFile;
    Collection parentFolder;
    QString opmlTitle;
    QVector<CreateInfo> collections;
    int currentlyCreatedIndex;
    QPointer<Session> session;
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
    Collection::List l;
    l.reserve( d->collections.size() );
    Q_FOREACH( const CreateInfo& ci, d->collections )
        if ( !ci.error )
            l.append( ci.collection );
    return l;
}

QString ImportFromOpmlJob::opmlTitle() const {
    return d->opmlTitle;
}

static QVector<CreateInfo> buildCollectionTree( const QString& opmlPath, const QList<shared_ptr<const ParsedNode> >& listOfNodes, const CreateInfo& parent )
{
    QVector<CreateInfo> list;
    list << parent;

    int relPos = 1;
    Q_FOREACH (const shared_ptr<const ParsedNode>& parsedNode, listOfNodes )
    {
        Collection c = parsedNode->toAkonadiCollection();
        c.setParentCollection( parent.collection );
        if ( !parsedNode->isFolder() ) {
            c.setRights( Collection::CanChangeCollection | Collection::CanDeleteCollection |
                         Collection::CanCreateItem | Collection::CanChangeItem | Collection::CanDeleteItem );
            list << CreateInfo( c, -relPos );
            ++relPos;
        } else {
            c.setRemoteId( opmlPath + parsedNode->title() );
            c.setRights( Collection::CanCreateCollection | Collection::CanChangeCollection | Collection::CanDeleteCollection );
            shared_ptr<const ParsedFolder> parsedFolder = static_pointer_cast<const ParsedFolder>(parsedNode);
            const QVector<CreateInfo> children = buildCollectionTree( opmlPath, parsedFolder->children(), CreateInfo( c, -relPos ) );
            list << children;
            relPos += children.count();
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
        q->setErrorText( i18n("Could not parse OPML from %1: [%2:%3] %4",
                              inputFile,
                              QString::number( reader.lineNumber() ),
                              QString::number( reader.columnNumber() ),
                              reader.errorString() ) );
        q->emitResult();
        return;
    }

    opmlTitle = parser.titleOpml();
    const QList<shared_ptr<const ParsedNode> > parsedNodes = parser.topLevelNodes();
    collections = buildCollectionTree( inputFile, parsedNodes, CreateInfo( parentFolder, 0 ) );

    if ( !createCollections ) {
        q->emitResult();
        return;
    }

    createNext();
}

void ImportFromOpmlJob::Private::collectionCreated( KJob* j ) {
    const CollectionCreateJob* const job = qobject_cast<CollectionCreateJob*>( j );
    Q_ASSERT( job );
    if ( job->error() != KJob::NoError ) {
        collections[currentlyCreatedIndex].error = true;
        collections[currentlyCreatedIndex].errorString = job->errorString();
    } else {
        collections[currentlyCreatedIndex].collection = job->collection();
    }
    createNext();
}

struct Successful {
    bool operator()( const CreateInfo& ci ) const {
        return !ci.error;
    }
};

void ImportFromOpmlJob::Private::createNext() {
    if ( currentlyCreatedIndex == collections.size()-1 ) {
        QVector<CreateInfo> errors = collections;
        errors.erase( std::remove_if( errors.begin(), errors.end(), Successful() ), errors.end() );
        if ( !errors.isEmpty() ) {
            q->setError( KJob::UserDefinedError );
            QStringList lines;
            Q_FOREACH( const CreateInfo& ci, errors )
                lines += i18nc( "feed title: reason why feed could not be imported", "%1: %2", ci.collection.attribute<Akonadi::EntityDisplayAttribute>()->displayName(), ci.errorString );
            q->setErrorText( i18n("The import of the following feeds and folders failed:\n\n%1", lines.join( QLatin1String("\n") ) ) );
        }
        q->emitResult();
        return;
    }

    currentlyCreatedIndex++;
    CreateInfo& ci = collections[currentlyCreatedIndex];
    Q_ASSERT( ci.relativeParentIndex < 0 || currentlyCreatedIndex == 0 );
    if ( ci.relativeParentIndex < 0 ) {
        const int parentIndex = currentlyCreatedIndex + ci.relativeParentIndex;
        ci.collection.setParentCollection( collections[parentIndex].collection );
    }
    CollectionCreateJob* job = new CollectionCreateJob( ci.collection, session );
    job->connect( job, SIGNAL(result(KJob*)), q, SLOT(collectionCreated(KJob*)) );
    job->start();
}

bool ImportFromOpmlJob::createCollections() const {
    return d->createCollections;
}

void ImportFromOpmlJob::setCreateCollections( bool create ) {
    d->createCollections = create;
}

#include "moc_importfromopmljob.cpp"
