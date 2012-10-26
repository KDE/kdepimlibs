/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

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

#include "selftestdialog_p.h"
#include "agentmanager.h"
#include "dbusconnectionpool.h"
#include "session_p.h"
#include "servermanager.h"
#include "servermanager_p.h"

#include <akonadi/private/xdgbasedirs_p.h>

#include <KDebug>
#include <KIcon>
#include <KFileDialog>
#include <KLocale>
#include <KMessageBox>
#include <KRun>
#include <KStandardDirs>
#include <KUser>

#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtDBus/QtDBus>
#include <QApplication>
#include <QClipboard>
#include <QStandardItemModel>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

// @cond PRIVATE

#define AKONADI_SEARCH_SERVICE QLatin1String( "org.kde.nepomuk.services.nepomukqueryservice" )

using namespace Akonadi;

static QString makeLink( const QString &file )
{
  return QString::fromLatin1( "<a href=\"%1\">%2</a>" ).arg( file, file );
}

enum SelfTestRole {
  ResultTypeRole = Qt::UserRole,
  FileIncludeRole,
  ListDirectoryRole,
  EnvVarRole,
  SummaryRole,
  DetailsRole
};

SelfTestDialog::SelfTestDialog(QWidget * parent) :
    KDialog( parent )
{
  setCaption( i18n( "Akonadi Server Self-Test" ) );
  setButtons( Close | User1 | User2 );
  setButtonText( User1, i18n( "Save Report..." ) );
  setButtonIcon( User1, KIcon( QString::fromLatin1( "document-save" ) ) );
  setButtonText( User2, i18n( "Copy Report to Clipboard" ) );
  setButtonIcon( User2, KIcon( QString::fromLatin1( "edit-copy" ) ) );
  showButtonSeparator( true );
  ui.setupUi( mainWidget() );

  mTestModel = new QStandardItemModel( this );
  ui.testView->setModel( mTestModel );
  connect( ui.testView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
           SLOT(selectionChanged(QModelIndex)) );
  connect( ui.detailsLabel, SIGNAL(linkActivated(QString)), SLOT(linkActivated(QString)) );

  connect( this, SIGNAL(user1Clicked()), SLOT(saveReport()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(copyReport()) );

  connect( ServerManager::self(), SIGNAL(stateChanged(Akonadi::ServerManager::State)), SLOT(runTests()) );
  runTests();
}

void SelfTestDialog::hideIntroduction()
{
  ui.introductionLabel->hide();
}

QStandardItem* SelfTestDialog::report( ResultType type, const KLocalizedString & summary, const KLocalizedString & details)
{
  QStandardItem *item = new QStandardItem( summary.toString() );
  switch ( type ) {
    case Skip:
      item->setIcon( KIcon( QString::fromLatin1( "dialog-ok" ) ) );
      break;
    case Success:
      item->setIcon( KIcon( QString::fromLatin1( "dialog-ok-apply" ) ) );
      break;
    case Warning:
      item->setIcon( KIcon( QString::fromLatin1( "dialog-warning" ) ) );
      break;
    case Error:
    default:
      item->setIcon( KIcon( QString::fromLatin1( "dialog-error" ) ) );
  }
  item->setEditable( false );
  item->setWhatsThis( details.toString() );
  item->setData( type, ResultTypeRole );
  item->setData( summary.toString( 0 ), SummaryRole );
  item->setData( details.toString( 0 ), DetailsRole );
  mTestModel->appendRow( item );
  return item;
}

void SelfTestDialog::selectionChanged(const QModelIndex &index )
{
  if ( index.isValid() ) {
    ui.detailsLabel->setText( index.data( Qt::WhatsThisRole ).toString() );
    ui.detailsGroup->setEnabled( true );
  } else {
    ui.detailsLabel->setText( QString() );
    ui.detailsGroup->setEnabled( false );
  }
}

void SelfTestDialog::runTests()
{
  mTestModel->clear();

  const QString driver = serverSetting( QLatin1String( "General" ), "Driver", QLatin1String( "QMYSQL" ) ).toString();
  testSQLDriver();
  if (driver == QLatin1String( "QPSQL" )) {
    testPSQLServer();
  }
  else {
#ifndef Q_OS_WIN
    testRootUser();
#endif
    testMySQLServer();
    testMySQLServerLog();
    testMySQLServerConfig();
  }
  testAkonadiCtl();
  testServerStatus();
  testSearchStatus();
  testProtocolVersion();
  testResources();
  testServerLog();
  testControlLog();
}

QVariant SelfTestDialog::serverSetting(const QString & group, const char *key, const QVariant &def ) const
{
  const QString serverConfigFile = XdgBaseDirs::akonadiServerConfigFile( XdgBaseDirs::ReadWrite );
  QSettings settings( serverConfigFile, QSettings::IniFormat );
  settings.beginGroup( group );
  return settings.value( QString::fromLatin1(key), def );
}

bool SelfTestDialog::useStandaloneMysqlServer() const
{
  const QString driver = serverSetting( QLatin1String( "General" ), "Driver", QLatin1String( "QMYSQL" ) ).toString();
  if ( driver != QLatin1String( "QMYSQL" ) )
    return false;
  const bool startServer = serverSetting( driver, "StartServer", true ).toBool();
  if ( !startServer )
    return false;
  return true;
}

bool SelfTestDialog::runProcess(const QString & app, const QStringList & args, QString & result) const
{
  QProcess proc;
  proc.start( app, args );
  const bool rv = proc.waitForFinished();
  result.clear();
  result += QString::fromLocal8Bit( proc.readAllStandardError() );
  result += QString::fromLocal8Bit( proc.readAllStandardOutput() );
  return rv;
}

void SelfTestDialog::testSQLDriver()
{
  const QString driver = serverSetting( QLatin1String( "General" ), "Driver", QLatin1String( "QMYSQL" ) ).toString();
  const QStringList availableDrivers = QSqlDatabase::drivers();
  const KLocalizedString detailsOk = ki18n( "The QtSQL driver '%1' is required by your current Akonadi server configuration and was found on your system." )
      .subs( driver );
  const KLocalizedString detailsFail = ki18n( "The QtSQL driver '%1' is required by your current Akonadi server configuration.\n"
      "The following drivers are installed: %2.\n"
      "Make sure the required driver is installed." )
      .subs( driver )
      .subs( availableDrivers.join( QLatin1String( ", " ) ) );
  QStandardItem *item = 0;
  if ( availableDrivers.contains( driver ) )
    item = report( Success, ki18n( "Database driver found." ), detailsOk );
  else
    item = report( Error, ki18n( "Database driver not found." ), detailsFail );
  item->setData( XdgBaseDirs::akonadiServerConfigFile( XdgBaseDirs::ReadWrite ), FileIncludeRole );
}

void SelfTestDialog::testMySQLServer()
{
  if ( !useStandaloneMysqlServer() ) {
    report( Skip, ki18n( "MySQL server executable not tested." ),
            ki18n( "The current configuration does not require an internal MySQL server." ) );
    return;
  }

  const QString driver = serverSetting( QLatin1String( "General" ), "Driver", QLatin1String( "QMYSQL" ) ).toString();
  const QString serverPath = serverSetting( driver,  "ServerPath", QLatin1String( "" ) ).toString(); // ### default?

  const KLocalizedString details = ki18n( "You have currently configured Akonadi to use the MySQL server '%1'.\n"
      "Make sure you have the MySQL server installed, set the correct path and ensure you have the "
      "necessary read and execution rights on the server executable. The server executable is typically "
      "called 'mysqld'; its location varies depending on the distribution." ).subs( serverPath );

  QFileInfo info( serverPath );
  if ( !info.exists() )
    report( Error, ki18n( "MySQL server not found." ), details );
  else if ( !info.isReadable() )
    report( Error, ki18n( "MySQL server not readable." ), details );
  else if ( !info.isExecutable() )
    report( Error, ki18n( "MySQL server not executable." ), details );
  else if ( !serverPath.contains( QLatin1String( "mysqld" ) ) )
    report( Warning, ki18n( "MySQL found with unexpected name." ), details );
  else
    report( Success, ki18n( "MySQL server found." ), details );

  // be extra sure and get the server version while we are at it
  QString result;
  if ( runProcess( serverPath, QStringList() << QLatin1String( "--version" ), result ) ) {
    const KLocalizedString details = ki18n( "MySQL server found: %1" ).subs( result );
    report( Success, ki18n( "MySQL server is executable." ), details );
  } else {
    const KLocalizedString details = ki18n( "Executing the MySQL server '%1' failed with the following error message: '%2'" )
        .subs( serverPath ).subs( result );
    report( Error, ki18n( "Executing the MySQL server failed." ), details );
  }
}

void SelfTestDialog::testMySQLServerLog()
{
  if ( !useStandaloneMysqlServer() ) {
    report( Skip, ki18n( "MySQL server error log not tested." ),
            ki18n( "The current configuration does not require an internal MySQL server." ) );
    return;
  }

  const QString logFileName = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi/db_data" ) )
      + QDir::separator() + QString::fromLatin1( "mysql.err" );
  const QFileInfo logFileInfo( logFileName );
  if ( !logFileInfo.exists() || logFileInfo.size() == 0 ) {
    report( Success, ki18n( "No current MySQL error log found." ),
      ki18n( "The MySQL server did not report any errors during this startup. The log can be found in '%1'." ).subs( logFileName ) );
    return;
  }
  QFile logFile( logFileName );
  if ( !logFile.open( QFile::ReadOnly | QFile::Text  ) ) {
    report( Error, ki18n( "MySQL error log not readable." ),
      ki18n( "A MySQL server error log file was found but is not readable: %1" ).subs( makeLink( logFileName ) ) );
    return;
  }
  bool warningsFound = false;
  QStandardItem *item = 0;
  while ( !logFile.atEnd() ) {
    const QString line = QString::fromUtf8( logFile.readLine() );
    if ( line.contains( QLatin1String( "error" ), Qt::CaseInsensitive ) ) {
      item = report( Error, ki18n( "MySQL server log contains errors." ),
        ki18n( "The MySQL server error log file '%1' contains errors." ).subs( makeLink( logFileName ) ) );
      item->setData( logFileName, FileIncludeRole );
      return;
    }
    if ( !warningsFound && line.contains( QLatin1String( "warn" ), Qt::CaseInsensitive ) ) {
      warningsFound = true;
    }
  }
  if ( warningsFound ) {
    item = report( Warning, ki18n( "MySQL server log contains warnings." ),
                   ki18n( "The MySQL server log file '%1' contains warnings." ).subs( makeLink( logFileName ) ) );
  } else {
    item = report( Success, ki18n( "MySQL server log contains no errors." ),
                   ki18n( "The MySQL server log file '%1' does not contain any errors or warnings." )
                         .subs( makeLink( logFileName ) ) );
  }
  item->setData( logFileName, FileIncludeRole );

  logFile.close();
}

void SelfTestDialog::testMySQLServerConfig()
{
  if ( !useStandaloneMysqlServer() ) {
    report( Skip, ki18n( "MySQL server configuration not tested." ),
            ki18n( "The current configuration does not require an internal MySQL server." ) );
    return;
  }

  QStandardItem *item = 0;
  const QString globalConfig = XdgBaseDirs::findResourceFile( "config", QLatin1String( "akonadi/mysql-global.conf" ) );
  const QFileInfo globalConfigInfo( globalConfig );
  if ( !globalConfig.isEmpty() && globalConfigInfo.exists() && globalConfigInfo.isReadable() ) {
    item = report( Success, ki18n( "MySQL server default configuration found." ),
                   ki18n( "The default configuration for the MySQL server was found and is readable at %1." )
                   .subs( makeLink( globalConfig ) ) );
    item->setData( globalConfig, FileIncludeRole );
  } else {
    report( Error, ki18n( "MySQL server default configuration not found." ),
            ki18n( "The default configuration for the MySQL server was not found or was not readable. "
                  "Check your Akonadi installation is complete and you have all required access rights." ) );
  }

  const QString localConfig  = XdgBaseDirs::findResourceFile( "config", QLatin1String( "akonadi/mysql-local.conf" ) );
  const QFileInfo localConfigInfo( localConfig );
  if ( localConfig.isEmpty() || !localConfigInfo.exists() ) {
    report( Skip, ki18n( "MySQL server custom configuration not available." ),
            ki18n( "The custom configuration for the MySQL server was not found but is optional." ) );
  } else if ( localConfigInfo.exists() && localConfigInfo.isReadable() ) {
    item = report( Success, ki18n( "MySQL server custom configuration found." ),
                   ki18n( "The custom configuration for the MySQL server was found and is readable at %1" )
                   .subs( makeLink( localConfig ) ) );
    item->setData( localConfig, FileIncludeRole );
  } else {
    report( Error, ki18n( "MySQL server custom configuration not readable." ),
            ki18n( "The custom configuration for the MySQL server was found at %1 but is not readable. "
                  "Check your access rights." ).subs( makeLink( localConfig ) ) );
  }

  const QString actualConfig = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi" ) ) + QLatin1String( "/mysql.conf" );
  const QFileInfo actualConfigInfo( actualConfig );
  if ( actualConfig.isEmpty() || !actualConfigInfo.exists() || !actualConfigInfo.isReadable() ) {
    report( Error, ki18n( "MySQL server configuration not found or not readable." ),
            ki18n( "The MySQL server configuration was not found or is not readable." ) );
  } else {
    item = report( Success, ki18n( "MySQL server configuration is usable." ),
                   ki18n( "The MySQL server configuration was found at %1 and is readable." ).subs( makeLink( actualConfig ) ) );
    item->setData( actualConfig, FileIncludeRole );
  }
}

void SelfTestDialog::testPSQLServer()
{
  const QString dbname = serverSetting( QLatin1String( "QPSQL" ), "Name", QLatin1String( "akonadi" )).toString();
  const QString hostname = serverSetting( QLatin1String( "QPSQL" ), "Host", QLatin1String( "localhost" )).toString();
  const QString username = serverSetting( QLatin1String( "QPSQL" ), "User", QString() ).toString();
  const QString password = serverSetting( QLatin1String( "QPSQL" ), "Password", QString() ).toString();
  const int port = serverSetting( QLatin1String( "QPSQL" ), "Port", 5432).toInt();

  QSqlDatabase db = QSqlDatabase::addDatabase( QLatin1String( "QPSQL" ) );
  db.setHostName( hostname );
  db.setDatabaseName( dbname );

  if ( !username.isEmpty() )
    db.setUserName( username );

  if ( !password.isEmpty() )
    db.setPassword( password );

  db.setPort( port );

  if ( !db.open() ) {
    const KLocalizedString details = ki18n( db.lastError().text().toLatin1() );
    report( Error, ki18n( "Cannot connect to PostgreSQL server." ),  details);
  }
  else {
    report( Success, ki18n( "PostgreSQL server found." ),
                   ki18n( "The PostgreSQL server was found and connection is working." ));
  }
  db.close();
}

void SelfTestDialog::testAkonadiCtl()
{
  const QString path = KStandardDirs::findExe( QLatin1String( "akonadictl" ) );
  if ( path.isEmpty() ) {
    report( Error, ki18n( "akonadictl not found" ),
                 ki18n( "The program 'akonadictl' needs to be accessible in $PATH. "
                       "Make sure you have the Akonadi server installed." ) );
    return;
  }
  QString result;
  if ( runProcess( path, QStringList() << QLatin1String( "--version" ), result ) ) {
    report( Success, ki18n( "akonadictl found and usable" ),
                   ki18n( "The program '%1' to control the Akonadi server was found "
                         "and could be executed successfully.\nResult:\n%2" ).subs( path ).subs( result ) );
  } else {
    report( Error, ki18n( "akonadictl found but not usable" ),
                 ki18n( "The program '%1' to control the Akonadi server was found "
                       "but could not be executed successfully.\nResult:\n%2\n"
                       "Make sure the Akonadi server is installed correctly." ).subs( path ).subs( result ) );
  }
}

void SelfTestDialog::testServerStatus()
{
  if ( DBusConnectionPool::threadConnection().interface()->isServiceRegistered( ServerManager::serviceName(ServerManager::Control) ) ) {
    report( Success, ki18n( "Akonadi control process registered at D-Bus." ),
                   ki18n( "The Akonadi control process is registered at D-Bus which typically indicates it is operational." ) );
  } else {
    report( Error, ki18n( "Akonadi control process not registered at D-Bus." ),
                 ki18n( "The Akonadi control process is not registered at D-Bus which typically means it was not started "
                       "or encountered a fatal error during startup."  ) );
  }

  if ( DBusConnectionPool::threadConnection().interface()->isServiceRegistered( ServerManager::serviceName(ServerManager::Server) ) ) {
    report( Success, ki18n( "Akonadi server process registered at D-Bus." ),
                   ki18n( "The Akonadi server process is registered at D-Bus which typically indicates it is operational." ) );
  } else {
    report( Error, ki18n( "Akonadi server process not registered at D-Bus." ),
                 ki18n( "The Akonadi server process is not registered at D-Bus which typically means it was not started "
                       "or encountered a fatal error during startup."  ) );
  }
}

void SelfTestDialog::testSearchStatus()
{
  bool searchAvailable = false;
  if ( DBusConnectionPool::threadConnection().interface()->isServiceRegistered( AKONADI_SEARCH_SERVICE ) ) {
    searchAvailable = true;
    report( Success, ki18n( "Nepomuk search service registered at D-Bus." ),
                   ki18n( "The Nepomuk search service is registered at D-Bus which typically indicates it is operational." ) );
  } else {
    report( Error, ki18n( "Nepomuk search service not registered at D-Bus." ),
                   ki18n( "The Nepomuk search service is not registered at D-Bus which typically means it was not started "
                          "or encountered a fatal error during startup."  ) );
  }

  if ( searchAvailable ) {
    // check which backend is used
    QDBusInterface interface( QLatin1String( "org.kde.NepomukStorage" ), QLatin1String( "/nepomukstorage" ) );
    const QDBusReply<QString> reply = interface.call( QLatin1String( "usedSopranoBackend" ) );
    if ( reply.isValid() ) {
      const QString name = reply.value();

      // put blacklisted backends here
      if ( name.contains( QLatin1String( "redland" ) ) ) {
        report( Error, ki18n( "Nepomuk search service uses inappropriate backend." ),
                       ki18n( "The Nepomuk search service uses the '%1' backend, which is not "
                              "recommended for use with Akonadi." ).subs( name ) );
      } else {
        report( Success, ki18n( "Nepomuk search service uses an appropriate backend. " ),
                         ki18n( "The Nepomuk search service uses one of the recommended backends." ) );
      }
    }
  }
}

void SelfTestDialog::testProtocolVersion()
{
  if ( Internal::serverProtocolVersion() < 0 ) {
    report( Skip, ki18n( "Protocol version check not possible." ),
            ki18n( "Without a connection to the server it is not possible to check if the protocol version meets the requirements." ) );
    return;
  }
  if ( Internal::serverProtocolVersion() < SessionPrivate::minimumProtocolVersion() ) {
    report( Error, ki18n( "Server protocol version is too old." ),
            ki18n( "The server protocol version is %1, but at least version %2 is required. "
                  "Install a newer version of the Akonadi server." )
                  .subs( Internal::serverProtocolVersion() )
                  .subs( SessionPrivate::minimumProtocolVersion() ) );
  } else {
    report( Success, ki18n( "Server protocol version is recent enough." ),
            ki18n( "The server Protocol version is %1, which equal or newer than the required version %2." )
                .subs( Internal::serverProtocolVersion() )
                .subs( SessionPrivate::minimumProtocolVersion() ) );
  }
}

void SelfTestDialog::testResources()
{
  AgentType::List agentTypes = AgentManager::self()->types();
  bool resourceFound = false;
  foreach ( const AgentType &type, agentTypes ) {
    if ( type.capabilities().contains( QLatin1String( "Resource" ) ) ) {
      resourceFound = true;
      break;
    }
  }

  const QStringList pathList = XdgBaseDirs::findAllResourceDirs( "data", QLatin1String( "akonadi/agents" ) );
  QStandardItem *item = 0;
  if ( resourceFound ) {
    item = report( Success, ki18n( "Resource agents found." ), ki18n( "At least one resource agent has been found." ) );
  } else {
    item = report( Error, ki18n( "No resource agents found." ),
      ki18n( "No resource agents have been found, Akonadi is not usable without at least one. "
            "This usually means that no resource agents are installed or that there is a setup problem. "
            "The following paths have been searched: '%1'. "
            "The XDG_DATA_DIRS environment variable is set to '%2'; make sure this includes all paths "
            "where Akonadi agents are installed." )
          .subs( pathList.join( QLatin1String( " " ) ) )
          .subs( QString::fromLocal8Bit( qgetenv( "XDG_DATA_DIRS" ) ) ) );
  }
  item->setData( pathList, ListDirectoryRole );
  item->setData( QByteArray( "XDG_DATA_DIRS" ), EnvVarRole );
}

void Akonadi::SelfTestDialog::testServerLog()
{
  QString serverLog = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi" ) )
      + QDir::separator() + QString::fromLatin1( "akonadiserver.error" );
  QFileInfo info( serverLog );
  if ( !info.exists() || info.size() <= 0 ) {
    report( Success, ki18n( "No current Akonadi server error log found." ),
                   ki18n( "The Akonadi server did not report any errors during its current startup." ) );
  } else {
    QStandardItem *item = report( Error, ki18n( "Current Akonadi server error log found." ),
      ki18n( "The Akonadi server reported errors during its current startup. The log can be found in %1." ).subs( makeLink( serverLog ) ) );
    item->setData( serverLog, FileIncludeRole );
  }

  serverLog += QLatin1String( ".old" );
  info.setFile( serverLog );
  if ( !info.exists() || info.size() <= 0 ) {
    report( Success, ki18n( "No previous Akonadi server error log found." ),
                   ki18n( "The Akonadi server did not report any errors during its previous startup." ) );
  } else {
    QStandardItem *item = report( Error, ki18n( "Previous Akonadi server error log found." ),
      ki18n( "The Akonadi server reported errors during its previous startup. The log can be found in %1." ).subs( makeLink( serverLog ) ) );
    item->setData( serverLog, FileIncludeRole );
  }
}

void SelfTestDialog::testControlLog()
{
  QString controlLog = XdgBaseDirs::saveDir( "data", QLatin1String( "akonadi" ) )
      + QDir::separator() + QString::fromLatin1( "akonadi_control.error" );
  QFileInfo info( controlLog );
  if ( !info.exists() || info.size() <= 0 ) {
    report( Success, ki18n( "No current Akonadi control error log found." ),
                   ki18n( "The Akonadi control process did not report any errors during its current startup." ) );
  } else {
    QStandardItem *item = report( Error, ki18n( "Current Akonadi control error log found." ),
      ki18n( "The Akonadi control process reported errors during its current startup. The log can be found in %1." ).subs( makeLink( controlLog ) ) );
    item->setData( controlLog, FileIncludeRole );
  }

  controlLog += QLatin1String( ".old" );
  info.setFile( controlLog );
  if ( !info.exists() || info.size() <= 0 ) {
    report( Success, ki18n( "No previous Akonadi control error log found." ),
                   ki18n( "The Akonadi control process did not report any errors during its previous startup." ) );
  } else {
    QStandardItem *item = report( Error, ki18n( "Previous Akonadi control error log found." ),
      ki18n( "The Akonadi control process reported errors during its previous startup. The log can be found in %1." ).subs( makeLink( controlLog ) ) );
    item->setData( controlLog, FileIncludeRole );
  }
}


void SelfTestDialog::testRootUser()
{
  KUser user;
  if ( user.isSuperUser() ) {
    report( Error, ki18n( "Akonadi was started as root" ), ki18n( "Running Internet-facing applications as root/administrator exposes you to many security risks. MySQL, used by this Akonadi installation, will not allow itself to run as root, to protect you from these risks." ) );
  } else {
    report( Success, ki18n( "Akonadi is not running as root" ), ki18n( "Akonadi is not running as a root/administrator user, which is the recommended setup for a secure system." ) );
  }
}

QString SelfTestDialog::createReport()
{
  QString result;
  QTextStream s( &result );
  s << "Akonadi Server Self-Test Report" << endl;
  s << "===============================" << endl;

  for ( int i = 0; i < mTestModel->rowCount(); ++i ) {
    QStandardItem *item = mTestModel->item( i );
    s << endl;
    s << "Test " << (i + 1) << ":  ";
    switch ( item->data( ResultTypeRole ).toInt() ) {
      case Skip:
        s << "SKIP"; break;
      case Success:
        s << "SUCCESS"; break;
      case Warning:
        s << "WARNING"; break;
      case Error:
      default:
        s << "ERROR"; break;
    }
    s << endl << "--------" << endl;
    s << endl;
    s << item->data( SummaryRole ).toString() << endl;
    s << "Details: " << item->data( DetailsRole ).toString() << endl;
    if ( item->data( FileIncludeRole ).isValid() ) {
      s << endl;
      const QString fileName = item->data( FileIncludeRole ).toString();
      QFile f( fileName );
      if ( f.open( QFile::ReadOnly ) ) {
        s << "File content of '" << fileName << "':" << endl;
        s << f.readAll() << endl;
      } else {
        s << "File '" << fileName << "' could not be opened" << endl;
      }
    }
    if ( item->data( ListDirectoryRole ).isValid() ) {
      s << endl;
      const QStringList pathList = item->data( ListDirectoryRole ).toStringList();
      if ( pathList.isEmpty() )
        s << "Directory list is empty." << endl;
      foreach ( const QString &path, pathList ) {
        s << "Directory listing of '" << path << "':" << endl;
        QDir dir( path );
        dir.setFilter( QDir::AllEntries | QDir::NoDotAndDotDot );
        foreach ( const QString &entry, dir.entryList() )
          s << entry << endl;
      }
    }
    if ( item->data( EnvVarRole ).isValid() ) {
      s << endl;
      const QByteArray envVarName = item->data( EnvVarRole ).toByteArray();
      const QByteArray envVarValue = qgetenv( envVarName );
      s << "Environment variable " << envVarName << " is set to '" << envVarValue << "'" << endl;
    }
  }

  s << endl;
  s.flush();
  return result;
}

void SelfTestDialog::saveReport()
{
  const QString defaultFileName = QLatin1String( "akonadi-selftest-report-" )
                   + QDate::currentDate().toString( QLatin1String( "yyyyMMdd" ) )
                   + QLatin1String( ".txt" );
  const QString fileName =  KFileDialog::getSaveFileName( QUrl(defaultFileName), QString(), this,
                                                          i18n( "Save Test Report" ) );
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );
  if ( !file.open( QFile::ReadWrite ) ) {
    KMessageBox::error( this, i18n( "Could not open file '%1'", fileName ) );
    return;
  }

  file.write( createReport().toUtf8() );
  file.close();
}

void SelfTestDialog::copyReport()
{
#ifndef QT_NO_CLIPBOARD
  QApplication::clipboard()->setText( createReport() );
#endif
}

void SelfTestDialog::linkActivated(const QString & link)
{
  KRun::runUrl( KUrl::fromPath( link ), QLatin1String( "text/plain" ), this );
}

// @endcond

#include "moc_selftestdialog_p.cpp"
