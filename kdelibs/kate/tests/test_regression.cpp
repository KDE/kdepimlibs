/**
 * This file is part of the KDE project
 *
 * Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
 * Copyright (C) 2003,2004 Stephan Kulow (coolo@kde.org)
 * Copyright (C) 2004 Dirk Mueller ( mueller@kde.org )
 * Copyright 2006, 2007 Leo Savernik (l.savernik@aon.at)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

//BEGIN Includes
#include "test_regression.h"
#include "testutils.h"

#include "kateview.h"
#include "katedocument.h"
#include "katedocumenthelpers.h"
#include "kateconfig.h"
#include "katecmd.h"
#include "kateglobal.h"
#include <ktexteditor/commandinterface.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kmainwindow.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <kdefakes.h>
#include <kstatusbar.h>
#include <kio/job.h>

#include <memory>
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>

#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtCore/Q_PID>
#include <QtCore/QEvent>
#include <QtCore/QTimer>

#include <QtScript/QScriptEngine>
#include <QTest>

//END Includes

#define BASE_DIR_CONFIG "/.testkateregression"
#define UNIQUE_HOME_DIR "/var/tmp/%1_kate4_non_existent"

static KMainWindow* toplevel;

// -------------------------------------------------------------------------

const char failureSnapshotPrefix[] = "testkateregressionrc-FS.";

static QString findMostRecentFailureSnapshot()
{
  QDir dir(KGlobal::dirs()->saveLocation("config"),
           QString(failureSnapshotPrefix) + '*',
           QDir::Time, QDir::Files);
  QStringList entries = dir.entryList();
  return entries.isEmpty() ? QString() : dir[0].mid(sizeof failureSnapshotPrefix - 1);
}

int main(int argc, char *argv[])
{
  KCmdLineOptions options;
  options.add("b");
  options.add("base <base_dir>", ki18n("Directory containing tests, basedir and output directories."));
  options.add("cmp-failures <snapshot>", ki18n("Compare failures of this testrun against snapshot <snapshot>. Defaults to the most recently captured failure snapshot or none if none exists."));
  options.add("d");
  options.add("debug", ki18n("Do not suppress debug output"));
  options.add("g");
  options.add("genoutput", ki18n("Regenerate baseline (instead of checking)"));
  options.add("keep-output", ki18n("Keep output files even on success"));
  options.add("save-failures <snapshot>", ki18n("Save failures of this testrun as failure snapshot <snapshot>"));
  options.add("s");
  options.add("show", ki18n("Show the window while running tests"));
  options.add("t");
  options.add("test <filename>", ki18n("Only run a single test. Multiple options allowed."));
  options.add("o");
  options.add("output <directory>", ki18n("Put output in <directory> instead of <base_dir>/output"));
  options.add("fork", ki18n("Run each test case in a separate process."));
  options.add("+[base_dir]", ki18n("Directory containing tests, basedir and output directories. Only regarded if -b is not specified."));
  options.add("+[testcases]", ki18n("Relative path to testcase, or directory of testcases to be run (equivalent to -t)."));

  // forget about any settings
  passwd* password = getpwuid( getuid() );
  if (!password) {
    fprintf(stderr, "dang, I don't even know who I am.\n");
    exit(1);
  }

  QString kdeHome(UNIQUE_HOME_DIR);
  kdeHome = kdeHome.arg( password->pw_name );
  setenv( "KDEHOME", kdeHome.toLatin1().constData(), 1 );
  setenv( "LC_ALL", "C", 1 );
  setenv( "LANG", "C", 1 );

//   signal( SIGALRM, signal_handler );

  KCmdLineArgs::init(argc, argv, "testregression", 0, ki18n("TestRegression"),
                     "1.0", ki18n("Regression tester for kate"));
  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs( );

  QString baseDir = args->getOption("base");
  QByteArray homeDir = qgetenv("HOME");
  QByteArray baseDirConfigFile(homeDir + QByteArray(BASE_DIR_CONFIG));
  {
    QFile baseDirConfig(baseDirConfigFile);
    if (baseDirConfig.open(QFile::ReadOnly)) {
      QTextStream bds(&baseDirConfig);
      baseDir = bds.readLine();
    }
  }

  if ( args->count() < 1 && baseDir.isEmpty() ) {
    printf("For regression testing, make sure to have checked out the kate regression\n"
           "testsuite from svn:\n"
           "\tsvn co \"https://<user>@svn.kde.org:/home/kde/trunk/tests/katetests/regression\"\n"
           "Remember the root path into which you checked out the testsuite.\n"
           "\n");
    printf("%s needs the root path of the kate regression\n"
           "testsuite to function properly\n"
           "By default, the root path is looked up in the file\n"
           "\t%s\n"
           "If it doesn't exist yet, create it by invoking\n"
           "\techo \"<root-path>\" > %s\n"
           "You may override the location by specifying the root explicitly on the\n"
           "command line with option -b\n"
           "", argv[0],
           baseDirConfigFile.constData(),
           baseDirConfigFile.constData());
    ::exit( 1 );
  }

  int testcase_index = 0;
  if (baseDir.isEmpty()) baseDir = args->arg(testcase_index++);

  QFileInfo bdInfo(baseDir);
  baseDir = bdInfo.absoluteFilePath();

  const char *subdirs[] = {"tests", "baseline", "output", "resources"};
  for ( int i = 0; i < 2; i++ ) {
    QFileInfo sourceDir(baseDir + '/' + subdirs[i]);
    if ( !sourceDir.exists() || !sourceDir.isDir() ) {
      fprintf(stderr,"ERROR: Source directory \"%s/%s\": no such directory.\n", baseDir.toLatin1().constData(), subdirs[i]);
      exit(1);
    }
  }

  KateTestApp a(args, baseDir, testcase_index);

  // queue quit action
  QTimer::singleShot(0, &a, SLOT(quit()));

  a.exec();

  return a.allTestsSucceeded() ? 0 : 1;
}

// -------------------------------------------------------------------------

RegressionTest::RegressionTest(KateDocument *part, const KConfig *baseConfig,
                               const QString &baseDir, const KCmdLineArgs *args)
  : QObject(part)
  , m_part(part)
  , m_view( static_cast<KateView *>(m_part->widget()) )
  , m_baseConfig(baseConfig)
  , m_baseDir(baseDir)
  , m_outputDir(args->getOption("output"))
  , m_genOutput(args->isSet("genoutput"))
  , m_fork(args->isSet("fork"))
  , m_failureComp(0)
  , m_failureSave(0)
  , m_keepOutput(args->isSet("keep-output"))
  , m_passes_work(0)
  , m_passes_fail(0)
  , m_passes_new(0)
  , m_failures_work(0)
  , m_failures_fail(0)
  , m_failures_new(0)
  , m_errors(0)
{
  m_baseDir = m_baseDir.replace( "//", "/" );
  if ( m_baseDir.endsWith( "/" ) )
    m_baseDir = m_baseDir.left( m_baseDir.length() - 1 );

  if (m_outputDir.isEmpty())
    m_outputDir = m_baseDir + "/output";

  QFile::remove( m_outputDir + "/links.html" );
  QFile f( m_outputDir + "/empty.html" );
  QString s;
  f.open( QFile::WriteOnly | QFile::Truncate );
  s = "<html><body>Follow the white rabbit";
  f.write( s.toLatin1(), s.length() );
  f.close();
  f.setFileName( m_outputDir + "/index.html" );
  f.open( QFile::WriteOnly | QFile::Truncate );
  s = "<html><frameset cols=150,*><frame src=links.html><frame name=content src=empty.html>";
  f.write( s.toLatin1(), s.length() );
  f.close();
}

static QStringList readListFile( const QString &filename )
{
  // Read ignore file for this directory
  QString ignoreFilename = filename;
  QFileInfo ignoreInfo(ignoreFilename);
  QStringList ignoreFiles;
  if (ignoreInfo.exists()) {
    QFile ignoreFile(ignoreFilename);
    if (!ignoreFile.open(QFile::ReadOnly)) {
      fprintf(stderr,"Can't open %s\n",ignoreFilename.toLatin1().constData());
      exit(1);
    }
    QTextStream ignoreStream(&ignoreFile);
    QString line;
    while (!(line = ignoreStream.readLine()).isNull())
      ignoreFiles.append(line);
    ignoreFile.close();
  }
  return ignoreFiles;
}

RegressionTest::~RegressionTest()
{
  // Important! Delete comparison config *first* as saver config
  // might point to the same physical file.
  KConfig *tmp = m_failureComp ? m_failureComp->config() : 0;
  delete m_failureComp;
  if (m_failureSave && tmp != m_failureSave->config()) {
    delete tmp;
    tmp = m_failureSave->config();
  }
  delete m_failureSave;
  delete tmp;
}

void RegressionTest::setFailureSnapshotConfig(KConfig *cfg, const QString &sname)
{
  Q_ASSERT(cfg);
  m_failureComp = new KConfigGroup(cfg, sname);
}

void RegressionTest::setFailureSnapshotSaver(KConfig *cfg, const QString &sname)
{
  Q_ASSERT(cfg);
  m_failureSave = new KConfigGroup(cfg, sname);
}

QStringList RegressionTest::concatListFiles(const QString &relPath, const QString &filename)
{
  QStringList cmds;
  int pos = relPath.lastIndexOf('/');
  if (pos >= 0)
    cmds += concatListFiles(relPath.left(pos), filename);
  cmds += readListFile(m_baseDir + "/tests/" + relPath + '/' + filename);
  return cmds;
}

bool RegressionTest::runTests(QString relPath, bool mustExist, int known_failure)
{
  if (!QFile(m_baseDir + "/tests/" + relPath).exists()) {
    fprintf(stderr,"%s: No such file or directory\n",relPath.toLatin1().constData());
    return false;
  }

  QString fullPath = m_baseDir + "/tests/"+relPath;
  QFileInfo info(fullPath);

  if (!info.exists() && mustExist) {
    fprintf(stderr,"%s: No such file or directory\n",relPath.toLatin1().constData());
    return false;
  }

  if (!info.isReadable() && mustExist) {
    fprintf(stderr,"%s: Access denied\n",relPath.toLatin1().constData());
    return false;
  }

  if (info.isDir()) {
    QStringList ignoreFiles = readListFile(  m_baseDir + "/tests/"+relPath+"/ignore" );
    QStringList failureFiles = readListFile(  m_baseDir + "/tests/"+relPath+"/KNOWN_FAILURES" );

    // Run each test in this directory, recusively
    QDir sourceDir(m_baseDir + "/tests/"+relPath);
    for (uint fileno = 0; fileno < sourceDir.count(); fileno++) {
      QString filename = sourceDir[fileno];
      QString relFilename = relPath.isEmpty() ? filename : relPath+'/'+filename;

      if (filename.startsWith(".") || ignoreFiles.contains(filename) )
        continue;
      int failure_type = NoFailure;
      if ( failureFiles.contains( filename ) )
        failure_type |= AllFailure;
      if ( failureFiles.contains ( filename + "-result" ) )
        failure_type |= ResultFailure;
      runTests(relFilename, false, failure_type);
    }
  } else if (info.isFile()) {

    QString relativeDir = QFileInfo(relPath).dir().path();
    QString filename = info.fileName();
    QString currentBase = m_baseDir + "/tests/"+relativeDir;
    m_currentCategory = relativeDir;
    m_currentTest = filename;
    m_known_failures = known_failure;
    m_outputCustomised = false;
    // gather commands
    // directory-specific commands
    QStringList commands = concatListFiles(relPath, ".kateconfig-commands");
    // testcase-specific commands
    commands += readListFile(currentBase + '/' + filename + "-commands");

    rereadConfig(); // reset options to default
    if ( filename.endsWith(".txt") ) {
#if 0
      if ( relPath.startsWith( "domts/" ) && !m_runJS )
        return true;
      if ( relPath.startsWith( "ecma/" ) && !m_runJS )
        return true;
#endif
//       if ( m_runHTML )
      testStaticFile(relPath, commands);
    } else if (mustExist) {
      fprintf(stderr,"%s: Not a valid test file (must be .txt)\n",relPath.toLatin1().constData());
      return false;
    }
  } else if (mustExist) {
    fprintf(stderr,"%s: Not a regular file\n",relPath.toLatin1().constData());
    return false;
  }

  return true;
}

bool RegressionTest::allTestsSucceeded() const
{
  return m_failures_work == 0 && m_errors == 0;
}

void RegressionTest::createLink( const QString& test, int failures )
{
  OutputObject::createMissingDirs( m_outputDir + '/' + test + "-compare.html" );

  QFile list( m_outputDir + "/links.html" );
  list.open( QFile::WriteOnly|QFile::Append );
  QString link;
  link = QString( "<a href=\"%1\" target=\"content\" title=\"%2\">" )
      .arg( test + "-compare.html" )
      .arg( test );
  link += m_currentTest;
  link += "</a> ";
  if (failures & NewFailure)
    link += "<span style=\"font-weight:bold;color:red\">";
  link += '[';
  if ( failures & ResultFailure )
    link += 'R';
  link += ']';
  if (failures & NewFailure)
    link += "</span>";
  link += "<br>\n";
  list.write( link.toLatin1(), link.length() );
  list.close();
}

/** returns the path in a way that is relatively reachable from base.
 * @param base base directory (must not include trailing slash)
 * @param path directory/file to be relatively reached by base
 * @return path with all elements replaced by .. and concerning path elements
 *	to be relatively reachable from base.
 */
