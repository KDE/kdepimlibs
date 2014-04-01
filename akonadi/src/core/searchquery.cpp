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

#if 0
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif

using namespace Akonadi;

class SearchTerm::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , condition(SearchTerm::CondEqual)
        , relation(SearchTerm::RelAnd)
        , isNegated(false)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
        , key(other.key)
        , value(other.value)
        , condition(other.condition)
        , relation(other.relation)
        , terms(other.terms)
        , isNegated(other.isNegated)
    {
    }

    bool operator==(const Private &other) const
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

class SearchQuery::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , limit(-1)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
        , rootTerm(other.rootTerm)
        , limit(other.limit)
    {
    }

    bool operator==(const Private &other) const
    {
        return rootTerm == other.rootTerm && limit == other.limit;
    }

    static QVariantMap termToJSON(const SearchTerm &term)
    {
        const QList<SearchTerm> &subTerms = term.subTerms();
        QVariantMap termJSON;
        termJSON.insert(QStringLiteral("negated"), term.isNegated());
        if (subTerms.isEmpty()) {
            termJSON.insert(QStringLiteral("key"), term.key());
            termJSON.insert(QStringLiteral("value"), term.value());
            termJSON.insert(QStringLiteral("cond"), static_cast<int>(term.condition()));
        } else {
            termJSON.insert(QStringLiteral("rel"), static_cast<int>(term.relation()));
            QVariantList subTermsJSON;
            Q_FOREACH (const SearchTerm &term, subTerms) {
                subTermsJSON.append(termToJSON(term));
            }
            termJSON.insert(QStringLiteral("subTerms"), subTermsJSON);
        }

        return termJSON;
    }

    static SearchTerm JSONToTerm(const QVariantMap &json)
    {
        if (json.contains(QStringLiteral("key"))) {
            SearchTerm term(json[QStringLiteral("key")].toString(),
                            json[QStringLiteral("value")],
                            static_cast<SearchTerm::Condition>(json[QStringLiteral("cond")].toInt()));
            term.setIsNegated(json[QStringLiteral("negated")].toBool());
            return term;
        } else if (json.contains(QStringLiteral("rel"))) {
            SearchTerm term(static_cast<SearchTerm::Relation>(json[QStringLiteral("rel")].toInt()));
            term.setIsNegated(json[QStringLiteral("negated")].toBool());
            const QVariantList subTermsJSON = json[QStringLiteral("subTerms")].toList();
            Q_FOREACH (const QVariant &subTermJSON, subTermsJSON) {
                term.addSubTerm(JSONToTerm(subTermJSON.toMap()));
            }
            return term;
        } else {
            qWarning() << "Invalid JSON for term: " << json;
            return SearchTerm();
        }
    }

    SearchTerm rootTerm;
    int limit;
};

SearchTerm::SearchTerm(SearchTerm::Relation relation)
    : d(new Private)
{
    d->relation = relation;
}

SearchTerm::SearchTerm(const QString &key, const QVariant &value, SearchTerm::Condition condition)
    : d(new Private)
{
    d->relation = RelAnd;
    d->key = key;
    d->value = value;
    d->condition = condition;
}

SearchTerm::SearchTerm(const SearchTerm &other)
    : d(other.d)
{
}

SearchTerm::~SearchTerm()
{
}

SearchTerm &SearchTerm::operator=(const SearchTerm &other)
{
    d = other.d;
    return *this;
}

