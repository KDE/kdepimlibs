/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "qskypedialer.h"

#include <KDBusConnectionPool>

#include <QtCore/QProcess>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <KLocalizedString>

#include <unistd.h>

static bool isSkypeServiceRegistered()
{
    const QString service(QStringLiteral("com.Skype.API"));

    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    if (interface->isServiceRegistered(service)) {
        return true;
    }

    interface = KDBusConnectionPool::threadConnection().interface();
    if (interface->isServiceRegistered(service)) {
        return true;
    }

    return false;
}

static QDBusInterface *searchSkypeDBusInterface()
{
    const QString service(QStringLiteral("com.Skype.API"));
    const QString path(QStringLiteral("/com/Skype"));

    QDBusInterface *interface = new QDBusInterface(service, path, QString(), QDBusConnection::sessionBus());
    if (!interface->isValid()) {
        delete interface;
        interface = new QDBusInterface(service, path, QString(), KDBusConnectionPool::threadConnection());
    }

    return interface;
}

QSkypeDialer::QSkypeDialer(const QString &applicationName)
    : QDialer(applicationName), mInterface(0)
{
}

QSkypeDialer::~QSkypeDialer()
{
    delete mInterface;
}

bool QSkypeDialer::initializeSkype()
{
    if (mInterface && mInterface->isValid()) {
        return true;
    }

    // first check whether dbus interface is available yet
    if (!isSkypeServiceRegistered()) {

        // it could be skype is not running yet, so start it now
        if (!QProcess::startDetached(QStringLiteral("skype"), QStringList())) {
            mErrorMessage = i18n("Unable to start skype process, check that skype executable is in your PATH variable.");
            return false;
        }

        const int runs = 100;
        for (int i = 0; i < runs; ++i) {
            if (!isSkypeServiceRegistered()) {
                ::sleep(2);
            } else {
                break;
            }
        }
    }

    // check again for the dbus interface
    mInterface = searchSkypeDBusInterface();

    if (!mInterface->isValid()) {
        delete mInterface;
        mInterface = 0;

        mErrorMessage = i18n("Skype Public API (D-Bus) seems to be disabled.");
        return false;
    }

    QDBusReply<QString> reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("NAME %1").arg(mApplicationName));
    if (reply.value() != QLatin1String("OK")) {
        delete mInterface;
        mInterface = 0;

        mErrorMessage = i18n("Skype registration failed.");
        return false;
    }

    reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("PROTOCOL 1"));
    if (reply.value() != QLatin1String("PROTOCOL 1")) {
        delete mInterface;
        mInterface = 0;

        mErrorMessage = i18n("Protocol mismatch.");
        return false;
    }

    return true;
}

bool QSkypeDialer::dialNumber(const QString &number)
{
    if (!initializeSkype()) {
        return false;
    }

    QDBusReply<QString> reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("CALL %1").arg(number));

    return true;
}

bool QSkypeDialer::sendSms(const QString &number, const QString &text)
{
    if (!initializeSkype()) {
        return false;
    }

    // First we create a new SMS object that gets an ID. We need that ID later...
    QDBusReply<QString> reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("CREATE SMS OUTGOING %1").arg(number));
    const QString messageId = reply.value().section(QStringLiteral(" "), 1, 1);

    // Set the SMS text
    reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("SET SMS %1 BODY %2").arg(messageId, text));

    // Send the SMS
    reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("ALTER SMS %1 SEND").arg(messageId));
    if (reply.value().contains(QStringLiteral("ERROR"))) {
        mErrorMessage = reply.value();
        // As sending the message failed (not enough Skype credit), lets delete the message
        reply = mInterface->call(QStringLiteral("Invoke"), QStringLiteral("DELETE SMS %1").arg(messageId));
        return false;
    }

    return true;
}
