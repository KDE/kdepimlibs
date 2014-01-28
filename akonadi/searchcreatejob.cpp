
/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2014 Daniel Vrátil <dvratil@redhat.com>

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

#include "searchcreatejob.h"

#include "collection.h"
#include "imapparser_p.h"
#include "protocolhelper_p.h"
#include "job_p.h"
#include "searchquery.h"
#include <akonadi/private/protocol_p.h>

using namespace Akonadi;

class Akonadi::SearchCreateJobPrivate : public JobPrivate
{
  public:
    SearchCreateJobPrivate( const QString &name, const SearchQuery &query, SearchCreateJob *parent )
      : JobPrivate( parent )
      , mName( name )
      , mQuery( query )
    {
    }

    QString mName;
    SearchQuery mQuery;
    Collection mCreatedCollection;
};

SearchCreateJob::SearchCreateJob( const QString &name, const QString &query, QObject *parent )
  : Job( new SearchCreateJobPrivate( name, SearchQuery::fromJSON( query.toLatin1() ), this ), parent )
{
}

SearchCreateJob::SearchCreateJob( const QString &name, const SearchQuery &searchQuery, QObject *parent)
  : Job( new SearchCreateJobPrivate( name, searchQuery, this ), parent )
{
}


SearchCreateJob::~SearchCreateJob()
{
}

void SearchCreateJob::setQueryLanguage( const QString &queryLanguage )
{
  Q_UNUSED( queryLanguage );
}

void SearchCreateJob::doStart()
{
  Q_D( SearchCreateJob );

  QByteArray command = d->newTag() + " SEARCH_STORE ";
  command += ImapParser::quote( d->mName.toUtf8() );
  command += ' ';
  command += ImapParser::quote( d->mQuery.toJSON() );
  command += '\n';
  d->writeData( command );
}

Akonadi::Collection SearchCreateJob::createdCollection() const
{
  Q_D( const SearchCreateJob );
  return d->mCreatedCollection;
}

void SearchCreateJob::doHandleResponse( const QByteArray &tag, const QByteArray &data )
{
  Q_D( SearchCreateJob );
  if ( tag == "*" ) {
   ProtocolHelper::parseCollection( data, d->mCreatedCollection );
   return;
  }
  kDebug() << "Unhandled response: " << tag << data;
}

