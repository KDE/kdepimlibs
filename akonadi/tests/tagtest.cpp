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

#include <QObject>

#include "test_utils.h"

#include <akonadi/control.h>
#include <akonadi/tagcreatejob.h>
#include <akonadi/tagfetchjob.h>
#include <akonadi/tagdeletejob.h>
#include <akonadi/qtest_akonadi.h>

using namespace Akonadi;

class TagTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testCreateFetch();
};

void TagTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();
    AkonadiTest::setAllResourcesOffline();
}

void TagTest::testCreateFetch()
{
    Tag tag;
    tag.setGid("gid");
    TagCreateJob *createjob = new TagCreateJob(tag, this);
    AKVERIFYEXEC(createjob);
    QVERIFY(createjob->tag().isValid());

    {
        TagFetchJob *fetchJob = new TagFetchJob(this);
        AKVERIFYEXEC(fetchJob);
        QCOMPARE(fetchJob->tags().size(), 1);
        QCOMPARE(fetchJob->tags().first().gid(), QByteArray("gid"));
        kDebug() << fetchJob->tags().first().id();

        TagDeleteJob *deleteJob = new TagDeleteJob(fetchJob->tags().first(), this);
        AKVERIFYEXEC(deleteJob);
    }

    {
        TagFetchJob *fetchJob = new TagFetchJob(this);
        AKVERIFYEXEC(fetchJob);
        QCOMPARE(fetchJob->tags().size(), 0);
    }
}

#include "tagtest.moc"

QTEST_AKONADIMAIN(TagTest, NoGUI)