static QString makeRelativePath(const QString &base, const QString &path)
{
  QString absBase = QFileInfo(base).absoluteFilePath();
  QString absPath = QFileInfo(path).absoluteFilePath();
//   kDebug() << "absPath: \"" << absolutePath << "\"";
//   kDebug() << "absBase: \"" << absoluteBase << "\"";

  // walk up to common ancestor directory
  int pos = 0;
  do {
    pos++;
    int newpos = absBase.indexOf('/', pos);
    if (newpos == -1) newpos = absBase.length();
    QString cmpPathComp = QString::fromRawData(absPath.unicode() + pos, newpos - pos);
    QString cmpBaseComp = QString::fromRawData(absBase.unicode() + pos, newpos - pos);
//       kDebug() << "cmpPathComp: \"" << cmpPathComp << "\"";
//       kDebug() << "cmpBaseComp: \"" << cmpBaseComp << "\"";
//       kDebug() << "pos: " << pos << " newpos: " << newpos;
    if (cmpPathComp != cmpBaseComp) { pos--; break; }
    pos = newpos;
  } while (pos < (int)absBase.length() && pos < (int)absPath.length());
  int basepos = pos < (int)absBase.length() ? pos + 1 : pos;
  int pathpos = pos < (int)absPath.length() ? pos + 1 : pos;

//   kDebug() << "basepos " << basepos << " pathpos " << pathpos;

  QString rel;
  {
    QString relBase = QString::fromRawData(absBase.unicode() + basepos, absBase.length() - basepos);
    QString relPath = QString::fromRawData(absPath.unicode() + pathpos, absPath.length() - pathpos);
    // generate as many .. as there are path elements in relBase
    if (relBase.length() > 0) {
      for (int i = relBase.count('/'); i > 0; --i)
        rel += "../";
      rel += "..";
      if (relPath.length() > 0) rel += '/';
    }
    rel += relPath;
  }
  return rel;
}

