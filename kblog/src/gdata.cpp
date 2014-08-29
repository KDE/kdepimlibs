/*
    This file is part of the kblog library.

    Copyright (c) 2007 Christian Weilbach <christian_weilbach@web.de>

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

#include "gdata.h"
#include "gdata_p.h"
#include "blogpost.h"
#include "blogcomment.h"

#include <syndication/loader.h>
#include <syndication/item.h>
#include <syndication/category.h>

#include <kio/netaccess.h>
#include <kio/job.h>
#include <QDebug>
#include <KLocalizedString>
#include <KDateTime>
#include <QUrl>

#include <QByteArray>
#include <QRegExp>

#define TIMEOUT 600

using namespace KBlog;

GData::GData(const QUrl &server, QObject *parent)
    : Blog(server, *new GDataPrivate, parent)
{
    qDebug();
    setUrl(server);
}

GData::~GData()
{
    qDebug();
}

QString GData::interfaceName() const
{
    qDebug();
    return QLatin1String("Google Blogger Data");
}

QString GData::fullName() const
{
    qDebug();
    return d_func()->mFullName;
}

void GData::setFullName(const QString &fullName)
{
    qDebug();
    Q_D(GData);
    d->mFullName = fullName;
}

QString GData::profileId() const
{
    qDebug();
    return d_func()->mProfileId;
}

void GData::setProfileId(const QString &pid)
{
    qDebug();
    Q_D(GData);
    d->mProfileId = pid;
}

void GData::fetchProfileId()
{
    qDebug();
    QByteArray data;
    KIO::StoredTransferJob *job = KIO::storedGet(url(), KIO::NoReload, KIO::HideProgressInfo);
    QUrl blogUrl = url();
    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotFetchProfileId(KJob*)));
}

void GData::listBlogs()
{
    qDebug();
    Syndication::Loader *loader = Syndication::Loader::create();
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListBlogs(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QString::fromLatin1("http://www.blogger.com/feeds/%1/blogs").arg(profileId())));
}

void GData::listRecentPosts(const QStringList &labels, int number,
                            const KDateTime &upMinTime, const KDateTime &upMaxTime,
                            const KDateTime &pubMinTime, const KDateTime &pubMaxTime)
{
    qDebug();
    Q_D(GData);
    QString urlString(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default"));
    if (! labels.empty()) {
        urlString += QStringLiteral("/-/") + labels.join(QStringLiteral("/"));
    }
    qDebug() << "listRecentPosts()";
    QUrl url(urlString);

    if (!upMinTime.isNull()) {
        url.addQueryItem(QStringLiteral("updated-min"), upMinTime.toString());
    }

    if (!upMaxTime.isNull()) {
        url.addQueryItem(QStringLiteral("updated-max"), upMaxTime.toString());
    }

    if (!pubMinTime.isNull()) {
        url.addQueryItem(QStringLiteral("published-min"), pubMinTime.toString());
    }

    if (!pubMaxTime.isNull()) {
        url.addQueryItem(QStringLiteral("published-max"), pubMaxTime.toString());
    }

    Syndication::Loader *loader = Syndication::Loader::create();
    if (number > 0) {
        d->mListRecentPostsMap[ loader ] = number;
    }
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListRecentPosts(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(url);
}

void GData::listRecentPosts(int number)
{
    qDebug();
    listRecentPosts(QStringList(), number);
}

void GData::listComments(KBlog::BlogPost *post)
{
    qDebug();
    Q_D(GData);
    Syndication::Loader *loader = Syndication::Loader::create();
    d->mListCommentsMap[ loader ] = post;
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QString(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QLatin1Char('/') +
                                  post->postId() + QStringLiteral("/comments/default"))));
}

void GData::listAllComments()
{
    qDebug();
    Syndication::Loader *loader = Syndication::Loader::create();
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotListAllComments(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QString::fromLatin1("http://www.blogger.com/feeds/%1/comments/default").arg(blogId())));
}

void GData::fetchPost(KBlog::BlogPost *post)
{
    qDebug();
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    qDebug();
    Syndication::Loader *loader = Syndication::Loader::create();
    d->mFetchPostMap[ loader ] = post;
    connect(loader,
            SIGNAL(loadingComplete(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)),
            this,
            SLOT(slotFetchPost(Syndication::Loader*,Syndication::FeedPtr,Syndication::ErrorCode)));
    loader->loadFrom(QUrl(QString::fromLatin1("http://www.blogger.com/feeds/%1/posts/default").arg(blogId())));
}

void GData::modifyPost(KBlog::BlogPost *post)
{
    qDebug();
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        emit errorPost(Atom, i18n("Authentication failed."), post);
        return;
    }

    QString atomMarkup = QStringLiteral("<entry xmlns='http://www.w3.org/2005/Atom'>");
    atomMarkup += QStringLiteral("<id>tag:blogger.com,1999:blog-") + blogId();
    atomMarkup += QStringLiteral(".post-") + post->postId() + QStringLiteral("</id>");
    atomMarkup += QStringLiteral("<published>") + post->creationDateTime().toString() + QStringLiteral("</published>");
    atomMarkup += QStringLiteral("<updated>") + post->modificationDateTime().toString() + QStringLiteral("</updated>");
    atomMarkup += QStringLiteral("<title type='text'>") + post->title() + QStringLiteral("</title>");
    if (post->isPrivate()) {
        atomMarkup += QStringLiteral("<app:control xmlns:app='http://purl.org/atom/app#'>");
        atomMarkup += QStringLiteral("<app:draft>yes</app:draft></app:control>");
    }
    atomMarkup += QStringLiteral("<content type='xhtml'>");
    atomMarkup += QStringLiteral("<div xmlns='http://www.w3.org/1999/xhtml'>");
    atomMarkup += post->content();
    atomMarkup += QStringLiteral("</div></content>");
    QList<QString>::ConstIterator it = post->tags().constBegin();
    QList<QString>::ConstIterator end = post->tags().constEnd();
    for (; it != end; ++it) {
        atomMarkup += QStringLiteral("<category scheme='http://www.blogger.com/atom/ns#' term='") + (*it) + QStringLiteral("' />");
    }
    atomMarkup += QStringLiteral("<author>");
    if (!fullName().isEmpty()) {
        atomMarkup += QStringLiteral("<name>") + fullName() + QStringLiteral("</name>");
    }
    atomMarkup += QStringLiteral("<email>") + username() + QStringLiteral("</email>");
    atomMarkup += QStringLiteral("</author>");
    atomMarkup += QStringLiteral("</entry>");
    QByteArray postData;
    QDataStream stream(&postData, QIODevice::WriteOnly);
    stream.writeRawData(atomMarkup.toUtf8(), atomMarkup.toUtf8().length());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default/") + post->postId()),
                                  KIO::HideProgressInfo);

    Q_ASSERT(job);

    d->mModifyPostMap[ job ] = post;

    job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/atom+xml; charset=utf-8"));
    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString +
                     QStringLiteral("\r\nX-HTTP-Method-Override: PUT"));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotModifyPost(KJob*)));
}

void GData::createPost(KBlog::BlogPost *post)
{
    qDebug();
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        emit errorPost(Atom, i18n("Authentication failed."), post);
        return;
    }

    QString atomMarkup = QStringLiteral("<entry xmlns='http://www.w3.org/2005/Atom'>");
    atomMarkup += QStringLiteral("<title type='text'>") + post->title() + QStringLiteral("</title>");
    if (post->isPrivate()) {
        atomMarkup += QStringLiteral("<app:control xmlns:app='http://purl.org/atom/app#'>");
        atomMarkup += QStringLiteral("<app:draft>yes</app:draft></app:control>");
    }
    atomMarkup += QStringLiteral("<content type='xhtml'>");
    atomMarkup += QStringLiteral("<div xmlns='http://www.w3.org/1999/xhtml'>");
    atomMarkup += post->content(); // FIXME check for Utf
    atomMarkup += QStringLiteral("</div></content>");
    QList<QString>::ConstIterator it = post->tags().constBegin();
    QList<QString>::ConstIterator end = post->tags().constEnd();
    for (; it != end; ++it) {
        atomMarkup += QStringLiteral("<category scheme='http://www.blogger.com/atom/ns#' term='") + (*it) + QStringLiteral("' />");
    }
    atomMarkup += QStringLiteral("<author>");
    if (!fullName().isEmpty()) {
        atomMarkup += QStringLiteral("<name>") + fullName() + QStringLiteral("</name>");
    }
    atomMarkup += QStringLiteral("<email>") + username() + QStringLiteral("</email>");
    atomMarkup += QStringLiteral("</author>");
    atomMarkup += QStringLiteral("</entry>");

    QByteArray postData;
    QDataStream stream(&postData, QIODevice::WriteOnly);
    stream.writeRawData(atomMarkup.toUtf8(), atomMarkup.toUtf8().length());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default")),
                                  KIO::HideProgressInfo);

    Q_ASSERT(job);
    d->mCreatePostMap[ job ] = post;

    job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/atom+xml; charset=utf-8"));
    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotCreatePost(KJob*)));
}

void GData::removePost(KBlog::BlogPost *post)
{
    qDebug();
    Q_D(GData);

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        emit errorPost(Atom, i18n("Authentication failed."), post);
        return;
    }

    QByteArray postData;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/posts/default/") + post->postId()),
                                  KIO::HideProgressInfo);

    d->mRemovePostMap[ job ] = post;

    if (!job) {
        qWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
                   << blogId() << QStringLiteral("/posts/default/") + post->postId();
    }

    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString +
                     QStringLiteral("\r\nX-HTTP-Method-Override: DELETE"));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotRemovePost(KJob*)));
}

void GData::createComment(KBlog::BlogPost *post, KBlog::BlogComment *comment)
{
    qDebug();

    if (!comment) {
        qCritical() << "comment is null pointer";
        return;
    }

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    Q_D(GData);
    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        emit errorComment(Atom, i18n("Authentication failed."), post, comment);
        return;
    }
    QString atomMarkup = QStringLiteral("<entry xmlns='http://www.w3.org/2005/Atom'>");
    atomMarkup += QStringLiteral("<title type=\"text\">") + comment->title() + QStringLiteral("</title>");
    atomMarkup += QStringLiteral("<content type=\"html\">") + comment->content() + QStringLiteral("</content>");
    atomMarkup += QStringLiteral("<author>");
    atomMarkup += QStringLiteral("<name>") + comment->name() + QStringLiteral("</name>");
    atomMarkup += QStringLiteral("<email>") + comment->email() + QStringLiteral("</email>");
    atomMarkup += QStringLiteral("</author></entry>");

    QByteArray postData;
    qDebug() <<  postData;
    QDataStream stream(&postData, QIODevice::WriteOnly);
    stream.writeRawData(atomMarkup.toUtf8(), atomMarkup.toUtf8().length());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/") + post->postId() + QStringLiteral("/comments/default")),
                                  KIO::HideProgressInfo);

    d->mCreateCommentMap[ job ][post] = comment;

    if (!job) {
        qWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
                   << blogId() << "/" << post->postId() << "/comments/default";
    }

    job->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/atom+xml; charset=utf-8"));
    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") + d->mAuthenticationString);
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotCreateComment(KJob*)));
}

void GData::removeComment(KBlog::BlogPost *post, KBlog::BlogComment *comment)
{
    qDebug();
    Q_D(GData);
    qDebug();

    if (!comment) {
        qCritical() << "comment is null pointer";
        return;
    }

    if (!post) {
        qCritical() << "post is null pointer";
        return;
    }

    if (!d->authenticate()) {
        qCritical() << "Authentication failed.";
        emit errorComment(Atom, i18n("Authentication failed."), post, comment);
        return;
    }

    QByteArray postData;

    KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,
                                  QUrl(QStringLiteral("http://www.blogger.com/feeds/") + blogId() + QStringLiteral("/") + post->postId() +
                                       QStringLiteral("/comments/default/") + comment->commentId()), KIO::HideProgressInfo);
    d->mRemoveCommentMap[ job ][ post ] = comment;

    if (!job) {
        qWarning() << "Unable to create KIO job for http://www.blogger.com/feeds/"
                   << blogId() << post->postId()
                   << "/comments/default/" << comment->commentId();
    }

    job->addMetaData(QStringLiteral("ConnectTimeout"), QStringLiteral("50"));
    job->addMetaData(QStringLiteral("UserAgent"), userAgent());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: GoogleLogin auth=") +
                     d->mAuthenticationString + QStringLiteral("\r\nX-HTTP-Method-Override: DELETE"));

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotRemoveComment(KJob*)));
}

GDataPrivate::GDataPrivate(): mAuthenticationString(), mAuthenticationTime()
{
    qDebug();
}

GDataPrivate::~GDataPrivate()
{
    qDebug();
}

bool GDataPrivate::authenticate()
{
    qDebug();
    Q_Q(GData);
    QByteArray data;
    QUrl authGateway(QStringLiteral("https://www.google.com/accounts/ClientLogin"));
    authGateway.addQueryItem(QStringLiteral("Email"), q->username());
    authGateway.addQueryItem(QStringLiteral("Passwd"), q->password());
    authGateway.addQueryItem(QStringLiteral("source"), q->userAgent());
    authGateway.addQueryItem(QStringLiteral("service"), QStringLiteral("blogger"));
    if (!mAuthenticationTime.isValid() ||
            QDateTime::currentDateTime().toTime_t() - mAuthenticationTime.toTime_t() > TIMEOUT ||
            mAuthenticationString.isEmpty()) {
        KIO::Job *job = KIO::http_post(authGateway, QByteArray(), KIO::HideProgressInfo);
        if (KIO::NetAccess::synchronousRun(job, (QWidget *)0, &data, &authGateway)) {
            QRegExp rx(QStringLiteral("Auth=(.+)"));
            if (rx.indexIn(QLatin1String(data)) != -1) {
                qDebug() << "RegExp got authentication string:" << rx.cap(1);
                mAuthenticationString = rx.cap(1);
                mAuthenticationTime = QDateTime::currentDateTime();
                return true;
            }
        }
        return false;
    }
    return true;
}

void GDataPrivate::slotFetchProfileId(KJob *job)
{
    qDebug();
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    Q_Q(GData);
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data(), stj->data().size());
    if (!job->error()) {
        QRegExp pid(QStringLiteral("http://www.blogger.com/profile/(\\d+)"));
        if (pid.indexIn(data) != -1) {
            q->setProfileId(pid.cap(1));
            qDebug() << "QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' matches" << pid.cap(1);
            emit q->fetchedProfileId(pid.cap(1));
        } else {
            qCritical() << "QRegExp bid( 'http://www.blogger.com/profile/(\\d+)' "
                        << " could not regexp the Profile ID";
            emit q->error(GData::Other, i18n("Could not regexp the Profile ID."));
            emit q->fetchedProfileId(QString());
        }
    } else {
        qCritical() << "Job Error: " << job->errorString();
        emit q->error(GData::Other, job->errorString());
        emit q->fetchedProfileId(QString());
    }
}

void GDataPrivate::slotListBlogs(Syndication::Loader *loader,
                                 Syndication::FeedPtr feed,
                                 Syndication::ErrorCode status)
{
    qDebug();
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }
    if (status != Syndication::Success) {
        emit q->error(GData::Atom, i18n("Could not get blogs."));
        return;
    }

    QList<QMap<QString, QString> > blogsList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        QRegExp rx(QStringLiteral("blog-(\\d+)"));
        QMap<QString, QString> blogInfo;
        if (rx.indexIn((*it)->id()) != -1) {
            qDebug() << "QRegExp rx( 'blog-(\\d+)' matches" << rx.cap(1);
            blogInfo[QStringLiteral("id")] = rx.cap(1);
            blogInfo[QStringLiteral("title")] = (*it)->title();
            blogInfo[QStringLiteral("url")] = (*it)->link();
            blogInfo[QStringLiteral("summary")] = (*it)->description();   //TODO fix/add more
            blogsList << blogInfo;
        } else {
            qCritical() << "QRegExp rx( 'blog-(\\d+)' does not match anything in:"
                        << (*it)->id();
            emit q->error(GData::Other, i18n("Could not regexp the blog id path."));
        }
    }
    qDebug() << "Emitting listedBlogs(); ";
    emit q->listedBlogs(blogsList);
}

void GDataPrivate::slotListComments(Syndication::Loader *loader,
                                    Syndication::FeedPtr feed,
                                    Syndication::ErrorCode status)
{
    qDebug();
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }
    BlogPost *post = mListCommentsMap[ loader ];
    mListCommentsMap.remove(loader);

    if (status != Syndication::Success) {
        emit q->errorPost(GData::Atom, i18n("Could not get comments."), post);
        return;
    }

    QList<KBlog::BlogComment> commentList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        BlogComment comment;
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) == -1) {
            qCritical() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
            emit q->error(GData::Other, i18n("Could not regexp the comment id path."));
        } else {
            comment.setCommentId(rx.cap(1));
        }
        qDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
        comment.setTitle((*it)->title());
        comment.setContent((*it)->content());
//  FIXME: assuming UTC for now
        comment.setCreationDateTime(
            KDateTime(QDateTime::fromTime_t((*it)->datePublished()),
                      KDateTime::Spec::UTC()));
        comment.setModificationDateTime(
            KDateTime(QDateTime::fromTime_t((*it)->dateUpdated()),
                      KDateTime::Spec::UTC()));
        commentList.append(comment);
    }
    qDebug() << "Emitting listedComments()";
    emit q->listedComments(post, commentList);
}

void GDataPrivate::slotListAllComments(Syndication::Loader *loader,
                                       Syndication::FeedPtr feed,
                                       Syndication::ErrorCode status)
{
    qDebug();
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }

    if (status != Syndication::Success) {
        emit q->error(GData::Atom, i18n("Could not get comments."));
        return;
    }

    QList<KBlog::BlogComment> commentList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        BlogComment comment;
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) == -1) {
            qCritical() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
            emit q->error(GData::Other, i18n("Could not regexp the comment id path."));
        } else {
            comment.setCommentId(rx.cap(1));
        }

        qDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
        comment.setTitle((*it)->title());
        comment.setContent((*it)->content());
//  FIXME: assuming UTC for now
        comment.setCreationDateTime(
            KDateTime(QDateTime::fromTime_t((*it)->datePublished()),
                      KDateTime::Spec::UTC()));
        comment.setModificationDateTime(
            KDateTime(QDateTime::fromTime_t((*it)->dateUpdated()),
                      KDateTime::Spec::UTC()));
        commentList.append(comment);
    }
    qDebug() << "Emitting listedAllComments()";
    emit q->listedAllComments(commentList);
}

void GDataPrivate::slotListRecentPosts(Syndication::Loader *loader,
                                       Syndication::FeedPtr feed,
                                       Syndication::ErrorCode status)
{
    qDebug();
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }

    if (status != Syndication::Success) {
        emit q->error(GData::Atom, i18n("Could not get posts."));
        return;
    }
    int number = 0;

    if (mListRecentPostsMap.contains(loader)) {
        number = mListRecentPostsMap[ loader ];
    }
    mListRecentPostsMap.remove(loader);

    QList<KBlog::BlogPost> postList;

    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        BlogPost post;
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) == -1) {
            qCritical() << "QRegExp rx( 'post-(\\d+)' does not match" << rx.cap(1);
            emit q->error(GData::Other, i18n("Could not regexp the post id path."));
        } else {
            post.setPostId(rx.cap(1));
        }

        qDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
        post.setTitle((*it)->title());
        post.setContent((*it)->content());
        post.setLink(QUrl((*it)->link()));
        QStringList labels;
        int catCount = (*it)->categories().count();
        QList< Syndication::CategoryPtr > cats = (*it)->categories();
        for (int i = 0; i < catCount; ++i) {
            if (cats[i].get()->label().isEmpty()) {
                labels.append(cats[i].get()->term());
            } else {
                labels.append(cats[i].get()->label());
            }
        }
        post.setTags(labels);
//  FIXME: assuming UTC for now
        post.setCreationDateTime(
            KDateTime(QDateTime::fromTime_t((*it)->datePublished()),
                      KDateTime::Spec::UTC()).toLocalZone());
        post.setModificationDateTime(
            KDateTime(QDateTime::fromTime_t((*it)->dateUpdated()),
                      KDateTime::Spec::UTC()).toLocalZone());
        post.setStatus(BlogPost::Fetched);
        postList.append(post);
        if (number-- == 0) {
            break;
        }
    }
    qDebug() << "Emitting listedRecentPosts()";
    emit q->listedRecentPosts(postList);
}

void GDataPrivate::slotFetchPost(Syndication::Loader *loader,
                                 Syndication::FeedPtr feed,
                                 Syndication::ErrorCode status)
{
    qDebug();
    Q_Q(GData);
    if (!loader) {
        qCritical() << "loader is a null pointer.";
        return;
    }

    bool success = false;

    BlogPost *post = mFetchPostMap.take(loader);
    qCritical() << "Post" << post;
    post->postId();

    if (status != Syndication::Success) {
        emit q->errorPost(GData::Atom, i18n("Could not get posts."), post);
        return;
    }

    QString postId = post->postId();
    QList<Syndication::ItemPtr> items = feed->items();
    QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
    QList<Syndication::ItemPtr>::ConstIterator end = items.constEnd();
    for (; it != end; ++it) {
        QRegExp rx(QStringLiteral("post-(\\d+)"));
        if (rx.indexIn((*it)->id()) != -1 &&
                rx.cap(1) == postId) {
            qDebug() << "QRegExp rx( 'post-(\\d+)' matches" << rx.cap(1);
            post->setPostId(rx.cap(1));
            post->setTitle((*it)->title());
            post->setContent((*it)->content());
            post->setStatus(BlogPost::Fetched);
            post->setLink(QUrl((*it)->link()));
            post->setCreationDateTime(
                KDateTime(QDateTime::fromTime_t((*it)->datePublished()),
                          KDateTime::Spec::UTC()).toLocalZone());
            post->setModificationDateTime(
                KDateTime(QDateTime::fromTime_t((*it)->dateUpdated()),
                          KDateTime::Spec::UTC()).toLocalZone());
            qDebug() << "Emitting fetchedPost( postId=" << postId << ");";
            success = true;
            emit q->fetchedPost(post);
            break;
        }
    }
    if (!success) {
        qCritical() << "QRegExp rx( 'post-(\\d+)' does not match"
                    << mFetchPostMap[ loader ]->postId() << ".";
        emit q->errorPost(GData::Other, i18n("Could not regexp the blog id path."), post);
    }
}

void GDataPrivate::slotCreatePost(KJob *job)
{
    qDebug();
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data(), stj->data().size());

    Q_Q(GData);

    KBlog::BlogPost *post = mCreatePostMap[ job ];
    mCreatePostMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotCreatePost error:" << job->errorString();
        emit q->errorPost(GData::Atom, job->errorString(), post);
        return;
    }

    QRegExp rxId(QStringLiteral("post-(\\d+)"));   //FIXME check and do better handling, esp the creation date time
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

    QRegExp rxPub(QStringLiteral("<published>(.+)</published>"));
    if (rxPub.indexIn(data) == -1) {
        qCritical() << "Could not regexp the published time out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the published time out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

    QRegExp rxUp(QStringLiteral("<updated>(.+)</updated>"));
    if (rxUp.indexIn(data) == -1) {
        qCritical() << "Could not regexp the update time out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the update time out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);

    post->setPostId(rxId.cap(1));
    post->setCreationDateTime(KDateTime().fromString(rxPub.cap(1)).toLocalZone());
    post->setModificationDateTime(KDateTime().fromString(rxUp.cap(1)));
    post->setStatus(BlogPost::Created);
    qDebug() << "Emitting createdPost()";
    emit q->createdPost(post);
}

void GDataPrivate::slotModifyPost(KJob *job)
{
    qDebug();
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data(), stj->data().size());

    KBlog::BlogPost *post = mModifyPostMap[ job ];
    mModifyPostMap.remove(job);
    Q_Q(GData);
    if (job->error() != 0) {
        qCritical() << "slotModifyPost error:" << job->errorString();
        emit q->errorPost(GData::Atom, job->errorString(), post);
        return;
    }

    QRegExp rxId(QStringLiteral("post-(\\d+)"));   //FIXME check and do better handling, esp creation date time
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

    QRegExp rxPub(QStringLiteral("<published>(.+)</published>"));
    if (rxPub.indexIn(data) == -1) {
        qCritical() << "Could not regexp the published time out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the published time out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

    QRegExp rxUp(QStringLiteral("<updated>(.+)</updated>"));
    if (rxUp.indexIn(data) == -1) {
        qCritical() << "Could not regexp the update time out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the update time out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
    post->setPostId(rxId.cap(1));
    post->setCreationDateTime(KDateTime().fromString(rxPub.cap(1)));
    post->setModificationDateTime(KDateTime().fromString(rxUp.cap(1)));
    post->setStatus(BlogPost::Modified);
    emit q->modifiedPost(post);
}

void GDataPrivate::slotRemovePost(KJob *job)
{
    qDebug();
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data(), stj->data().size());

    KBlog::BlogPost *post = mRemovePostMap[ job ];
    mRemovePostMap.remove(job);
    Q_Q(GData);
    if (job->error() != 0) {
        qCritical() << "slotRemovePost error:" << job->errorString();
        emit q->errorPost(GData::Atom, job->errorString(), post);
        return;
    }

    post->setStatus(BlogPost::Removed);
    qDebug() << "Emitting removedPost()";
    emit q->removedPost(post);
}

void GDataPrivate::slotCreateComment(KJob *job)
{
    qDebug();
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data(), stj->data().size());
    qDebug() << "Dump data: " << data;

    Q_Q(GData);

    KBlog::BlogComment *comment = mCreateCommentMap[ job ].values().first();
    KBlog::BlogPost *post = mCreateCommentMap[ job ].keys().first();
    mCreateCommentMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotCreateComment error:" << job->errorString();
        emit q->errorComment(GData::Atom, job->errorString(), post, comment);
        return;
    }

// TODO check for result and fit appropriately
    QRegExp rxId(QStringLiteral("post-(\\d+)"));
    if (rxId.indexIn(data) == -1) {
        qCritical() << "Could not regexp the id out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the id out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( 'post-(\\d+)' ) matches" << rxId.cap(1);

    QRegExp rxPub(QStringLiteral("<published>(.+)</published>"));
    if (rxPub.indexIn(data) == -1) {
        qCritical() << "Could not regexp the published time out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the published time out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( '<published>(.+)</published>' ) matches" << rxPub.cap(1);

    QRegExp rxUp(QStringLiteral("<updated>(.+)</updated>"));
    if (rxUp.indexIn(data) == -1) {
        qCritical() << "Could not regexp the update time out of the result:" << data;
        emit q->errorPost(GData::Atom,
                          i18n("Could not regexp the update time out of the result."), post);
        return;
    }
    qDebug() << "QRegExp rx( '<updated>(.+)</updated>' ) matches" << rxUp.cap(1);
    comment->setCommentId(rxId.cap(1));
    comment->setCreationDateTime(KDateTime().fromString(rxPub.cap(1)));
    comment->setModificationDateTime(KDateTime().fromString(rxUp.cap(1)));
    comment->setStatus(BlogComment::Created);
    qDebug() << "Emitting createdComment()";
    emit q->createdComment(post, comment);
}

void GDataPrivate::slotRemoveComment(KJob *job)
{
    qDebug();
    if (!job) {
        qCritical() << "job is a null pointer.";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);
    const QString data = QString::fromUtf8(stj->data(), stj->data().size());

    Q_Q(GData);

    KBlog::BlogComment *comment = mRemoveCommentMap[ job ].values().first();
    KBlog::BlogPost *post = mRemoveCommentMap[ job ].keys().first();
    mRemoveCommentMap.remove(job);

    if (job->error() != 0) {
        qCritical() << "slotRemoveComment error:" << job->errorString();
        emit q->errorComment(GData::Atom, job->errorString(), post, comment);
        return;
    }

    comment->setStatus(BlogComment::Created);
    qDebug() << "Emitting removedComment()";
    emit q->removedComment(post, comment);
}

#include "moc_gdata.cpp"
