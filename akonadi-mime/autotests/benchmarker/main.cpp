/*
    Copyright (c) 2009 Igor Trindade Oliveira <igor_trindade@yahoo.com.br>
    based on kdepimlibs/akonadi/tests/benchmarker.cpp wrote by Robert Zwerus <arzie@dds.nl>

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

#include "testmaildir.h"
#include "testvcard.h"


#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
  KAboutData aboutData( QLatin1String("benchmarker"), i18n("Benchmarker") , QLatin1String("1.0" ));
  aboutData.setShortDescription(i18n("benchmark application") );
  QApplication app(argc, argv);
  QCommandLineParser parser;
  KAboutData::setApplicationData(aboutData);
  parser.addVersionOption();
  parser.addHelpOption();
  parser.addOption(QCommandLineOption(QStringList() << QLatin1String("maildir"), i18n("Path to maildir to be used as data source"), QLatin1String("argument")));
  parser.addOption(QCommandLineOption(QStringList() << QLatin1String("vcarddir"), i18n("Path to vvcarddir to be used as data source"), QLatin1String("argument")));

  aboutData.setupCommandLine(&parser);
  parser.process(app);
  aboutData.processCommandLine(&parser);

  const QString maildir = parser.value( QLatin1String("maildir") );
  const QString vcarddir = parser.value( QLatin1String("vcarddir") );

  TestMailDir *mailDirTest = new TestMailDir(maildir);
  TestVCard *vcardTest = new TestVCard(vcarddir);

  mailDirTest->runTests();
  vcardTest->runTests();

  return app.exec();
}