/**
 * returns a unique file name
 * (Cannot have QTemporaryFile as it won't return a file name without actually
 * opening the file. Besides, it contains an indeterminate id which differs
 * between processes.)
 */
static QString getTempFileName(const QString &name)
{
  return QDir::tempPath()+"/testkateregression-"+name;
}

/** writes an ipc-variable */
static void writeVariable(const QString &varName, const QString &content)
{
  QString fn = getTempFileName(varName);
  QFile::remove(fn);
  QFile f(fn);
  if (!f.open(QFile::WriteOnly))
    return;   // FIXME be more elaborate
  f.write(content.toLatin1());
//   fprintf(stderr, "writing: %s: %s\n", fn.toLatin1().constData(), content.toLatin1().constData());
}

/** reads an ipc-variable */
static QString readVariable(const QString &varName)
{
  QString fn = getTempFileName(varName);
  QFile f(fn);
  if (!f.open(QFile::ReadOnly))
    return QString();   // FIXME be more elaborate
  QByteArray content = f.readAll();
  f.close();
  QFile::remove(fn);
//   fprintf(stderr, "reading: %s: %s\n", fn.toLatin1().constData(), content.constData());
  return QLatin1String(content.constData());
}

void RegressionTest::doFailureReport( const QString& test, int failures )
{
  if ( failures == NoFailure ) {
    QFile::remove( m_outputDir + '/' + test + "-compare.html" );
    return;
  }

  createLink( test, failures );

  QFile compare( m_outputDir + '/' + test + "-compare.html" );

  QString testFile = QFileInfo(test).fileName();

  QString renderDiff;
  QString domDiff;

  QString pwd = QDir::currentPath();
  QDir::setCurrent( m_baseDir );
  QString resolvedBaseDir = QDir::currentPath();

  QString relOutputDir = makeRelativePath(resolvedBaseDir/*m_baseDir*/, m_outputDir);

  // are blocking reads possible with K3Process?

  if ( failures & ResultFailure ) {
    domDiff += "<pre>";
    QProcess diff;
    QStringList args;
    args << "-u" << QString::fromLatin1("baseline/%1-result").arg(test)
        << QString::fromLatin1("%3/%2-result").arg ( test, relOutputDir );
    diff.start("diff", args);
    diff.waitForFinished();
    QByteArray out = diff.readAllStandardOutput();
    QByteArray err = diff.readAllStandardError();
    QTextStream *is = new QTextStream( out, QFile::ReadOnly );
    for ( int line = 0; line < 100 && !is->atEnd(); ++line ) {
      QString l = is->readLine();
      l = l.replace( '<', "&lt;" );
      l = l.replace( '>', "&gt;" );
      l = l.replace( QRegExp("(\t+)"), "<span style=\"background:lightblue\">\\1</span>" );
      domDiff += l  + '\n';
    }
    delete is;
    domDiff += "</pre>";
    if (!err.isEmpty()) {
      qWarning() << "cwd: " << resolvedBaseDir << ", basedir " << m_baseDir;
      qWarning() << "diff " << args.join(" ");
      qWarning() << "Errors: " << err;
    }
  }

  QDir::setCurrent( pwd );

    // create a relative path so that it works via web as well. ugly
  QString relpath = makeRelativePath(m_outputDir + '/'
      + QFileInfo(test).dir().path(), resolvedBaseDir/*m_baseDir*/);

  compare.open( QFile::WriteOnly|QFile::Truncate );
  QString cl;
  cl = QString( "<html><head><title>%1</title>" ).arg( test );
  cl += QString( "<script>\n"
      "var pics = new Array();\n"
      "pics[0]=new Image();\n"
      "pics[0].src = '%1';\n"
      "pics[1]=new Image();\n"
      "pics[1].src = '%2';\n"
      "var doflicker = 1;\n"
      "var t = 1;\n"
      "var lastb=0;\n" )
      .arg( relpath+"/baseline/"+test+"-dump.png" )
      .arg( testFile+"-dump.png" );
  cl += QString( "function toggleVisible(visible) {\n"
                 "     document.getElementById('render').style.visibility= visible == 'render' ? 'visible' : 'hidden';\n"
                 "     document.getElementById('image').style.visibility= visible == 'image' ? 'visible' : 'hidden';\n"
                 "     document.getElementById('dom').style.visibility= visible == 'dom' ? 'visible' : 'hidden';\n"
                 "}\n"
                 "function show() { document.getElementById('image').src = pics[t].src; "
                 "document.getElementById('image').style.borderColor = t && !doflicker ? 'red' : 'gray';\n"
                 "toggleVisible('image');\n"
                 "}" );
  cl += QString ( "function runSlideShow(){\n"
                  "   document.getElementById('image').src = pics[t].src;\n"
                  "   if (doflicker)\n"
                  "       t = 1 - t;\n"
                  "   setTimeout('runSlideShow()', 200);\n"
                  "}\n"
                  "function m(b) { if (b == lastb) return; document.getElementById('b'+b).className='buttondown';\n"
                  "                var e = document.getElementById('b'+lastb);\n"
                  "                 if(e) e.className='button';\n"
                  "                 lastb = b;\n"
                  "}\n"
                  "function showRender() { doflicker=0;toggleVisible('render')\n"
                  "}\n"
                  "function showDom() { doflicker=0;toggleVisible('dom')\n"
                  "}\n"
                  "</script>\n");

  cl += QString ("<style>\n"
                 ".buttondown { cursor: pointer; padding: 0px 20px; color: white; background-color: blue; border: inset blue 2px;}\n"
                 ".button { cursor: pointer; padding: 0px 20px; color: black; background-color: white; border: outset blue 2px;}\n"
                 ".diff { position: absolute; left: 10px; top: 100px; visibility: hidden; border: 1px black solid; background-color: white; color: black; /* width: 800; height: 600; overflow: scroll; */ }\n"
                 "</style>\n" );

  cl += QString( "<body onload=\"m(5); toggleVisible('dom');\"" );
  cl += QString(" text=black bgcolor=gray>\n<h1>%3</h1>\n" ).arg( test );
  if ( renderDiff.length() )
    cl += "<span id='b4' class='button' onclick='showRender();m(4)'>R-DIFF</span>&nbsp;\n";
  if ( domDiff.length() )
    cl += "<span id='b5' class='button' onclick='showDom();m(5);'>D-DIFF</span>&nbsp;\n";
  // The test file always exists - except for checkOutput called from *.js files
  if ( QFile::exists( m_baseDir + "/tests/"+ test ) )
    cl += QString( "<a class=button href=\"%1\">HTML</a>&nbsp;" )
        .arg( relpath+"/tests/"+test );

  cl += QString( "<hr>"
                 "<img style='border: solid 5px gray' src=\"%1\" id='image'>" )
    .arg( relpath+"/baseline/"+test+"-dump.png" );

  cl += "<div id='render' class='diff'>" + renderDiff + "</div>";
  cl += "<div id='dom' class='diff'>" + domDiff + "</div>";

  cl += "</body></html>";
  compare.write( cl.toLatin1(), cl.length() );
  compare.close();
}

