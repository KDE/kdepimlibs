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

#ifndef KMBOX_MBOX_P_H
#define KMBOX_MBOX_P_H

#include "mbox.h"

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTimer>

namespace KMBox
{

class MBoxPrivate : public QObject
{
    Q_OBJECT

public:
    MBoxPrivate(MBox *mbox);

    virtual ~MBoxPrivate();

    void close();

    void initLoad(const QString &fileName);

    bool open();

    bool startTimerIfNeeded();

    bool isMBoxSeparator(const QByteArray &line) const;

public Q_SLOTS:
    void unlockMBox();

public:
    QByteArray      mAppendedEntries;
    MBoxEntry::List mEntries;
    bool            mFileLocked;
    quint64         mInitialMboxFileSize;
    QString         mLockFileName;
    MBox::LockType  mLockType;
    MBox            *mMBox;
    QFile           mMboxFile;
    bool            mReadOnly;
    QTimer          mUnlockTimer;
    QRegExp         mSeparatorMatcher;

public: /// Static helper methods
    static QByteArray escapeFrom(const QByteArray &msg);

    /**
     * Generates a mbox message sperator line for given message.
     */
    static QByteArray mboxMessageSeparator(const QByteArray &msg);

    /**
     * Unescapes the raw message read from the file.
     */
    static void unescapeFrom(char *msg, size_t size);
};

}

#endif // KMBOX_MBOX_P_H
