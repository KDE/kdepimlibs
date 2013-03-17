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

#ifndef KRSS_ITEM_H
#define KRSS_ITEM_H

#include "krss_export.h"

#include <akonadi/item.h>

#include <QtCore/QSharedDataPointer>

class KDateTime;

template <class T> class QList;
template <class K, class T> class QHash;
class QString;

namespace KRss {

class Category;
class Enclosure;
class Person;

typedef qint64 ItemId;

enum ItemStatusFlag {
    Unread      = 0x02,
    Important   = 0x04,
    Deleted     = 0x08,
    Updated     = 0x16
};
Q_DECLARE_FLAGS( ItemStatus, ItemStatusFlag )

KRSS_EXPORT
ItemStatus itemStatus( const Akonadi::Item& item );

KRSS_EXPORT
void setItemStatus( Akonadi::Item& aitem, const ItemStatus& stat );

class KRSS_EXPORT Item
{
public:
      // mimetype
    static QString mimeType();

    static const char *HeadersPart;     //  All attributes except description + content
    static const char *ContentPart;     //  Description + Content

    // return the corresponding Akonadi flags
    // for the status flags defined in item.h
    static QByteArray flagRead();
    static QByteArray flagImportant();
    static QByteArray flagDeleted();
    static QByteArray flagUpdated();

    static ItemId itemIdFromAkonadi( const Akonadi::Item::Id& id );
    static Akonadi::Item::Id itemIdToAkonadi( const ItemId& itemId );

    Item();
    Item( const Item& other );
    ~Item();

    void swap( Item& other );
    Item& operator=( const Item& other );
    bool operator==( const Item& other ) const;
    bool operator!=( const Item& other ) const;

    static bool isImportant( const Akonadi::Item& );
    static bool isRead( const Akonadi::Item& );
    static bool isUnread( const Akonadi::Item& );
    static bool isDeleted( const Akonadi::Item& );
    static bool isUpdated( const Akonadi::Item& );

    bool headersLoaded() const;
    void setHeadersLoaded( bool headersLoaded );
    bool contentLoaded() const;
    void setContentLoaded( bool contentLoaded );

    int hash() const;
    void setHash( int hash );

    bool guidIsHash() const;
    void setGuidIsHash( bool isHash );

    int sourceFeedId() const;
    void setSourceFeedId( int id );

    /**
     * The title of the item.
     *
     * This string might contain HTML markup.(Importantly, occurrences of
     * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
     *
     * @return the title of the item as HTML, or a null string if not
     * specified
     */
    QString title() const;
    void setTitle( const QString& title );
    QString titleAsPlainText() const;

    /**
     * returns a link to the (web) resource described by this item. In most
     * cases, this will be a website containing the full article associated
     * with this item.
     *
     * @return a URL, or a null string if not specified
     */
    QString link() const;
    void setLink( const QString& link );

    /**
     * returns the description of the item. The description can either be
     * a tag line, a short summary of the item content up to a complete
     * article. If content() is non-empty, it
     *
     * This string may contain HTML markup. (Importantly, occurrences of
     * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
     *
     * @return the description as HTML, or a null string if not specified
     */
    QString description() const;
    void setDescription( const QString& description );

    /**
     * returns the content of the item. If provided, this is the most
     * comprehensive text content available for this item. If it is empty,
     * use description() (which might also contain complete article
     * content).
     *
     * This string may contain HTML markup. (Importantly, occurrences of
     * the characters &lt;,'\n', '&amp;', '\'' and  '\"' are escaped).
     *
     * @return content string as HTML, or a null string if not set
     */
    QString content() const;
    void setContent( const QString& content );

    /**
     * Convenience function to get content if available, and description otherwise.
     * If content is available, it's returned.
     * If the content was loaded but is empty, returns the description instead.
     * If the content wasn't retrieved, returns an empty string.
     */
    QString contentWithDescriptionAsFallback() const;

    /**
     * returns the date when the item was initially published.
     *
     * @return publication date, as seconds since epoch (Jan 1st 1970), or 0
     * (epoch) if not set
     */
    KDateTime datePublished() const;
    void setDatePublished( const KDateTime& date );

    /**
     * returns the date when the item was modified the last time. If no such
     * date is provided by the feed, this method returns the value of
     * datePublished().
     *
     * @return modification date, as seconds since epoch (Jan 1st 1970)
     */
    KDateTime dateUpdated() const;
    void setDateUpdated( const KDateTime& date );

    /**
     * returns an identifier that identifies the item within its
     * feed. The ID must be unique within its feed. If no ID is provided
     * by the feed source, a hash from title, description and content is
     * returned.
     * Generated hash IDs start with "hash:".
     */
    QString guid() const;
    void setGuid( const QString& guid );

    /**
     * returns a list of persons who created the item content. If there is a
     * distinction between authors and contributors (Atom), both are added
     * to the list, where authors are added first.
     *
     * @return list of authors (and possibly other contributing persons)
     */
    QList<Person> authors() const;
    void setAuthors( const QList<Person>& authors );

    QString authorsAsHtml() const;

    /**
     * returns the language used in the item's content
     *
     * @return TODO: tell about language codes and link them
     */
    QString language() const;
    void setLanguage( const QString& language );

    /**
     * returns a list of enclosures describing files available on the net.
     * (often used for audio files, so-called "Podcasts").
     *
     * @return a list of enclosures associated with this item
     */
    QList<Enclosure> enclosures() const;
    void setEnclosures( const QList<Enclosure>& enclosures );

    /**
     * returns a list of categories this item is filed in.
     * See Category for more information on categories.
     *
     * @return a list of categories
     */
    QList<Category> categories() const;
    void setCategories( const QList<Category>& categories );

    /**
     * The number of comments posted for this item.
     *
     * @return the number of comments associated to this item, or -1 if not
     * specified
     */
    int commentsCount() const;
    void setCommentsCount( int count );

    /**
     * Link to an HTML site which contains the comments belonging to
     * this item.
     *
     * @return URL to the comments page, or a null string if not set
     */
    QString commentsLink() const;
    void setCommentsLink( const QString& link );

    /**
     * URL of feed syndicating comments belonging to this item.
     *
     * @return comments feed URL, or a null string if not set
     */
    QString commentsFeed() const;
    void setCommentsFeed( const QString& feed );

    /**
     * URI that can be used to post comments via an HTTP POST request using
     * the Comment API.
     * For more details on the Comment API, see
     * http://wellformedweb.org/story/9
     *
     * @return URI for posting comments, or a null string if not set
     */
    QString commentPostUri() const;
    void setCommentPostUri( const QString& uri );

    QHash<QString, QString> customProperties() const;
    QString customProperty( const QString& key ) const;
    void setCustomProperty( const QString& key, const QString& value );

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace KRss

Q_DECLARE_METATYPE( KRss::Item )

#endif // KRSS_RSSITEM_H