void RegressionTest::testStaticFile(const QString & filename, const QStringList &commands)
{
  toplevel->resize( 800, 600); // restore size

  // Set arguments
  KParts::OpenUrlArguments args;
  if (filename.endsWith(".txt")) args.setMimeType("text/plain");
  m_part->setArguments(args);
  // load page
  KUrl url;
  url.setProtocol("file");
  url.setPath(QFileInfo(m_baseDir + "/tests/"+filename).absoluteFilePath());
  m_part->openUrl(url);

  // inject commands
  for (QStringList::ConstIterator cit = commands.begin(); cit != commands.end(); ++cit) {
    QString str = (*cit).trimmed();
//     kDebug() << "command: " << str;
    if (str.isEmpty() || str.startsWith("#")) continue;
    KTextEditor::Command *cmd = KateCmd::self()->queryCommand(str);
    if (cmd) {
      QString msg;
      if (!cmd->exec(m_view, str, msg))
        fprintf(stderr, "ERROR executing command '%s': %s\n", str.toLatin1().constData(), msg.toLatin1().constData());
    }
  }

//   pause(200);

//   Q_ASSERT(m_part->config()->configFlags() & KateDocumentConfig::cfDoxygenAutoTyping);

  bool script_error = false;
  pid_t pid = m_fork ? fork() : 0;
  if (pid == 0) {
    // Execute script
    TestScriptEnv jsenv(m_part, m_outputCustomised);
    jsenv.output()->setOutputFile( ( m_genOutput ? QString(m_baseDir + "/baseline/") : QString(m_outputDir + '/') ) + filename + "-result" );
    script_error = evalJS(jsenv.engine(), m_baseDir + "/tests/"+QFileInfo(filename).dir().path()+"/.kateconfig-script", true)
        && evalJS(jsenv.engine(), m_baseDir + "/tests/"+filename+"-script");

    if (m_fork) {
      writeVariable("script_error", QString::number(script_error));
      writeVariable("m_errors", QString::number(m_errors));
      writeVariable("m_outputCustomised", QString::number(m_outputCustomised));
      writeVariable("m_part.text", m_part->text());
      signal(SIGABRT, SIG_DFL);   // Dr. Konqi, no interference please
      ::abort();  // crash, don't let Qt/KDE do any fancy deinit stuff
    }
  } else if (pid == -1) {
    // whoops, will fail later on comparison
    m_errors++;
  } else {
    // wait for child to finish
    int status;
    waitpid(pid, &status, 0);
    // read in potentially changed variables
    script_error = (bool)readVariable("script_error").toInt();
    m_errors = readVariable("m_errors").toInt();
    m_outputCustomised = (bool)readVariable("m_outputCustomised").toInt();
    m_part->setText(readVariable("m_part.text"));
//     fprintf(stderr, "script_error = %d, m_errors = %d, m_outputCustomised = %d\n", script_error, m_errors, m_outputCustomised);
  }

  int back_known_failures = m_known_failures;

  if (!script_error) goto bail_out;

  kapp->processEvents();

  if ( m_genOutput ) {
    reportResult(checkOutput(filename+"-result"), "result");
  } else {
    int failures = NoFailure;

    // compare with output file
    if ( m_known_failures & ResultFailure)
      m_known_failures = AllFailure;
    bool newfail;
    if ( !reportResult( checkOutput(filename+"-result"), "result", &newfail ) )
      failures |= ResultFailure;
    if (newfail)
      failures |= NewFailure;

    doFailureReport(filename, failures );
  }

bail_out:
  m_known_failures = back_known_failures;
  m_part->setModified(false);
  m_part->closeUrl();
}

