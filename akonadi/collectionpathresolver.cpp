/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "collectionpathresolver_p.h"

#include "collectionfetchjob.h"
#include "job_p.h"

#include <klocale.h>

#include <QtCore/QStringList>

using namespace Akonadi;

//@cond PRIVATE

class Akonadi::CollectionPathResolverPrivate : public JobPrivate
{
  public:
    CollectionPathResolverPrivate( CollectionPathResolver *parent )
      : JobPrivate( parent )
    {
    }

    void jobResult( KJob* );

    QStringList splitPath( const QString &path )
    {
      if ( path.isEmpty() ) // path is normalized, so non-empty means at least one hit
        return QStringList();

      QStringList rv;
      int begin = 0;
      for ( int i = 0; i < path.size(); ++i ) {
        if ( path[i] == QLatin1Char('/') ) {
          QString pathElement = path.mid( begin, i - begin );
          pathElement = pathElement.replace( QLatin1String( "\\/" ), QLatin1String( "/" ) );
          rv.append( pathElement );
          begin = i + 1;
        }
        if ( i < path.size() - 2 && path[i] == QLatin1Char('\\') && path[i + 1] == QLatin1Char('/') )
          ++i;
      }
      QString pathElement = path.mid( begin );
      pathElement = pathElement.replace( QLatin1String( "\\/" ), QLatin1String( "/" ) );
      rv.append( pathElement );
      return rv;
    }

    Q_DECLARE_PUBLIC( CollectionPathResolver )

    Collection::Id mColId;
    QString mPath;
    bool mPathToId;
    QStringList mPathParts;
    Collection mCurrentNode;
};

void CollectionPathResolverPrivate::jobResult(KJob *job )
{
  if ( job->error() )
    return;

  Q_Q( CollectionPathResolver );

  CollectionFetchJob *list = static_cast<CollectionFetchJob*>( job );
  CollectionFetchJob *nextJob = 0;
  const Collection::List cols = list->collections();
  if ( cols.isEmpty() ) {
      q->setError( CollectionPathResolver::Unknown );
      q->setErrorText( i18n( "No such collection." ) );
      q->emitResult();
      return;
  }

  if ( mPathToId ) {
    const QString currentPart = mPathParts.takeFirst();
    bool found = false;
    foreach ( const Collection &c, cols ) {
      if ( c.name() == currentPart ) {
        mCurrentNode = c;
        found = true;
        break;
      }
    }
    if ( !found ) {
      q->setError( CollectionPathResolver::Unknown );
      q->setErrorText( i18n( "No such collection." ) );
      q->emitResult();
      return;
    }
    if ( mPathParts.isEmpty() ) {
      mColId = mCurrentNode.id();
      q->emitResult();
      return;
    }
    nextJob = new CollectionFetchJob( mCurrentNode, CollectionFetchJob::FirstLevel, q );
  } else {
    Collection col = list->collections().first();
    mCurrentNode = col.parentCollection();
    mPathParts.prepend( col.name() );
    if ( mCurrentNode == Collection::root() ) {
      q->emitResult();
      return;
    }
    nextJob = new CollectionFetchJob( mCurrentNode, CollectionFetchJob::Base, q );
  }
  q->connect( nextJob, SIGNAL(result(KJob*)), q, SLOT(jobResult(KJob*)) );
}

CollectionPathResolver::CollectionPathResolver(const QString & path, QObject * parent)
  : Job( new CollectionPathResolverPrivate( this ), parent )
{
  Q_D( CollectionPathResolver );

  d->mPathToId = true;
  d->mPath = path;
  if ( d->mPath.startsWith( pathDelimiter() )  )
    d->mPath = d->mPath.right( d->mPath.length() - pathDelimiter().length() );
  if ( d->mPath.endsWith( pathDelimiter() )  )
    d->mPath = d->mPath.left( d->mPath.length() - pathDelimiter().length() );

  d->mPathParts = d->splitPath( d->mPath );
  d->mCurrentNode = Collection::root();
}

CollectionPathResolver::CollectionPathResolver(const Collection & collection, QObject * parent)
  : Job( new CollectionPathResolverPrivate( this ), parent )
{
  Q_D( CollectionPathResolver );

  d->mPathToId = false;
  d->mColId = collection.id();
  d->mCurrentNode = collection;
}

CollectionPathResolver::~CollectionPathResolver()
{
}

Collection::Id CollectionPathResolver::collection() const
{
  Q_D( const CollectionPathResolver );

  return d->mColId;
}

QString CollectionPathResolver::path() const
{
  Q_D( const CollectionPathResolver );

  if ( d->mPathToId )
    return d->mPath;
  return d->mPathParts.join( pathDelimiter() );
}

QString CollectionPathResolver::pathDelimiter()
{
  return QLatin1String( "/" );
}

void CollectionPathResolver::doStart()
{
  Q_D( CollectionPathResolver );

  CollectionFetchJob *job = 0;
  if ( d->mPathToId ) {
    if ( d->mPath.isEmpty() ) {
      d->mColId = Collection::root().id();
      emitResult();
      return;
    }
    job = new CollectionFetchJob( d->mCurrentNode, CollectionFetchJob::FirstLevel, this );
  } else {
    if ( d->mColId == 0 ) {
      d->mColId = Collection::root().id();
      emitResult();
      return;
    }
    job = new CollectionFetchJob( d->mCurrentNode, CollectionFetchJob::Base, this );
  }
  connect( job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)) );
}

//@endcond

#include "collectionpathresolver_p.moc"
