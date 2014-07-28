/* This file is based on qtest_kde.h from kdelibs
    Copyright (C) 2006 David Faure <faure@kde.org>
    Copyright (C) 2009 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QTEST_AKONADI_H
#define QTEST_AKONADI_H

#include <qtest_kde.h>

#include <agentinstance.h>
#include <agentmanager.h>
#include <KLocalizedString>

/**
* \short Akonadi Replacement for QTEST_MAIN from QTestLib
*
* This macro should be used for classes that run inside the Akonadi Testrunner.
* So instead of writing QTEST_MAIN( TestClass ) you write
* QTEST_AKONADIMAIN( TestClass ).
*
* \param TestObject The class you use for testing.
*
* \see QTestLib
* \see QTEST_KDEMAIN
*/
#define QTEST_AKONADIMAIN(TestObject) \
int main(int argc, char *argv[]) \
{ \
  setenv( "LC_ALL", "C", 1); \
  unsetenv( "KDE_COLOR_DEBUG" ); \
  KAboutData aboutData( QLatin1String( "qttest" ), i18n( "KDE Test Program" ), QLatin1String( "version" ) );  \
  QApplication app( argc, argv ); \
  KAboutData::setApplicationData(aboutData); \
  qRegisterMetaType<KUrl>(); /*as done by kapplication*/ \
  qRegisterMetaType<KUrl::List>(); \
  TestObject tc; \
  KGlobal::ref(); /* don't quit qeventloop after closing a mainwindow */ \
  return QTest::qExec( &tc, argc, argv ); \
}

namespace AkonadiTest {
/**
 * Checks that the test is running in the proper test environment
 */
void checkTestIsIsolated() {
    Q_ASSERT_X(!qgetenv("TESTRUNNER_DB_ENVIRONMENT").isEmpty(),
               "AkonadiTest::checkTestIsIsolated",
               "This test must be run using ctest, in order to use the testrunner environment. Aborting, to avoid messing up your real akonadi");
}

/**
 * Switch all resources offline to reduce interference from them
 */
void setAllResourcesOffline() {
    // switch all resources offline to reduce interference from them
    foreach (Akonadi::AgentInstance agent, Akonadi::AgentManager::self()->instances()) {    //krazy:exclude=foreach
        agent.setIsOnline(false);
    }
}

} // namespace

/**
 * Runs an Akonadi::Job synchronously and aborts if the job failed.
 * Similar to QVERIFY( job->exec() ) but includes the job error message
 * in the output in case of a failure.
 */
#define AKVERIFYEXEC( job ) \
  QVERIFY2( job->exec(), job->errorString().toUtf8().constData() )

#endif
