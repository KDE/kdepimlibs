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


#ifndef AKONADI_SEARCHQUERY_H
#define AKONADI_SEARCHQUERY_H

#include <QSharedPointer>

#include "akonadi_export.h"

namespace Akonadi
{

/**
 * Search term represents the actual condition within query.
 *
 * SearchTerm can either have multiple subterms, or can be so-called endterm, when
 * there are no more subterms, but instead the actual condition is specified, that
 * is have key, value and relation between them.
 *
 * @since 4.13
 */
class AKONADI_EXPORT SearchTerm
{
  public:
    enum Relation {
      RelAnd,
      RelOr
    };

    enum Condition {
      CondEqual,
      CondGreaterThan,
      CondGreaterOrEqual,
      CondLessThan,
      CondLessOrEqueal,
      CondContains
    };

    /**
     * Constructs a term where all subterms will be in given relation
     */
    SearchTerm( SearchTerm::Relation relation = SearchTerm::RelAnd );

    /**
     * Constructs an end term
     */
    SearchTerm( const QString &key, const QVariant &value, SearchTerm::Condition condition = SearchTerm::CondEqual );

    SearchTerm( const SearchTerm &other );
    ~SearchTerm();

    SearchTerm& operator=( const SearchTerm &other );
    bool operator==( const SearchTerm &other );

    bool isNull() const;

    /**
     * Returns key of this end term.
     */
    QString key() const;

    /**
     * Returns value of this end term.
     */
    QVariant value() const;

    /**
     * Returns relation between key and value.
     */
    SearchTerm::Condition condition() const;

    /**
     * Adds a new subterm to this term.
     *
     * Subterms will be in relation as specified in SearchTerm constructor.
     *
     * If there are subterms in a term, key, value and condition are ignored.
     */
    void addSubTerm( const SearchTerm &term );

    /**
     * Returns all subterms, or an empty list if this is an end term.
     */
    QList<SearchTerm> subTerms() const;

    /**
     * Returns relation in which all subterms are.
     */
    SearchTerm::Relation relation() const;

    /**
     * Sets whether the entire term is negated.
     */
    void setIsNegated( bool negated );

    /**
     * Returns whether the entire term is negated.
     */
    bool isNegated() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
 * @brief A query that can be passed to ItemSearchJob or others.
 *
 * @since 4.13
 */
class AKONADI_EXPORT SearchQuery
{
  public:
    /**
     * Constructs query where all added terms will be in given relation
     */
    SearchQuery( SearchTerm::Relation rel = SearchTerm::RelAnd );

    ~SearchQuery();
    SearchQuery( const SearchQuery &other );
    SearchQuery& operator=( const SearchQuery &other );
    bool operator==( const SearchQuery &other );

    bool isNull() const;

    /**
     * Adds a new term.
     */
    void addTerm( const QString &key, const QVariant &value, SearchTerm::Condition condition = SearchTerm::CondEqual );

    /**
     * Adds a new term with subterms
     */
    void addTerm( const SearchTerm &term );

    /**
     * Sets the root term
     */
    void setTerm( const SearchTerm &term );

    /**
     * Returns the root term.
     */
    SearchTerm term() const;

    QByteArray toJSON() const;
    static SearchQuery fromJSON( const QByteArray &json );

  private:
    class Private;
    QSharedDataPointer<Private> d;

};

/**
 * A search term for an email field.
 *
 * This class can be used to create queries that akonadi email search backends understand.
 *
 * TODO support everything from IMAP:
 * Flag searches:
 * ALL, ANSWERED, DELETED, DRAFT, FLAGGED, SEEN, UN* (NOT flag)
 *
 * Text searches:
 * BCC, CC, BODY, FROM, HEADER <field-name> <string>, TO, UID, SUBJECT, TEXT (body or header of message)
 *
 * KEYWORD <flag>
 *
 * Property searches:
 * BEFORE/SINCE/ON <date>, LARGER/SMALLER <n>, SENTBEFORE/SENTSINCE/SENTON (Date:), SINCE (internal date)
 *
 *
 */
class AKONADI_EXPORT EmailSearchTerm : public SearchTerm
{
  public:
    enum EmailSearchField {
      Unknown,
      Body, //textsearch on body of message
      Headers, //textsearch on all headers
      Recipients, //textsearch on recipient list
      Subject, //textsearch on subject
      From, //textsearch on From: header
      To, //textsearch on To: header
      CC, //textsearch on CC: header
      BCC, //textsearch on BCC: header
      MessageTag, //textsearch on message tags
      ReplyTo, //textsearch on ?
      Organization, //textsearch ?
      ListId, //textsearch?
      ResentFrom, //textsearch?
      XLoop, //textsearch?
      XMailingList, //textsearch?
      XSpamFlag, //textsearch?
      All, //textsearch on everything
      MessageStatus, //Message flags (see Akonadi::MessageFlags)
      Age, //datesearch on (internaldate or Date: ?)
      Date, //datesearch on (internaldate or Date: ?)
      Size, //search on bytesize
      Attachment //textsearch on attachment
    };

    /**
     * Constructs an email end term
     */
    EmailSearchTerm( EmailSearchField field, const QVariant &value, SearchTerm::Condition condition = SearchTerm::CondEqual );

    /**
     * Translates field to key
     */
    static QString toKey(EmailSearchField);

    /**
     * Translates key to field
     */
    static EmailSearchField fromKey(const QString &key);
};

}

#endif // AKONADI_SEARCHQUERY_H
