/*
 * This file is part of the krss library
 *
 * Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
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

#include "item.h"
#include "category.h"
#include "enclosure.h"
#include "person.h"

#include <syndication/tools.h>

#include <KDateTime>
#include <KLocalizedString>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QTextDocument>

#include <algorithm>

using namespace KRss;

const char* Item::HeadersPart = "RssHeaders";
const char* Item::ContentPart = "RssContent";

// static
QString Item::mimeType()
{
    return QLatin1String("application/rss+xml");
}

KRss::ItemStatus itemStatus( const Akonadi::Item& aitem )
{
    //PENDING(frank) this looks like a candidate for caching
    ItemStatus stat;
    if ( !aitem.hasFlag( Item::flagRead() ) )
        stat |= KRss::Unread;

    if ( aitem.hasFlag( Item::flagImportant() ) )
        stat |= KRss::Important;

    if ( aitem.hasFlag( Item::flagDeleted() ) )
        stat |= KRss::Deleted;

    if ( aitem.hasFlag( Item::flagUpdated() ) )
        stat |= KRss::Updated;

    return stat;
}

void KRss::setItemStatus( Akonadi::Item& aitem, const ItemStatus& stat )
{
    Akonadi::Item::Flags flags;
    if ( !stat.testFlag( KRss::Unread ) )
        flags.insert( Item::flagRead() );

    if ( stat.testFlag( KRss::Important ) )
        flags.insert( Item::flagImportant() );

    if ( stat.testFlag( KRss::Deleted ) )
        flags.insert( Item::flagDeleted() );

    if ( stat.testFlag( KRss::Updated ) )
        flags.insert( Item::flagUpdated() );

    aitem.setFlags( flags );
}

class Item::Private : public QSharedData
{
public:
    Private() : hash( 0 ), guidIsHash( false ), commentsCount( -1 ), feedId( -1 ),
        headersLoaded( false ), contentLoaded( false )
    {}

    Private( const Private& other );

    bool operator!=( const Private& other ) const
    {
        return !( *this == other );
    }

    bool operator==( const Private& other ) const
    {
        return hash == other.hash
            && guidIsHash == other.guidIsHash
            && guid == other.guid && title == other.title
            && link == other.link
            && description == other.description
            && content == other.content
            && datePublished == other.datePublished
            && dateUpdated == other.dateUpdated
            && authors == other.authors
            && enclosures == other.enclosures
            && categories == other.categories
            && language == other.language
            && commentsCount == other.commentsCount
            && commentsLink == other.commentsLink
            && commentsFeed == other.commentsFeed
            && commentPostUri == other.commentPostUri
            && customProperties == other.customProperties
            && feedId == other.feedId
            && headersLoaded == other.headersLoaded
            && contentLoaded == other.contentLoaded;
    }

    int hash;
    bool guidIsHash;
    QString guid;
    QString title;
    QString link;
    QString description;
    QString content;
    KDateTime datePublished;
    KDateTime dateUpdated;
    QList<Person> authors;
    QList<Enclosure> enclosures;
    QList<Category> categories;
    QString language;
    int commentsCount;
    QString commentsLink;
    QString commentsFeed;
    QString commentPostUri;
    QHash<QString, QString> customProperties;
    int feedId;
    bool headersLoaded;
    bool contentLoaded;
    mutable QString titleAsPlainText;
};

Item::Private::Private( const Private& other )
    : QSharedData( other ),
    hash( other.hash ),
    guidIsHash( other.guidIsHash ),
    guid( other.guid ),
    title( other.title ),
    link( other.link ),
    description( other.description ),
    content( other.content ),
    datePublished( other.datePublished ),
    dateUpdated( other.dateUpdated ),
    authors( other.authors ),
    enclosures( other.enclosures ),
    categories( other.categories ),
    language( other.language ),
    commentsCount( other.commentsCount ),
    commentsLink( other.commentsLink ),
    commentPostUri( other.commentPostUri ),
    customProperties( other.customProperties ),
    feedId( other.feedId ),
    headersLoaded( other.headersLoaded ),
    contentLoaded( other.contentLoaded )
{
}

QByteArray Item::flagRead()
{
    return "\\SEEN";
}

QByteArray Item::flagImportant()
{
    return "\\Important";
}

QByteArray Item::flagDeleted()
{
    return "\\Deleted";
}

QByteArray Item::flagUpdated()
{
    return "\\Updated";
}

bool Item::isImportant( const Akonadi::Item& item )
{
    return item.hasFlag( flagImportant() );
}

bool Item::isRead( const Akonadi::Item& item )
{
    return item.hasFlag( flagRead() );
}

bool Item::isUnread( const Akonadi::Item& item )
{
    return !item.hasFlag( flagRead() );
}

bool Item::isDeleted( const Akonadi::Item& item )
{
    return item.hasFlag( flagDeleted() );
}

bool Item::isUpdated( const Akonadi::Item& item )
{
    return item.hasFlag( flagUpdated() );
}

ItemId Item::itemIdFromAkonadi( const Akonadi::Item::Id& id )
{
    return id;
}

Akonadi::Item::Id Item::itemIdToAkonadi( const ItemId& itemId )
{
    return itemId;
}

Item::Item() : d( new Private )
{
}

Item::~Item()
{
}

void Item::swap( Item& other )
{
    std::swap( d, other.d );
}

Item& Item::operator=( const Item& other )
{
    Item copy( other );
    swap( copy );
    return *this;
}

bool Item::operator==( const Item& other ) const
{
    return *d == *(other.d);
}

bool Item::operator!=( const Item& other ) const
{
    return *d != *(other.d);
}

Item::Item( const Item& other ) : d( other.d )
{
}

bool Item::headersLoaded() const
{
    return d->headersLoaded;
}

void Item::setHeadersLoaded( bool headersLoaded )
{
    d->headersLoaded = headersLoaded;
}

bool Item::contentLoaded() const
{
    return d->contentLoaded;
}

void Item::setContentLoaded( bool contentLoaded )
{
    d->contentLoaded = contentLoaded;
}

QString Item::title() const
{
    return d->title;
}

QString Item::titleAsPlainText() const {
    if ( d->titleAsPlainText.isNull() )
        d->titleAsPlainText = Syndication::htmlToPlainText( title() );
    return d->titleAsPlainText;
}

void Item::setTitle( const QString& title )
{
    d->title = title;
}

QString Item::description() const
{
    return d->description;
}

void Item::setDescription( const QString& description )
{
    d->description = description;
}

QString Item::link() const
{
    return d->link;
}

void Item::setLink( const QString& link )
{
    d->link = link;
}

QString Item::content() const
{
    return d->content;
}

QString Item::contentWithDescriptionAsFallback() const
{
    if ( d->content.isEmpty() && d->contentLoaded )
        return d->description;
    else
        return d->content;
}

void Item::setContent( const QString& content )
{
    d->content = content;
}

KDateTime Item::datePublished() const
{
    return d->datePublished;
}

void Item::setDatePublished( const KDateTime& datePublished )
{
    d->datePublished = datePublished;
}

KDateTime Item::dateUpdated() const
{
    return d->dateUpdated.isValid() ? d->dateUpdated : d->datePublished;
}

void Item::setDateUpdated( const KDateTime& dateUpdated )
{
    d->dateUpdated = dateUpdated;
}

QString Item::guid() const
{
    return d->guid;
}

void Item::setGuid( const QString& guid )
{
    d->guid = guid;
}

QList<Person> Item::authors() const
{
    return d->authors;
}

void Item::setAuthors( const QList<Person>& authors )
{
    d->authors = authors;
}

static QString authorAsHtml( const Person& p ) {
    const QString name = Qt::escape( p.name() );
    const QString email = Qt::escape( p.email() );

    if (!email.isEmpty()) {
        if (!name.isEmpty())
            return QString::fromLatin1("<a href=\"mailto:%1\">%2</a>").arg( email, name );
        else
            return QString::fromLatin1("<a href=\"mailto:%1\">%1</a>").arg( email );
    }

    const QString uri = Qt::escape( p.uri() );
    if (!name.isEmpty()) {
        if (!uri.isEmpty())
            return QString::fromLatin1("<a href=\"%1\">%2</a>").arg( uri, name );
        else
            return Qt::escape( name );
    }

    if ( !uri.isEmpty() )
        return QString::fromLatin1( "<a href=\"%1\">%1</a>" ).arg( uri );
    return QString();
}

QString Item::authorsAsHtml() const
{
    QStringList formatted;
    Q_FOREACH( const Person& i, d->authors )
        formatted += authorAsHtml( i );
    return formatted.join( i18nc("separator for listing multiple authors", ", ") );
}


QList<Category> Item::categories() const
{
    return d->categories;
}

void Item::setCategories( const QList<Category>& categories )
{
    d->categories = categories;
}

QList<Enclosure> Item::enclosures() const
{
    return d->enclosures;
}

void Item::setEnclosures( const QList<Enclosure>& enclosures )
{
    d->enclosures = enclosures;
}

QString Item::language() const
{
    return d->language;
}

void Item::setLanguage( const QString& language )
{
    d->language = language;
}

int Item::commentsCount() const
{
    return d->commentsCount;
}

void Item::setCommentsCount( int commentsCount )
{
    d->commentsCount = commentsCount;
}

QString Item::commentsLink() const
{
    return d->commentsLink;
}

void Item::setCommentsLink( const QString& commentsLink )
{
    d->commentsLink = commentsLink;
}

QString Item::commentsFeed() const
{
    return d->commentsFeed;
}

void Item::setCommentsFeed( const QString& commentsFeed )
{
    d->commentsFeed = commentsFeed;
}

QString Item::commentPostUri() const
{
    return d->commentPostUri;
}

void Item::setCommentPostUri( const QString& commentPostUri )
{
    d->commentPostUri = commentPostUri;
}

bool Item::guidIsHash() const
{
    return d->guidIsHash;
}

void Item::setGuidIsHash( bool guidIsHash )
{
    d->guidIsHash = guidIsHash;
}

int Item::hash() const
{
    return d->hash;
}

void Item::setHash( int hash )
{
    d->hash = hash;
}

int Item::sourceFeedId() const
{
    return d->feedId;
}

void Item::setSourceFeedId( int feedId )
{
    d->feedId = feedId;
}

QHash<QString, QString> Item::customProperties() const
{
    return d->customProperties;
}

QString Item::customProperty( const QString& key ) const
{
    return d->customProperties.value( key );
}

void Item::setCustomProperty( const QString& key, const QString& value )
{
    d->customProperties.insert( key, value );
}