bool RegressionTest::evalJS(QScriptEngine *engine, const QString &filename, bool ignore_nonexistent)
{
  QFile sourceFile(filename);

  if (!sourceFile.open(QFile::ReadOnly)) {
    if (!ignore_nonexistent) {
      fprintf(stderr,"ERROR reading file %s\n",filename.toLatin1().constData());
      m_errors++;
    }
    return ignore_nonexistent;
  }

  QTextStream stream(&sourceFile);
  stream.setCodec("UTF8");
  QString code = stream.readAll();
  sourceFile.close();

  QScriptValue result = engine->evaluate(code, filename, 1);

  if (result.isError()) {
    fprintf(stderr, "eval script failed\n");
    QString errmsg = result.toString();
    printf( "ERROR: %s (%s)\n", filename.toLatin1().constData(), errmsg.toLatin1().constData());
    m_errors++;
    return false;
  }
  return true;
}

RegressionTest::CheckResult RegressionTest::checkOutput(const QString &againstFilename)
{
  QString absFilename = QFileInfo(m_baseDir + "/baseline/" + againstFilename).absoluteFilePath();
  if ( svnIgnored( absFilename ) ) {
    m_known_failures = NoFailure;
    return Ignored;
  }

  CheckResult result = Success;

  // compare result to existing file
  QString outputFilename = QFileInfo(m_outputDir + '/' + againstFilename).absoluteFilePath();
  bool kf = false;
  if ( m_known_failures & AllFailure )
    kf = true;
  if ( kf )
    outputFilename += "-KF";

  if ( m_genOutput )
    outputFilename = absFilename;

  // get existing content
  QString data;
  if (m_outputCustomised) {
    QFile file2(outputFilename);
    if (!file2.open(QFile::ReadOnly)) {
      fprintf(stderr,"Error reading file %s\n",outputFilename.toLatin1().constData());
      exit(1);
    }
    data = file2.readAll();
  } else {
    data = m_part->text();
  }

  QFile file(absFilename);
  if (file.open(QFile::ReadOnly)) {
    QTextStream stream ( &file );
    stream.setCodec( "UTF8" );

    QString fileData = stream.readAll();

    result = ( fileData == data ) ? Success : Failure;
    if ( !m_genOutput && result == Success && !m_keepOutput ) {
        QFile::remove( outputFilename );
        return Success;
    }
  } else if (!m_genOutput) {
    fprintf(stderr, "Error reading file %s\n", absFilename.toLatin1().constData());
    result = Failure;
  }

  // generate result file
  OutputObject::createMissingDirs( outputFilename );
  QFile file2(outputFilename);
  if (!file2.open(QFile::WriteOnly)) {
    fprintf(stderr,"Error writing to file %s\n",outputFilename.toLatin1().constData());
    exit(1);
  }

  QTextStream stream2(&file2);
  stream2.setCodec( "UTF8" );
  stream2 << data;
  if ( m_genOutput )
    printf("Generated %s\n", outputFilename.toLatin1().constData());

  return result;
}

