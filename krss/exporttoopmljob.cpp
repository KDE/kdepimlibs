#include "exporttoopmljob.h"
#include "feedcollection.h"
#include "opmlparser.h"

#include <KLocalizedString>
#include <KSaveFile>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>

#include <boost/shared_ptr.hpp>

using namespace boost;
using namespace Akonadi;
using namespace KRss;

static QList< shared_ptr< const ParsedNode > > parsedDescendants( const Collection::List& collections_, const Collection& parent )
{
    Akonadi::Collection::List collections = collections_;
    QList<shared_ptr< const ParsedNode > > nodesList;

    Q_FOREACH( const Akonadi::Collection& collection , collections ) {
        if (collection.parentCollection() == parent) {
            shared_ptr< ParsedNode > node;
            const FeedCollection feedCollection = collection;
            if (feedCollection.isFolder()) {
                QList<shared_ptr< const ParsedNode > > children = parsedDescendants( collections, collection );
                if (parent == Collection::root())
                    return children;
                shared_ptr<ParsedFolder> parsedFolder( new ParsedFolder );
                parsedFolder->setTitle( feedCollection.name() );
                parsedFolder->setChildren( children );
                node = parsedFolder;
            } else {
                node = ParsedFeed::fromAkonadiCollection ( collection );
            }

            collections.removeOne( collection );
            nodesList.append( node );
        }
    }

    return nodesList;

}

static bool writeFeedsToOpml(const QString &path,
                             const QList<shared_ptr< const ParsedNode> >& nodes,
                             const QString& titleOpml,
                             bool withCustomProperties,
                             QString* errorString)
{
    Q_UNUSED(withCustomProperties)
    Q_ASSERT(errorString);
    KSaveFile file( path );
    if ( !file.open( QIODevice::WriteOnly ) ) {
        *errorString = i18n("Could not open %1: %2", path, file.errorString());
        return false;
    }

    QXmlStreamWriter writer( &file );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    OpmlWriter::writeOpml( writer, nodes, titleOpml );
    writer.writeEndDocument();

    if ( writer.hasError() || !file.finalize() ) { //hasError() refers to the underlying device, so file.errorString() is our best bet in both cases
        *errorString = i18n("Could not save %1: %2", path, file.errorString() );
        return false;
    }

    return true;
}

class KRss::ExportToOpmlJob::Private {
    ExportToOpmlJob* const q;
public:
    explicit Private( ExportToOpmlJob* qq ) : q( qq ), includeCustomProperties( false ) {}

    void doStart();
    void fetchFinished( KJob* job );

    QString resource;
    QString outputFile;
    bool includeCustomProperties;
};

ExportToOpmlJob::ExportToOpmlJob( QObject* parent )
    : KJob( parent )
    , d( new Private( this ) )
{}

ExportToOpmlJob::~ExportToOpmlJob() {
    delete d;
}

void ExportToOpmlJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

QString ExportToOpmlJob::resource() const {
    return d->resource;
}

void ExportToOpmlJob::setResource( const QString& identifier ) {
    d->resource = identifier;
}


QString ExportToOpmlJob::outputFile() const {
    return d->outputFile;
}

void ExportToOpmlJob::setOutputFile( const QString& path ) {
    d->outputFile = path;
}

bool ExportToOpmlJob::includeCustomProperties() const {
    return d->includeCustomProperties;
}

void ExportToOpmlJob::setIncludeCustomProperties( bool includeCustomProperties ) {
    d->includeCustomProperties = includeCustomProperties;
}

void ExportToOpmlJob::Private::doStart() {
    CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, q );
    job->setResource( resource );
    job->fetchScope().setContentMimeTypes( QStringList() << QLatin1String("application/rss+xml") );
    connect( job, SIGNAL( result( KJob* ) ), q, SLOT( fetchFinished( KJob* ) ) );
}

void ExportToOpmlJob::Private::fetchFinished( KJob* j ) {
    CollectionFetchJob* job = qobject_cast<CollectionFetchJob*>( j );
    Q_ASSERT(job);
    if ( job->error() ) {
        q->setErrorText( job->errorText() );
        q->setError( KJob::UserDefinedError );
        q->emitResult();
        return;
    }

    const Collection::List collections = job->collections();
    QString errorString;
    const bool written = writeFeedsToOpml( outputFile,
                                           parsedDescendants( collections, Collection::root() ),
                                           QString(),
                                           includeCustomProperties,
                                           &errorString );
    if ( !written ) {
        q->setErrorText( errorString );
        q->setError( KJob::UserDefinedError );
    }

    q->emitResult();
}

#include "exporttoopmljob.moc"
