/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "abort.h"

#include <QTimer>

#include <QApplication>
#include <QDebug>
#include <KLocalizedString>

#include <control.h>

#include <dispatcherinterface.h>

using namespace Akonadi;
using namespace MailTransport;

Runner::Runner()
{
    Control::start();

    QTimer::singleShot(0, this, SLOT(sendAbort()));
}

void Runner::sendAbort()
{
    const AgentInstance mda = DispatcherInterface().dispatcherInstance();
    if (!mda.isValid()) {
        qDebug() << "Invalid instance; waiting.";
        QTimer::singleShot(1000, this, SLOT(sendAbort()));
        return;
    }

    mda.abortCurrentTask();
    qDebug() << "Told the MDA to abort.";
    QApplication::exit(0);
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QLatin1String("Abort"));
    QApplication app(argc, argv);

    new Runner();
    return app.exec();
}