void RegressionTest::rereadConfig()
{
  KConfigGroup g = m_baseConfig->group("Kate Document Defaults");
  m_part->config()->readConfig(g);
  g = m_baseConfig->group("Kate View Defaults");
  m_view->config()->readConfig(g);
}

bool RegressionTest::reportResult(CheckResult result, const QString & description, bool *newfail)
{
  if ( result == Ignored ) {
//     printf("IGNORED: ");
//     printDescription( description );
    return true; // no error
  } else {
    return reportResult( result == Success, description, newfail );
  }
}

bool RegressionTest::reportResult(bool passed, const QString & description, bool *newfail)
{
  if (newfail) *newfail = false;

  if (m_genOutput)
    return true;

  QString filename(m_currentTest + '-' + description);
  if (!m_currentCategory.isEmpty())
    filename = m_currentCategory + '/' + filename;

  const bool oldfailed = m_failureComp && m_failureComp->readEntry(filename, 0);
  if (passed) {
    if ( m_known_failures & AllFailure ) {
      printf("PASS (unexpected!)");
      m_passes_fail++;
    } else {
      printf("PASS");
      m_passes_work++;
    }
    if (oldfailed) {
      printf(" (new)");
      m_passes_new++;
    }
    if (m_failureSave)
      m_failureSave->deleteEntry(filename);
  } else {
    if ( m_known_failures & AllFailure ) {
      printf("FAIL (known)");
      m_failures_fail++;
      passed = true; // we knew about it
    } else {
      printf("FAIL");
      m_failures_work++;
    }
    if (!oldfailed && m_failureComp) {
      printf(" (new)");
      m_failures_new++;
      if (newfail) *newfail = true;
    }
    if (m_failureSave)
      m_failureSave->writeEntry(filename, 1);
  }
  printf(": ");

  printDescription( description );
  return passed;
}

