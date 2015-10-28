/*
 * Copyright 2015  Daniel Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CONNECTIONTHREAD_P_H
#define CONNECTIONTHREAD_P_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtNetwork/QLocalSocket>

#include <akonadi/private/protocol_p.h>

class QAbstractSocket;
class QFile;

namespace Akonadi
{

class ConnectionThread : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionThread(const QByteArray &sessionId, QObject *parent = Q_NULLPTR);
    ~ConnectionThread();

    Q_INVOKABLE void reconnect();
    void forceReconnect();
    void disconnect();
    void sendCommand(qint64 tag, const Protocol::Command &command);

Q_SIGNALS:
    void connected();
    void reconnected();
    void commandReceived(qint64 tag, const Akonadi::Protocol::Command &command);
    void socketDisconnected();
    void socketError(const QString &message);

private Q_SLOTS:
    void doThreadQuit();
    void doReconnect();
    void doForceReconnect();
    void doDisconnect();
    void doSendCommand(qint64 tag, const Akonadi::Protocol::Command &command);

    void dataReceived();

private:

    bool handleCommand(qint64 tag, const Protocol::Command &cmd);

    QLocalSocket *mSocket;
    QFile *mLogFile;
    QByteArray mSessionId;
    QMutex mLock;
    struct Command {
        qint64 tag;
        Protocol::Command cmd;
    };
    QQueue<Command> mOutQueue;
};

}

#endif // CONNECTIONTHREAD_P_H
