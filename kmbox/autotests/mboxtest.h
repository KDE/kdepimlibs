/*
  Copyright (c) 2009 Bertjan Broeksema <broeksema@kde.org>

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

#ifndef MBOXTEST_H
#define MBOXTEST_H

#include <QObject>
#include <QThread>

#include "mbox.h"

class QTemporaryDir;

class MboxTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testSetLockMethod();
    void testLockBeforeLoad();
    void testProcMailLock();
    void testConcurrentAccess();
    void testAppend();
    void testSaveAndLoad();
    void testBlankLines();
    void testLockTimeout();
    void cleanupTestCase();
    void testEntries();
    void testPurge();
    void testHeaders();

private:
    QString fileName();
    QString lockFileName();
    void removeTestFile();

private:
    QTemporaryDir *mTempDir;
    KMime::Message::Ptr mMail1;
    KMime::Message::Ptr mMail2;
};

class ThreadFillsMBox : public QThread
{
    Q_OBJECT

public:
    ThreadFillsMBox(const QString &fileName);

protected:
    virtual void run();

private:
    KMBox::MBox *mbox;
};

#endif // MBOXTEST_H
