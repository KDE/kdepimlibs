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

#include "tagcreatejob.h"
#include "job_p.h"
#include "tag.h"
#include "protocolhelper_p.h"

using namespace Akonadi;

struct Akonadi::TagCreateJobPrivate : public JobPrivate
{
    TagCreateJobPrivate(TagCreateJob *parent)
        :JobPrivate(parent)
    {
    }

    Tag mTag;
};

TagCreateJob::TagCreateJob(const Akonadi::Tag &tag, QObject *parent)
    :Job(new TagCreateJobPrivate(this), parent)
{
    Q_D(TagCreateJob);
    d->mTag = tag;
}

void TagCreateJob::doStart()
{
    Q_D(TagCreateJob);

    QByteArray command = d->newTag() + " TAGAPPEND (";

    QList<QByteArray> list;
    if (!d->mTag.gid().isEmpty()) {
        list << "GID";
        list << ImapParser::quote(d->mTag.gid());
    }
    if (!d->mTag.remoteId().isEmpty()) {
        list << "RID";
        list << ImapParser::quote(d->mTag.remoteId());
    }
    if (d->mTag.parent().isValid()) {
        list << "PARENT";
        list << QString::number(d->mTag.parent().id()).toLatin1();
    }
    command += ImapParser::join(list, " ");
    command += " "; // list of parts
    const QByteArray attrs = ProtocolHelper::attributesToByteArray(d->mTag, true);
    if (!attrs.isEmpty()) {
        command += attrs;
    }
    command += ")";

    d->writeData( command );
    d->mTag = Tag();
}

void TagCreateJob::doHandleResponse(const QByteArray &tag, const QByteArray &data)
{
    Q_D(TagCreateJob);

    if ( tag == "*" ) {
        int begin = data.indexOf("TAGAPPEND");
        if ( begin >= 0 ) {
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
                }
            }

            if ( !tag.isValid() ) {
              kWarning() << "got invalid tag back";
              return;
            }
            d->mTag = tag;
        }
    }
}

Tag TagCreateJob::tag() const
{
    Q_D(const TagCreateJob);

    return d->mTag;
}
