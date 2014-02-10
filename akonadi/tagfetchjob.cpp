/*
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "tagfetchjob.h"
#include "job_p.h"
#include "tag.h"
#include "protocolhelper_p.h"
#include <QTimer>
#include <QFile>
#include <akonadi/attributefactory.h>

using namespace Akonadi;

class Akonadi::TagFetchJobPrivate : public JobPrivate
{
public:
    TagFetchJobPrivate(TagFetchJob *parent)
        :JobPrivate(parent)
    {
    }

    void init()
    {
        Q_Q(TagFetchJob);
        mEmitTimer = new QTimer(q);
        mEmitTimer->setSingleShot(true);
        mEmitTimer->setInterval(100);
        q->connect(mEmitTimer, SIGNAL(timeout()), q, SLOT(timeout()));
        q->connect(q, SIGNAL(result(KJob*)), q, SLOT(timeout()));
    }

    void timeout()
    {
        Q_Q(TagFetchJob);
        mEmitTimer->stop(); // in case we are called by result()
        if (!mPendingTags.isEmpty()) {
            if (!q->error()) {
                emit q->tagsReceived(mPendingTags);
            }
            mPendingTags.clear();
        }
    }

    Q_DECLARE_PUBLIC(TagFetchJob)

    QList<QByteArray> mRequestedAttributes;
    Tag::List mRequestedTags;
    Tag::List mResultTags;
    Tag::List mPendingTags; // items pending for emitting itemsReceived()
    QTimer* mEmitTimer;
};

TagFetchJob::TagFetchJob(QObject *parent)
    :Job(new TagFetchJobPrivate(this), parent)
{
    Q_D(TagFetchJob);
    d->init();
}

TagFetchJob::TagFetchJob(const Tag &tag, QObject *parent)
    :Job(new TagFetchJobPrivate(this), parent)
{
    Q_D(TagFetchJob);
    d->init();
    d->mRequestedTags << tag;
}

TagFetchJob::TagFetchJob(const Tag::List& tags, QObject* parent)
    :Job(new TagFetchJobPrivate(this), parent)
{
    Q_D(TagFetchJob);
    d->init();
    d->mRequestedTags << tags;
}

void TagFetchJob::fetchAttribute(const QByteArray& type, bool fetch)
{
    Q_D(TagFetchJob);
    if (fetch) {
        d->mRequestedAttributes << type;
    }
}

void TagFetchJob::doStart()
{
    Q_D(TagFetchJob);

    QByteArray command = d->newTag();
    if (d->mRequestedTags.isEmpty()) {
        command += " UID TAGFETCH 1:*";
    } else {
        try {
            command += ProtocolHelper::tagSetToByteArray(d->mRequestedTags, "TAGFETCH");
        } catch (const Exception &e) {
            setError(Job::Unknown);
            setErrorText(QString::fromUtf8(e.what()));
            emitResult();
            return;
        }
    }
    command += " (UID";
    Q_FOREACH (const QByteArray &part, d->mRequestedAttributes) {
        command += ' ' + ProtocolHelper::encodePartIdentifier(ProtocolHelper::PartAttribute, part);
    }
    command += ")\n";

    d->writeData( command );
}

void TagFetchJob::doHandleResponse(const QByteArray &tag, const QByteArray &data)
{
    Q_D(TagFetchJob);

    if (tag == "*") {
        int begin = data.indexOf("TAGFETCH");
        if (begin >= 0) {
            // split fetch response into key/value pairs
            QList<QByteArray> fetchResponse;
            ImapParser::parseParenthesizedList(data, fetchResponse, begin + 9);

            Tag tag;

            for (int i = 0; i < fetchResponse.count() - 1; i += 2) {
                const QByteArray key = fetchResponse.value(i);
                const QByteArray value = fetchResponse.value(i + 1);

                if (key == "UID") {
                    tag.setId(value.toLongLong());
                } else if (key == "GID") {
                    tag.setGid(value);
                } else if (key == "REMOTEID") {
                    tag.setRemoteId(value);
                } else if (key == "PARENT") {
                    tag.setParent(Tag(value.toLongLong()));
                } else {
                    int version = 0;
                    QByteArray plainKey(key);
                    ProtocolHelper::PartNamespace ns;

                    ImapParser::splitVersionedKey(key, plainKey, version);
                    plainKey = ProtocolHelper::decodePartIdentifier(plainKey, ns);

                    switch (ns) {
                        case ProtocolHelper::PartAttribute:
                        {
                            Attribute* attr = AttributeFactory::createAttribute(plainKey);
                            Q_ASSERT(attr);
                            if ( value == "[FILE]" ) {
                                ++i;
                                QFile file(QString::fromUtf8(value));
                                if (file.open(QFile::ReadOnly)) {
                                    attr->deserialize(file.readAll());
                                } else {
                                    kWarning() << "Failed to open attribute file: " << value;
                                    delete attr;
                                    attr = 0;
                                }
                            } else {
                                attr->deserialize(value);
                            }
                            if (attr) {
                                tag.addAttribute(attr);
                            }
                            break;
                        }
                        default:
                            kWarning() << "Unknown item part type:" << key;
                    }
                }
            }

            if (tag.isValid() ) {
                d->mResultTags.append(tag);
                d->mPendingTags.append(tag);
                if ( !d->mEmitTimer->isActive() ) {
                    d->mEmitTimer->start();
                }
            }
            return;
        }
    }
    kDebug() << "Unhandled response: " << tag << data;
}

Tag::List TagFetchJob::tags() const
{
    Q_D(const TagFetchJob);
    return d->mResultTags;
}

#include "moc_tagfetchjob.cpp"
