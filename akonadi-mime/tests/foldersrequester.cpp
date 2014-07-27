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

#include "foldersrequester.h"

#include <QDebug>
#include <KLocalizedString>
#include <KAboutData>
#include <QApplication>
#include <QCommandLineParser>
#include <collection.h>
#include <control.h>
#include <specialmailcollections.h>
#include <specialmailcollectionsrequestjob.h>

using namespace Akonadi;

Requester::Requester()
{
    Control::start();

    SpecialMailCollectionsRequestJob *rjob = new SpecialMailCollectionsRequestJob(this);
    rjob->requestDefaultCollection(SpecialMailCollections::Outbox);
    connect(rjob, SIGNAL(result(KJob*)), this, SLOT(requestResult(KJob*)));
    rjob->start();
}

void Requester::requestResult(KJob *job)
{
    if (job->error()) {
        qCritical() << "LocalFoldersRequestJob failed:" << job->errorString();
        QApplication::exit(1);
    } else {
        // Success.
        QApplication::exit(2);
    }
}

int main(int argc, char **argv)
{
    QCommandLineParser parser;

    KAboutData about(QStringLiteral("foldersrequester"),
                     i18n("An app that requests LocalFolders"), QStringLiteral("0.1"));

    about.setupCommandLine(&parser);
    KAboutData::setApplicationData(about);

    QApplication app(argc, argv);
    parser.process(app);
    about.processCommandLine(&parser);

    new Requester();
    return app.exec();
}
