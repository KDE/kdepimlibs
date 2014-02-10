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
#include <tagattribute.h>
#include <akonadi/qtest_akonadi.h>
#include <akonadi/item.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

using namespace Akonadi;

class TagTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testCreateFetch();
    void testAttributes();
    void testTagItem();
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
void TagTest::testAttributes()
{
    Tag tag;
    tag.setGid("gid2");
    TagAttribute *attr = tag.attribute<TagAttribute>(AttributeEntity::AddIfMissing);
    attr->setDisplayName("name");
    attr->setInToolbar(true);
    tag.addAttribute(attr);
    TagCreateJob *createjob = new TagCreateJob(tag, this);
    AKVERIFYEXEC(createjob);
    QVERIFY(createjob->tag().isValid());

    {
        TagFetchJob *fetchJob = new TagFetchJob(createjob->tag(), this);
        fetchJob->fetchAttribute<TagAttribute>();
        AKVERIFYEXEC(fetchJob);
        QCOMPARE(fetchJob->tags().size(), 1);
        QVERIFY(fetchJob->tags().first().hasAttribute<TagAttribute>());
        //we need to clone because the returned attribute is just a reference and destroyed on the next line
        //FIXME we should find a better solution for this (like returning a smart pointer or value object)
        QScopedPointer<TagAttribute> tagAttr(fetchJob->tags().first().attribute<TagAttribute>()->clone());
        QVERIFY(tagAttr);
        QCOMPARE(tagAttr->displayName(), QLatin1String("name"));
        QCOMPARE(tagAttr->inToolbar(), true);

        TagDeleteJob *deleteJob = new TagDeleteJob(fetchJob->tags().first(), this);
        AKVERIFYEXEC(deleteJob);
    }
}

void TagTest::testTagItem()
{
    const Collection res3 = Collection( collectionIdFromPath( "res3" ) );
    Tag tag;
    {
        TagCreateJob *createjob = new TagCreateJob(Tag("gid1"), this);
        AKVERIFYEXEC(createjob);
        tag = createjob->tag();
    }

    Item item1;
    {
        item1.setMimeType( "application/octet-stream" );
        ItemCreateJob *append = new ItemCreateJob(item1, res3, this);
        AKVERIFYEXEC(append);
        item1 = append->item();
    }

    item1.setTag(tag);

    ItemModifyJob *modJob = new ItemModifyJob(item1, this);
    AKVERIFYEXEC(modJob);

    ItemFetchJob *fetchJob = new ItemFetchJob(item1, this);
    fetchJob->fetchScope().setFetchTags(true);
    AKVERIFYEXEC(fetchJob);
    QCOMPARE(fetchJob->items().first().tags().size(), 1);
}


#include "tagtest.moc"

QTEST_AKONADIMAIN(TagTest, NoGUI)