void RegressionTest::printDescription(const QString& description)
{
  if (!m_currentCategory.isEmpty())
    printf("%s/", m_currentCategory.toLatin1().constData());

  printf("%s", m_currentTest.toLatin1().constData());

  if (!description.isEmpty()) {
    QString desc = description;
    desc.replace( '\n', ' ' );
    printf(" [%s]", desc.toLatin1().constData());
  }

  printf("\n");
  fflush(stdout);
}

void RegressionTest::printSummary()
{
  QTextStream out(stdout, QIODevice::WriteOnly);

  out << endl;
  out << "Tests completed." << endl;
  out << "Total:    " << m_passes_work + m_passes_fail +
      m_failures_work + m_failures_fail + m_errors << endl;

  //BEGIN Passes
  out << "Passes:   " << m_passes_work;
  if (m_passes_fail > 0)
    out << QString(" (%1 unexpected passes)").arg(m_passes_fail);
  if (m_passes_new > 0)
    out << QString(" (%1 new since %2)").arg(m_passes_new).arg(m_failureComp->name());
  out << endl;
  //END Passes

  //BEGIN Failures
  out << "Failures: " << m_failures_work;
  if (m_failures_fail > 0)
    out << QString(" (%1 expected failures)").arg(m_failures_fail);
  if (m_failures_new > 0)
    out << QString(" (%d new since %s)").arg(m_failures_new).arg(m_failureComp->name());
  out << endl;
  //END Failures

  if (m_errors > 0)
    out << "Errors:   " << m_errors << endl;

  //BEGIN html
  QFile list(m_outputDir + "/links.html");
  list.open(QFile::WriteOnly | QFile::Append);

  QTextStream ts(&list);
  ts << QString("<hr>%1 failures. (%2 expected failures)")
      .arg(m_failures_work)
      .arg(m_failures_fail);
  if (m_failures_new > 0) {
    ts << QString(" <span style=\"color:red;font-weight:bold\">(%1 new failures since %2)</span>")
        .arg(m_failures_new)
        .arg(m_failureComp->name());
  }
  if (m_passes_new > 0) {
    ts << QString(" <p style=\"color:green;font-weight:bold\">%1 new passes since %2</p>")
        .arg(m_passes_new)
        .arg(m_failureComp->name());
  }
  list.close();
  //END html
}

void RegressionTest::slotOpenURL(const KUrl &url, const KParts::OpenUrlArguments & args, const KParts::BrowserArguments&)
{
  m_part->setArguments(args);
  m_part->openUrl(url);
}

bool RegressionTest::svnIgnored( const QString &filename )
{
  QFileInfo fi( filename );
  QString ignoreFilename = fi.path() + "/svnignore";
  QFile ignoreFile(ignoreFilename);
  if (!ignoreFile.open(QFile::ReadOnly))
    return false;

  QTextStream ignoreStream(&ignoreFile);
  QString line;
  while (!(line = ignoreStream.readLine()).isNull()) {
    if ( line == fi.fileName() )
      return true;
  }
  ignoreFile.close();
  return false;
}

void RegressionTest::resizeTopLevelWidget( int w, int h )
{
  toplevel->resize( w, h );
  // Since we're not visible, this doesn't have an immediate effect, QWidget posts the event
  QApplication::sendPostedEvents( 0, QEvent::Resize );
}



