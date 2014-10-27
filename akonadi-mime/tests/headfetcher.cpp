/*
    Copyright (c) 2007 Robert Zwerus <arzie@dds.nl>

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

#include "headfetcher.h"

#include <collectionfetchjob.h>
#include <itemfetchjob.h>
#include <itemfetchscope.h>

#include "messageparts.h"

#include <QtCore/QDebug>
#include <QApplication>
#include <QTimer>

#include <kmime/kmime_message.h>

#include <boost/shared_ptr.hpp>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

using namespace Akonadi;

HeadFetcher::HeadFetcher(bool multipart)
{
    // fetch all headers from each folder
    timer.start();
    qDebug() << "Listing all headers of every folder, using" << (multipart ? "multi" : "single") << "part.";
    CollectionFetchJob *clj = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive);
    clj->exec();
    Collection::List list = clj->collections();
    foreach (const Collection &collection, list) {
        ItemFetchJob *ifj = new ItemFetchJob(collection, this);
        if (multipart) {
            ifj->fetchScope().fetchPayloadPart(MessagePart::Envelope);
        } else {
            ifj->fetchScope().fetchFullPayload();
        }
        ifj->exec();
        qDebug() << "  Listing" << ifj->items().count() << "item headers.";
        foreach (const Item &item, ifj->items()) {
            qDebug() << item.payload< boost::shared_ptr<KMime::Message> >()->subject()->asUnicodeString();
        }
    }

    qDebug() << "Took:" << timer.elapsed() << "ms.";
    QTimer::singleShot(1000, this, SLOT(stop()));
}

void HeadFetcher::stop()
{
    qApp->quit();
}

int main(int argc, char *argv[])
{
    KAboutData aboutData(QLatin1String("headfetcher"), i18n("Headfetcher"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("header fetching application"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("multipart"), i18n("Run test on multipart data (default is singlepart).")));

    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    bool multipart = parser.isSet(QLatin1String("multipart"));

    HeadFetcher d(multipart);

    return app.exec();
}