bool SearchTerm::operator==(const SearchTerm &other) const
{
    return *d == *other.d;
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

void SearchTerm::setIsNegated(bool negated)
{
    d->isNegated = negated;
}

bool SearchTerm::isNegated() const
{
    return d->isNegated;
}

void SearchTerm::addSubTerm(const SearchTerm &term)
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

SearchQuery::SearchQuery(SearchTerm::Relation rel)
    : d(new Private)
{
    d->rootTerm = SearchTerm(rel);
}

SearchQuery::SearchQuery(const SearchQuery &other)
    : d(other.d)
{
}

SearchQuery::~SearchQuery()
{
}

SearchQuery &SearchQuery::operator=(const SearchQuery &other)
{
    d = other.d;
    return *this;
}

bool SearchQuery::operator==(const SearchQuery &other) const
{
    return *d == *other.d;
}

bool SearchQuery::isNull() const
{
    return d->rootTerm.isNull();
}

SearchTerm SearchQuery::term() const
{
    return d->rootTerm;
}

void SearchQuery::addTerm(const QString &key, const QVariant &value, SearchTerm::Condition condition)
{
    addTerm(SearchTerm(key, value, condition));
}

void SearchQuery::addTerm(const SearchTerm &term)
{
    d->rootTerm.addSubTerm(term);
}

void SearchQuery::setTerm(const SearchTerm &term)
{
    d->rootTerm = term;
}

void SearchQuery::setLimit(int limit)
{
    d->limit = limit;
}

int SearchQuery::limit() const
{
    return d->limit;
}

QByteArray SearchQuery::toJSON() const
{
    QVariantMap root = Private::termToJSON(d->rootTerm);
    root.insert(QStringLiteral("limit"), d->limit);

#warning KF5 Port me!
#if 0
    QJson::Serializer serializer;
    return serializer.serialize(root);
#endif
    return QByteArray();
}

SearchQuery SearchQuery::fromJSON(const QByteArray &jsonData)
{
#warning KF5 Port me!
#if 0
    QJson::Parser parser;
    bool ok = false;
    const QVariant json = parser.parse(jsonData, &ok);
    if (!ok || json.isNull()) {
        return SearchQuery();
    }

    const QVariantMap map = json.toMap();
    SearchQuery query;
    query.d->rootTerm = Private::JSONToTerm(map);
    if (map.contains(QStringLiteral("limit"))) {
        query.d->limit = map.value(QStringLiteral("limit")).toInt();
    }
    return query;
#endif
    return SearchQuery();
}

QMap<EmailSearchTerm::EmailSearchField, QString> initializeMapping()
{
    QMap<EmailSearchTerm::EmailSearchField, QString> mapping;
    mapping.insert(EmailSearchTerm::Body, QStringLiteral("body"));
    mapping.insert(EmailSearchTerm::Headers, QStringLiteral("headers"));
    mapping.insert(EmailSearchTerm::Subject, QStringLiteral("subject"));
    mapping.insert(EmailSearchTerm::Message, QStringLiteral("message"));
    mapping.insert(EmailSearchTerm::HeaderFrom, QStringLiteral("from"));
    mapping.insert(EmailSearchTerm::HeaderTo, QStringLiteral("to"));
    mapping.insert(EmailSearchTerm::HeaderCC, QStringLiteral("cc"));
    mapping.insert(EmailSearchTerm::HeaderBCC, QStringLiteral("bcc"));
    mapping.insert(EmailSearchTerm::HeaderReplyTo, QStringLiteral("replyto"));
    mapping.insert(EmailSearchTerm::HeaderOrganization, QStringLiteral("organization"));
    mapping.insert(EmailSearchTerm::HeaderListId, QStringLiteral("listid"));
    mapping.insert(EmailSearchTerm::HeaderResentFrom, QStringLiteral("resentfrom"));
    mapping.insert(EmailSearchTerm::HeaderXLoop, QStringLiteral("xloop"));
    mapping.insert(EmailSearchTerm::HeaderXMailingList, QStringLiteral("xmailinglist"));
    mapping.insert(EmailSearchTerm::HeaderXSpamFlag, QStringLiteral("xspamflag"));
    mapping.insert(EmailSearchTerm::HeaderDate, QStringLiteral("date"));
    mapping.insert(EmailSearchTerm::HeaderOnlyDate, QStringLiteral("onlydate"));
    mapping.insert(EmailSearchTerm::MessageStatus, QStringLiteral("messagestatus"));
    mapping.insert(EmailSearchTerm::MessageTag, QStringLiteral("messagetag"));
    mapping.insert(EmailSearchTerm::ByteSize, QStringLiteral("size"));
    mapping.insert(EmailSearchTerm::Attachment, QStringLiteral("attachment"));
    return mapping;
}

static QMap<EmailSearchTerm::EmailSearchField, QString> emailSearchFieldMapping = initializeMapping();

EmailSearchTerm::EmailSearchTerm(EmailSearchTerm::EmailSearchField field, const QVariant &value, SearchTerm::Condition condition)
    : SearchTerm(toKey(field), value, condition)
{

}

QString EmailSearchTerm::toKey(EmailSearchTerm::EmailSearchField field)
{
    return emailSearchFieldMapping.value(field);
}

EmailSearchTerm::EmailSearchField EmailSearchTerm::fromKey(const QString &key)
{
    return emailSearchFieldMapping.key(key);
}

QMap<ContactSearchTerm::ContactSearchField, QString> initializeContactMapping()
{
    QMap<ContactSearchTerm::ContactSearchField, QString> mapping;
    mapping.insert(ContactSearchTerm::Name, QStringLiteral("name"));
    mapping.insert(ContactSearchTerm::Nickname, QStringLiteral("nickname"));
    mapping.insert(ContactSearchTerm::Email, QStringLiteral("email"));
    mapping.insert(ContactSearchTerm::Uid, QStringLiteral("uid"));
    mapping.insert(ContactSearchTerm::All, QStringLiteral("all"));
    return mapping;
}

static QMap<ContactSearchTerm::ContactSearchField, QString> contactSearchFieldMapping = initializeContactMapping();

ContactSearchTerm::ContactSearchTerm(ContactSearchTerm::ContactSearchField field, const QVariant &value, SearchTerm::Condition condition)
    : SearchTerm(toKey(field), value, condition)
{

}

QString ContactSearchTerm::toKey(ContactSearchTerm::ContactSearchField field)
{
    return contactSearchFieldMapping.value(field);
}

ContactSearchTerm::ContactSearchField ContactSearchTerm::fromKey(const QString &key)
{
    return contactSearchFieldMapping.key(key);
}