KateTestApp::KateTestApp(KCmdLineArgs *args, const QString& baseDir, int testcaseIndex)
  : KApplication()
  , m_args(args)
  , m_cfg("testkateregressionrc", KConfig::SimpleConfig)
  , m_baseDir(baseDir)
  , m_testcaseIndex(testcaseIndex)
{
  // FIXME: Any analogous call for dbus?
  //   a.disableAutoDcopRegistration();
  setStyle("windows");
  KConfigGroup group = m_cfg.group("Kate Document Defaults");
  group.writeEntry("Basic Config Flags",
                   KateDocumentConfig::cfBackspaceIndents
//                  | KateDocumentConfig::cfWordWrap
//                  | KateDocumentConfig::cfRemoveSpaces
                   | KateDocumentConfig::cfWrapCursor
//                  | KateDocumentConfig::cfAutoBrackets
//                  | KateDocumentConfig::cfTabIndentsMode
//                  | KateDocumentConfig::cfOvr
                   | KateDocumentConfig::cfKeepExtraSpaces
                   | KateDocumentConfig::cfTabIndents
                   | KateDocumentConfig::cfShowTabs
//                  | KateDocumentConfig::cfSpaceIndent TODO lookup replacement
                   | KateDocumentConfig::cfSmartHome
                   | KateDocumentConfig::cfTabInsertsTab
//                  | KateDocumentConfig::cfReplaceTabsDyn
//                  | KateDocumentConfig::cfRemoveTrailingDyn
//                  | KateDocumentConfig::cfDoxygenAutoTyping // TODO lookup replacement
//                  | KateDocumentConfig::cfMixedIndent
                   | KateDocumentConfig::cfIndentPastedText
                   );
  m_cfg.sync();

//   {
//     KConfig dc( "kdebugrc", KConfig::SimpleConfig );
//     // FIXME adapt to kate
//     static int areas[] = { 1000, 13000, 13001, 13002, 13010,
//     13020, 13025, 13030, 13033, 13035,
//     13040, 13050, 13051, 13070, 7000, 7006, 170,
//     171, 7101, 7002, 7019, 7027, 7014,
//     7001, 7011, 6070, 6080, 6090, 0};
//     int channel = args->isSet( "debug" ) ? 2 : 4;
//     for ( int i = 0; areas[i]; ++i ) {
//       KConfigGroup group = dc.group( QString::number( areas[i] ) );
//       group.writeEntry( "InfoOutput", channel );
//     }
//     dc.sync();
//
//     kClearDebugConfig();
//   }

  // create widgets
  toplevel = new KMainWindow();
  m_document = new KateDocument(true, false, false, toplevel);
  m_document->setObjectName("testkate");

  toplevel->setCentralWidget( m_document->widget() );

  if (args->isSet("show"))
    toplevel->show();

  // we're not interested
  toplevel->statusBar()->hide();

  if (false && qgetenv("KDE_DEBUG").isEmpty()) {
    // set ulimits
    rlimit vmem_limit = { 256*1024*1024, RLIM_INFINITY };   // 256Mb Memory should suffice
#if defined(RLIMIT_AS)
    setrlimit(RLIMIT_AS, &vmem_limit);
#endif
#if defined(RLIMIT_DATA)
    setrlimit(RLIMIT_DATA, &vmem_limit);
#endif
    rlimit stack_limit = { 8*1024*1024, RLIM_INFINITY };    // 8Mb Memory should suffice
    setrlimit(RLIMIT_STACK, &stack_limit);
  }

  // run the tests
  m_regressionTest = new RegressionTest(m_document, &m_cfg, baseDir, args);

  {
    QString failureSnapshot = args->getOption("cmp-failures");
    if (failureSnapshot.isEmpty())
      failureSnapshot = findMostRecentFailureSnapshot();
    if (!failureSnapshot.isEmpty())
      m_regressionTest->setFailureSnapshotConfig(new KConfig(failureSnapshotPrefix + failureSnapshot,
                                                             KConfig::SimpleConfig),
                                                 failureSnapshot);
  }

  if (args->isSet("save-failures")) {
    QString failureSaver = args->getOption("save-failures");
    m_regressionTest->setFailureSnapshotSaver(new KConfig(failureSnapshotPrefix + failureSaver,
                                                          KConfig::SimpleConfig),
                                              failureSaver);
  }

  QTimer::singleShot(0, this, SLOT(runTests()));
}

KateTestApp::~KateTestApp()
{
  delete m_document;
  m_document = 0;

  Q_ASSERT(m_regressionTest == 0);
}

bool KateTestApp::allTestsSucceeded()
{
  return m_regressionTest->allTestsSucceeded();
}

void KateTestApp::runTests()
{
  bool result = false;
  QStringList tests = m_args->getOptionList("test");
  // merge testcases specified on command line
  for (; m_testcaseIndex < m_args->count(); m_testcaseIndex++)
    tests << m_args->arg(m_testcaseIndex);
  if (tests.count() > 0) {
    foreach (const QString &test, tests) {
      result = m_regressionTest->runTests(test, true);
      if (!result) break;
    }
  } else {
    result = m_regressionTest->runTests();
  }

  if (result) {
    if (m_args->isSet("genoutput")) {
      printf("\nOutput generation completed.\n");
    } else {
      m_regressionTest->printSummary();
    }
  }
}

#include "test_regression.moc"
// kate: space-indent on; indent-width 2; replace-tabs on;
