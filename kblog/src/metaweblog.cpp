/*
  This file is part of the kblog library.

  Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "metaweblog.h"
#include "metaweblog_p.h"
#include "blogpost.h"
#include "blogmedia.h"

#include <kxmlrpcclient/client.h>
#include <QDebug>
#include <KLocalizedString>
#include <KDateTime>

#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

using namespace KBlog;

MetaWeblog::MetaWeblog(const QUrl &server, QObject *parent)
    : Blogger1(server, *new MetaWeblogPrivate, parent)
{
    qDebug();
}

MetaWeblog::MetaWeblog(const QUrl &server, MetaWeblogPrivate &dd, QObject *parent)
    : Blogger1(server, dd, parent)
{
    qDebug();
}

MetaWeblog::~MetaWeblog()
{
    qDebug();
}

QString MetaWeblog::interfaceName() const
{
    return QLatin1String("MetaWeblog");
}

void MetaWeblog::listCategories()
{
    Q_D(MetaWeblog);
    qDebug() << "Fetching List of Categories...";
    QList<QVariant> args(d->defaultArgs(blogId()));
    d->mXmlRpcClient->call(
        QStringLiteral("metaWeblog.getCategories"), args,
        this, SLOT(slotListCategories(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)));
}

void MetaWeblog::createMedia(KBlog::BlogMedia *media)
{
    Q_D(MetaWeblog);
    if (!media) {
        qCritical() << "MetaWeblog::createMedia: media is a null pointer";
        emit error(Other, i18n("Media is a null pointer."));
        return;
    }
    unsigned int i = d->mCallMediaCounter++;
    d->mCallMediaMap[ i ] = media;
    qDebug() << "MetaWeblog::createMedia: name=" << media->name();
    QList<QVariant> args(d->defaultArgs(blogId()));
    QMap<QString, QVariant> map;
    map[QStringLiteral("name")] = media->name();
    map[QStringLiteral("type")] = media->mimetype();
    map[QStringLiteral("bits")] = media->data();
    args << map;
    d->mXmlRpcClient->call(
        QStringLiteral("metaWeblog.newMediaObject"), args,
        this, SLOT(slotCreateMedia(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));

}

MetaWeblogPrivate::MetaWeblogPrivate()
{
    qDebug();
    mCallMediaCounter = 1;
    mCatLoaded = false;
}

MetaWeblogPrivate::~MetaWeblogPrivate()
{
    qDebug();
}

QList<QVariant> MetaWeblogPrivate::defaultArgs(const QString &id)
{
    Q_Q(MetaWeblog);
    QList<QVariant> args;
    if (!id.isEmpty()) {
        args << QVariant(id);
    }
    args << QVariant(q->username())
         << QVariant(q->password());
    return args;
}

void MetaWeblogPrivate::loadCategories()
{
    qDebug();

    if (mCatLoaded) {
        return;
    }
    mCatLoaded = true;

    if (mUrl.isEmpty() || mBlogId.isEmpty() || mUsername.isEmpty()) {
        qDebug() << "We need at least url, blogId and the username to create a unique filename.";
        return;
    }

    QString filename = QStringLiteral("kblog/") + mUrl.host() + QLatin1Char('_') + mBlogId + QLatin1Char('_') + mUsername;
    filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + filename;
    QDir().mkpath(QFileInfo(filename).absolutePath());
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open cached categories file: " << filename;
        return;
    }

    QDataStream stream(&file);
    stream >> mCategoriesList;
    file.close();
}

void MetaWeblogPrivate::saveCategories()
{
    qDebug();
    if (mUrl.isEmpty() || mBlogId.isEmpty() || mUsername.isEmpty()) {
        qDebug() << "We need at least url, blogId and the username to create a unique filename.";
        return;
    }

    QString filename = QStringLiteral("kblog/") + mUrl.host() + QLatin1Char('_') + mBlogId + QLatin1Char('_') + mUsername;
    filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + filename;
    QDir().mkpath(QFileInfo(filename).absolutePath());
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open cached categories file: " << filename;
        return;
    }

    QDataStream stream(&file);
    stream << mCategoriesList;
    file.close();
}

void MetaWeblogPrivate::slotListCategories(const QList<QVariant> &result,
        const QVariant &id)
{
    Q_Q(MetaWeblog);
    Q_UNUSED(id);

    qDebug() << "MetaWeblogPrivate::slotListCategories";
    qDebug() << "TOP:" << result[0].typeName();
    if (result[0].type() != QVariant::Map &&
            result[0].type() != QVariant::List) {
        // include fix for not metaweblog standard compatible apis with
        // array of structs instead of struct of structs, e.g. wordpress
        qCritical() << "Could not list categories out of the result from the server.";
        emit q->error(MetaWeblog::ParsingError,
                      i18n("Could not list categories out of the result "
                           "from the server."));
    } else {
        if (result[0].type() == QVariant::Map) {
            const QMap<QString, QVariant> serverMap = result[0].toMap();
            const QList<QString> serverKeys = serverMap.keys();

            QList<QString>::ConstIterator it = serverKeys.begin();
            QList<QString>::ConstIterator end = serverKeys.end();
            for (; it != end; ++it) {
                qDebug() << "MIDDLE:" << (*it);
                QMap<QString, QString> category;
                const QMap<QString, QVariant> serverCategory = serverMap[*it].toMap();
                category[QStringLiteral("name")] = (*it);
                category[QStringLiteral("description")] = serverCategory[ QStringLiteral("description") ].toString();
                category[QStringLiteral("htmlUrl")] = serverCategory[ QStringLiteral("htmlUrl") ].toString();
                category[QStringLiteral("rssUrl")] = serverCategory[ QStringLiteral("rssUrl") ].toString();
                category[QStringLiteral("categoryId")] = serverCategory[ QStringLiteral("categoryId") ].toString();
                category[QStringLiteral("parentId")] = serverCategory[ QStringLiteral("parentId") ].toString();
                mCategoriesList.append(category);
            }
            qDebug() << "Emitting listedCategories";
            emit q->listedCategories(mCategoriesList);
        }
    }
    if (result[0].type() == QVariant::List) {
        // include fix for not metaweblog standard compatible apis with
        // array of structs instead of struct of structs, e.g. wordpress
        const QList<QVariant> serverList = result[0].toList();
        QList<QVariant>::ConstIterator it = serverList.begin();
        QList<QVariant>::ConstIterator end = serverList.end();
        for (; it != end; ++it) {
            qDebug() << "MIDDLE:" << (*it).typeName();
            QMap<QString, QString> category;
            const QMap<QString, QVariant> serverCategory = (*it).toMap();
            category[ QStringLiteral("name") ] = serverCategory[QStringLiteral("categoryName")].toString();
            category[QStringLiteral("description")] = serverCategory[ QStringLiteral("description") ].toString();
            category[QStringLiteral("htmlUrl")] = serverCategory[ QStringLiteral("htmlUrl") ].toString();
            category[QStringLiteral("rssUrl")] = serverCategory[ QStringLiteral("rssUrl") ].toString();
            category[QStringLiteral("categoryId")] = serverCategory[ QStringLiteral("categoryId") ].toString();
            category[QStringLiteral("parentId")] = serverCategory[ QStringLiteral("parentId") ].toString();
            mCategoriesList.append(category);
        }
        qDebug() << "Emitting listedCategories()";
        emit q->listedCategories(mCategoriesList);
    }
    saveCategories();
}

void MetaWeblogPrivate::slotCreateMedia(const QList<QVariant> &result,
                                        const QVariant &id)
{
    Q_Q(MetaWeblog);

    KBlog::BlogMedia *media = mCallMediaMap[ id.toInt() ];
    mCallMediaMap.remove(id.toInt());

    qDebug() << "MetaWeblogPrivate::slotCreateMedia, no error!";
    qDebug() << "TOP:" << result[0].typeName();
    if (result[0].type() != 8) {
        qCritical() << "Could not read the result, not a map.";
        emit q->errorMedia(MetaWeblog::ParsingError,
                           i18n("Could not read the result, not a map."),
                           media);
        return;
    }
    const QMap<QString, QVariant> resultStruct = result[0].toMap();
    const QString url = resultStruct[QStringLiteral("url")].toString();
    qDebug() << "MetaWeblog::slotCreateMedia url=" << url;

    if (!url.isEmpty()) {
        media->setUrl(QUrl(url));
        media->setStatus(BlogMedia::Created);
        qDebug() << "Emitting createdMedia( url=" << url  << ");";
        emit q->createdMedia(media);
    }
}

bool MetaWeblogPrivate::readPostFromMap(BlogPost *post,
                                        const QMap<QString, QVariant> &postInfo)
{
    // FIXME: integrate error handling
    qDebug() << "readPostFromMap()";
    if (!post) {
        return false;
    }
    QStringList mapkeys = postInfo.keys();
    qDebug() << endl << "Keys:" << mapkeys.join(QStringLiteral(", "));
    qDebug() << endl;

    KDateTime dt =
        KDateTime(postInfo[QStringLiteral("dateCreated")].toDateTime(), KDateTime::UTC);
    if (dt.isValid() && !dt.isNull()) {
        post->setCreationDateTime(dt.toLocalZone());
    }

    dt =
        KDateTime(postInfo[QStringLiteral("lastModified")].toDateTime(), KDateTime::UTC);
    if (dt.isValid() && !dt.isNull()) {
        post->setModificationDateTime(dt.toLocalZone());
    }

    post->setPostId(postInfo[QStringLiteral("postid")].toString().isEmpty() ? postInfo[QStringLiteral("postId")].toString() :
                    postInfo[QStringLiteral("postid")].toString());

    QString title(postInfo[QStringLiteral("title")].toString());
    QString description(postInfo[QStringLiteral("description")].toString());
    QStringList categories(postInfo[QStringLiteral("categories")].toStringList());

    post->setTitle(title);
    post->setContent(description);
    if (!categories.isEmpty()) {
        qDebug() << "Categories:" << categories;
        post->setCategories(categories);
    }
    return true;
}

bool MetaWeblogPrivate::readArgsFromPost(QList<QVariant> *args, const BlogPost &post)
{
    if (!args) {
        return false;
    }
    QMap<QString, QVariant> map;
    map[QStringLiteral("categories")] = post.categories();
    map[QStringLiteral("description")] = post.content();
    map[QStringLiteral("title")] = post.title();
    map[QStringLiteral("lastModified")] = post.modificationDateTime().dateTime().toUTC();
    map[QStringLiteral("dateCreated")] = post.creationDateTime().dateTime().toUTC();
    *args << map;
    *args << QVariant(!post.isPrivate());
    return true;
}

QString MetaWeblogPrivate::getCallFromFunction(FunctionToCall type)
{
    switch (type) {
    case GetRecentPosts: return QStringLiteral("metaWeblog.getRecentPosts");
    case CreatePost:        return QStringLiteral("metaWeblog.newPost");
    case ModifyPost:       return QStringLiteral("metaWeblog.editPost");
    case FetchPost:        return QStringLiteral("metaWeblog.getPost");
    default: return QString();
    }
}
#include "moc_metaweblog.cpp"
