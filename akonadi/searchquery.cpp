/*
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

#include "searchquery.h"

#include <QtCore/QVariant>

#include <KDebug>

#include <qjson/parser.h>
#include <qjson/serializer.h>

using namespace Akonadi;

class SearchTerm::Private: public QSharedData
{
  public:
    Private():
      QSharedData(),
      condition( SearchTerm::CondEqual ),
      relation( SearchTerm::RelAnd ),
      isNegated( false )
    {
    }

    Private( const Private &other ):
      QSharedData( other ),
      key( other.key ),
      value( other.value ),
      condition( other.condition ),
      relation( other.relation ),
      terms( other.terms ),
      isNegated( other.isNegated )
    {
    }

    bool operator==( const Private &other )
    {
      return relation == other.relation
             && isNegated == other.isNegated
             && terms == other.terms
             && key == other.key
             && value == other.value
             && condition == other.condition;
    }

    QString key;
    QVariant value;
    Condition condition;
    Relation relation;
    QList<SearchTerm> terms;
    bool isNegated;
};

class SearchQuery::Private: public QSharedData
{
  public:
    Private():
      QSharedData()
    {
    }

    Private( const Private &other ):
      QSharedData( other ),
      rootTerm( other.rootTerm )
    {
    }

    bool operator==( const Private &other )
    {
      return rootTerm == other.rootTerm;
    }

    static QVariantMap termToJSON( const SearchTerm &term )
    {
      const QList<SearchTerm> &subTerms = term.subTerms();
      QVariantMap termJSON;
      termJSON.insert( QLatin1String( "negated"), term.isNegated() );
      if ( subTerms.isEmpty() ) {
        termJSON.insert( QLatin1String( "key" ), term.key() );
        termJSON.insert( QLatin1String( "value" ), term.value() );
        termJSON.insert( QLatin1String( "cond" ), static_cast<int>( term.condition() )  );
      } else {
        termJSON.insert( QLatin1String( "rel" ), static_cast<int>( term.relation() ) );
        QVariantList subTermsJSON;
        Q_FOREACH ( const SearchTerm &term, subTerms ) {
          subTermsJSON.append( termToJSON( term ) );
        }
        termJSON.insert( QLatin1String( "subTerms" ), subTermsJSON );
      }

      return termJSON;
    }

    static SearchTerm JSONToTerm( const QVariantMap &json )
    {
      if ( json.contains( QLatin1String( "key" ) ) ) {
        SearchTerm term( json[QLatin1String( "key" )].toString(),
                           json[QLatin1String( "value" )],
                           static_cast<SearchTerm::Condition>( json[QLatin1String( "cond" )].toInt() ) );
        term.setIsNegated( json[QLatin1String( "negated" )].toBool() );
        return term;
      } else if ( json.contains( QLatin1String( "rel" ) ) ) {
        SearchTerm term( static_cast<SearchTerm::Relation>( json[QLatin1String( "rel" )].toInt() ) );
        term.setIsNegated( json[QLatin1String( "negated" )].toBool() );
        const QVariantList subTermsJSON = json[QLatin1String( "subTerms" )].toList();
        Q_FOREACH ( const QVariant &subTermJSON, subTermsJSON ) {
          term.addSubTerm( JSONToTerm( subTermJSON.toMap() ) );
        }
        return term;
      } else {
        kWarning() << "Invalid JSON for term: "<< json;
        return SearchTerm();
      }
    }

    SearchTerm rootTerm;
};

SearchTerm::SearchTerm( SearchTerm::Relation relation ):
  d( new Private )
{
  d->relation = relation;
}

SearchTerm::SearchTerm( const QString &key, const QVariant &value, SearchTerm::Condition condition ):
  d( new Private )
{
  d->relation = RelAnd;
  d->key = key;
  d->value = value;
  d->condition = condition;
}

SearchTerm::SearchTerm( const SearchTerm &other ):
  d( other.d )
{
}

SearchTerm::~SearchTerm()
{
}

SearchTerm& SearchTerm::operator=( const SearchTerm &other )
{
  d = other.d;
  return *this;
}

bool SearchTerm::operator==( const SearchTerm &other )
{
  return d == other.d;
}

bool SearchTerm::isNull() const
{
  return d->key.isEmpty() && d->value.isNull() && d->terms.isEmpty();
}

QString SearchTerm::key() const
{
  return d->key;
}

QVariant SearchTerm::value() const
{
  return d->value;
}

SearchTerm::Condition SearchTerm::condition() const
{
  return d->condition;
}

void SearchTerm::setIsNegated( bool negated )
{
  d->isNegated = negated;
}

bool SearchTerm::isNegated() const
{
  return d->isNegated;
}

void SearchTerm::addSubTerm( const SearchTerm &term )
{
  d->terms << term;
}

QList< SearchTerm > SearchTerm::subTerms() const
{
  return d->terms;
}

SearchTerm::Relation SearchTerm::relation() const
{
  return d->relation;
}


SearchQuery::SearchQuery( SearchTerm::Relation rel ):
  d( new Private )
{
  d->rootTerm = SearchTerm( rel );
}

SearchQuery::SearchQuery( const SearchQuery &other ):
  d( other.d )
{
}

SearchQuery::~SearchQuery()
{
}

SearchQuery& SearchQuery::operator=( const SearchQuery &other )
{
  d = other.d;
  return *this;
}

bool SearchQuery::operator==( const SearchQuery &other )
{
  return d->rootTerm == other.d->rootTerm;
}

bool SearchQuery::isNull() const
{
  return d->rootTerm.isNull();
}

SearchTerm SearchQuery::term() const
{
  return d->rootTerm;
}

void SearchQuery::addTerm( const QString &key, const QVariant &value, SearchTerm::Condition condition )
{
  addTerm( SearchTerm( key, value, condition ) );
}

void SearchQuery::addTerm( const SearchTerm &term )
{
  d->rootTerm.addSubTerm( term );
}

void SearchQuery::setTerm( const SearchTerm& term )
{
  d->rootTerm = term;
}

QByteArray SearchQuery::toJSON() const
{
  QVariantMap root = Private::termToJSON( d->rootTerm );

  QJson::Serializer serializer;
  return serializer.serialize( root );
}

SearchQuery SearchQuery::fromJSON( const QByteArray &jsonData )
{
  QJson::Parser parser;
  bool ok = false;
  const QVariant json = parser.parse( jsonData, &ok );
  if ( !ok || json.isNull() ) {
    return SearchQuery();
  }

  SearchQuery query;
  query.d->rootTerm = Private::JSONToTerm( json.toMap() );
  return query;
}
