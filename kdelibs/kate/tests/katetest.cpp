/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#include "katetest.h"
#include "katetest.moc"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/modificationinterface.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/annotationinterface.h>

#include <kio/netaccess.h>

#include <kdeversion.h>
#include <kaboutapplicationdialog.h>
#include <kicon.h>
#include <kencodingfiledialog.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kapplication.h>
#include <klocale.h>
#include <kurl.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kshortcutsdialog.h>
#include <kedittoolbar.h>
#include <kparts/event.h>
#include <kmenubar.h>
#include <ksqueezedtextlabel.h>
#include <kstringhandler.h>
#include <krecentfilesaction.h>
#include <kactioncollection.h>

#include <QtGui/QStackedWidget>
#include <QtGui/QPainter>
#include <QtGui/QLabel>
#include <QtGui/QCursor>
#include <QtGui/QMenu>
#include <QtGui/QPixmap>
#include <QtCore/QTimer>
#include <QtCore/QTextCodec>
#include <QtGui/QLayout>
//Added by qt3to4:
#include <QtGui/QKeyEvent>
#include <QtGui/QBoxLayout>
#include <QtCore/QTextStream>

#include <kxmlguifactory.h>

#include "arbitraryhighlighttest.h"
#include "codecompletiontestmodel.h"
#include "annotationmodeltest.h"

// StatusBar field IDs
#define KWRITE_ID_GEN 1

QList<KTextEditor::Document*> KWrite::docList;
QList<KWrite*> KWrite::winList;

KWrite::KWrite (KTextEditor::Document *doc)
    : m_view(0),
      m_recentFiles(0),
      m_paShowPath(0),
      m_paShowStatusBar(0)
{
  setMinimumSize(200,200);

  if ( !doc )
  {
    KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();

    if ( !editor )
    {
      KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
                                    "please check your KDE installation."));
      kapp->exit(1);
    }

    doc = editor->createDocument(0);

    // enable the modified on disk warning dialogs if any
    if (qobject_cast<KTextEditor::ModificationInterface *>(doc))
      qobject_cast<KTextEditor::ModificationInterface *>(doc)->setModifiedOnDiskWarning (true);

    docList.append(doc);
  }

  new ArbitraryHighlightTest(doc);

  m_view = qobject_cast<KTextEditor::View*>(doc->createView (this));

  new CodeCompletionTestModel(m_view);

  // Test for the annotation interface
  AnnotationModelTest* annomodel = new AnnotationModelTest();
  if( qobject_cast<KTextEditor::AnnotationInterface*>(doc) )
    qobject_cast<KTextEditor::AnnotationInterface*>(doc)->setAnnotationModel(annomodel);

  if( qobject_cast<KTextEditor::AnnotationViewInterface*>(m_view) )
    qobject_cast<KTextEditor::AnnotationViewInterface*>(m_view)->setAnnotationBorderVisible(true);

  connect(m_view, SIGNAL(annotationContextMenuAboutToShow( KTextEditor::View*, QMenu*, int )), annomodel, SLOT(annotationContextMenuAboutToShow( KTextEditor::View*, QMenu*, int )));
  connect(m_view, SIGNAL(annotationActivated( KTextEditor::View*, int )), annomodel, SLOT(annotationActivated( KTextEditor::View*, int )));

  setCentralWidget(m_view);

  setupActions();
  setupStatusBar();

  // signals for the statusbar
  connect(m_view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)), this, SLOT(cursorPositionChanged(KTextEditor::View *)));
  connect(m_view, SIGNAL(viewModeChanged(KTextEditor::View *)), this, SLOT(viewModeChanged(KTextEditor::View *)));
  connect(m_view, SIGNAL(selectionChanged (KTextEditor::View *)), this, SLOT(selectionChanged (KTextEditor::View *)));
  connect(m_view, SIGNAL(informationMessage (KTextEditor::View *, const QString &)), this, SLOT(informationMessage (KTextEditor::View *, const QString &)));
  connect(m_view->document(), SIGNAL(modifiedChanged(KTextEditor::Document *)), this, SLOT(modifiedChanged()));
  if (KTextEditor::ModificationInterface* modiface = qobject_cast<KTextEditor::ModificationInterface*>(m_view->document()))
    connect(m_view->document(), SIGNAL(modifiedOnDisk(KTextEditor::Document*, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason)), this, SLOT(modifiedChanged()) );
  else
    kWarning() << "Modification interface not supported.";
  connect(m_view->document(), SIGNAL(documentNameChanged(KTextEditor::Document *)), this, SLOT(documentNameChanged()));

  setAcceptDrops(true);
  connect(m_view,SIGNAL(dropEventPass(QDropEvent *)),this,SLOT(slotDropEvent(QDropEvent *)));

  KGlobal::dirs()->addResourceDir( "data", QDir::currentPath() );
  setXMLFile( "katetest.rc" );
  createShellGUI( true );
  guiFactory()->addClient( m_view );

  // install a working kate part popup dialog thingy
  m_view->setContextMenu ((QMenu*)(factory()->container("ktexteditor_popup", this)) );

  // call it as last thing, must be sure everything is already set up ;)
  setAutoSaveSettings ("MainWindow Settings");

  // init with more useful size, stolen from konq :)
  if ( !initialGeometrySet() && !KGlobal::config()->hasGroup("MainWindow Settings"))
    resize( 700, 480 );

  readConfig ();

  winList.append (this);

  updateStatus ();
  show ();

  //doc->setText("{ test { test { test { test { test } test } test } test } test } test } blah");
}

KWrite::~KWrite()
{
  winList.removeAll(this);

  if (m_view->document()->views().count() == 1)
  {
    docList.removeAll(m_view->document());
    delete m_view->document();
  }

  KGlobal::config()->sync ();
}

void KWrite::setupActions()
{
    actionCollection()->addAction( KStandardAction::Close, "file_close", this, SLOT(slotFlush()) )
        ->setWhatsThis(i18n("Use this to close the current document"));

    // setup File menu
    actionCollection()->addAction( KStandardAction::Print, this, SLOT(printDlg()) )
        ->setWhatsThis(i18n("Use this command to print the current document"));
    actionCollection()->addAction( KStandardAction::New, "file_new", this, SLOT(slotNew()) )
        ->setWhatsThis(i18n("Use this command to create a new document"));
    actionCollection()->addAction( KStandardAction::Open, "file_open", this, SLOT(slotOpen()) )
        ->setWhatsThis(i18n("Use this command to open an existing document for editing"));

    m_recentFiles = KStandardAction::openRecent(this, SLOT(slotOpen(const KUrl&)), this );
    actionCollection()->addAction(m_recentFiles->objectName(), m_recentFiles);
    m_recentFiles->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

    QAction *a = new KAction(KIcon("window-new"), i18n("&New Window"), this);
    actionCollection()->addAction("view_new_view", a);
    connect(a, SIGNAL(triggered(bool)), SLOT(newView()));
    a->setWhatsThis(i18n("Create another view containing the current document"));

    a=new KAction(i18n("Choose Editor..."), this);
    actionCollection()->addAction("settings_choose_editor", a);
    connect(a, SIGNAL(triggered(bool)), SLOT(changeEditor()));
    a->setWhatsThis(i18n("Override the system-wide setting for the default editing component"));

    actionCollection()->addAction( KStandardAction::Quit, this, SLOT(close()) )
        ->setWhatsThis(i18n("Close the current document view"));

    // setup Settings menu
    setStandardToolBarMenuEnabled(true);

    m_paShowStatusBar = KStandardAction::showStatusbar(this, SLOT(toggleStatusBar()), this);
    actionCollection()->addAction("settings_show_statusbar", m_paShowStatusBar);
    m_paShowStatusBar->setWhatsThis(i18n("Use this command to show or hide the view's statusbar"));

    m_paShowPath = new KToggleAction(i18n("Sho&w Path"), this);
    actionCollection()->addAction("set_showPath", m_paShowPath);
    m_paShowPath->setShortcuts(KShortcut());
    m_paShowPath->setWhatsThis(i18n("Show the complete document path in the window caption"));
    connect(m_paShowPath, SIGNAL(triggered()), this, SLOT(documentNameChanged()));

    a=actionCollection()->addAction( KStandardAction::KeyBindings, this, SLOT(editKeys()));
    a->setWhatsThis(i18n("Configure the application's keyboard shortcut assignments."));

    a=actionCollection()->addAction( KStandardAction::ConfigureToolbars, "set_configure_toolbars",
                                     this, SLOT(editToolbars()) );
    a->setWhatsThis(i18n("Configure which items should appear in the toolbar(s)."));

    a=new KAction(this);
    actionCollection()->addAction("help_about_editor", a);
    a->setText(i18n("&About Editor Component"));
    connect(a, SIGNAL(triggered()), this, SLOT(aboutEditor()));
}

void KWrite::setupStatusBar()
{
  // statusbar stuff
  m_lineColLabel = new QLabel( statusBar() );
  statusBar()->addWidget( m_lineColLabel, 0 );
  m_lineColLabel->setAlignment( Qt::AlignCenter );

  m_modifiedLabel = new QLabel( statusBar() );
  m_modifiedLabel->setFixedSize( 16, 16 );
  statusBar()->addWidget( m_modifiedLabel, 0 );
  m_modifiedLabel->setAlignment( Qt::AlignCenter );

  m_insertModeLabel = new QLabel( i18n(" INS "), statusBar() );
  statusBar()->addWidget( m_insertModeLabel, 0 );
  m_insertModeLabel->setAlignment( Qt::AlignCenter );

  m_selectModeLabel = new QLabel( i18n(" LINE "), statusBar() );
  statusBar()->addWidget( m_selectModeLabel, 0 );
  m_selectModeLabel->setAlignment( Qt::AlignCenter );

  m_fileNameLabel=new KSqueezedTextLabel( statusBar() );
  statusBar()->addPermanentWidget( m_fileNameLabel, 1 );
  m_fileNameLabel->setMinimumSize( 0, 0 );
  m_fileNameLabel->setSizePolicy(QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed ));
  m_fileNameLabel->setAlignment( /*Qt::AlignRight*/Qt::AlignLeft );

//   m_modDiscPm = KIcon("drive-harddisk").pixmap(16);
//   m_modmodPm = KIcon("modmod").pixmap(16); // FIXME: is it still required? icon is broken.
}

// load on url
void KWrite::loadURL(const KUrl &url)
{
  m_view->document()->openUrl(url);
}

// is closing the window wanted by user ?
bool KWrite::queryClose()
{
  if (m_view->document()->views().count() > 1)
    return true;

  if (m_view->document()->queryClose())
  {
    writeConfig();

    return true;
  }

  return false;
}

void KWrite::changeEditor()
{
  KWriteEditorChooser choose(this);
  choose.exec();
}

void KWrite::slotFlush ()
{
   m_view->document()->closeUrl();
}

void KWrite::slotNew()
{
  if (m_view->document()->isModified() || !m_view->document()->url().isEmpty())
    new KWrite();
  else
    m_view->document()->openUrl(KUrl());
}

void KWrite::slotOpen()
{
	KEncodingFileDialog::Result r=KEncodingFileDialog::getOpenUrlsAndEncoding(m_view->document()->encoding(), m_view->document()->url().url(),QString(),this,i18n("Open File"));

  for (KUrl::List::Iterator i=r.URLs.begin(); i != r.URLs.end(); ++i)
  {
    encoding = r.encoding;
    slotOpen ( *i );
  }
}

void KWrite::slotOpen( const KUrl& url )
{
  if (url.isEmpty()) return;

  if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, this))
  {
    KMessageBox::error (this, i18n("The given file could not be read, check if it exists or if it is readable for the current user."));
    return;
  }

  if (m_view->document()->isModified() || !m_view->document()->url().isEmpty())
  {
    KWrite *t = new KWrite();
    t->m_view->document()->setEncoding(encoding);
    t->loadURL(url);
  }
  else
  {
    m_view->document()->setEncoding(encoding);
    loadURL(url);
  }
}

void KWrite::slotFileNameChanged()
{
  if ( ! m_view->document()->url().isEmpty() )
    m_recentFiles->addUrl( m_view->document()->url() );
}

void KWrite::newView()
{
  new KWrite(m_view->document());
}

void KWrite::toggleStatusBar()
{
  if( m_paShowStatusBar->isChecked() )
    statusBar()->show();
  else
    statusBar()->hide();
}

void KWrite::editKeys()
{
  KShortcutsDialog dlg;
  dlg.addCollection(actionCollection());
  if( m_view )
    dlg.addCollection(m_view->actionCollection());
  dlg.configure();
}

void KWrite::editToolbars()
{
  KEditToolBar *dlg = new KEditToolBar(guiFactory());

  if (dlg->exec())
  {
    KParts::GUIActivateEvent ev1( false );
    QApplication::sendEvent( m_view, &ev1 );
    guiFactory()->removeClient( m_view );
    createShellGUI( false );
    createShellGUI( true );
    guiFactory()->addClient( m_view );
    KParts::GUIActivateEvent ev2( true );
    QApplication::sendEvent( m_view, &ev2 );
  }

  delete dlg;
}

void KWrite::dragEnterEvent( QDragEnterEvent *event )
{
  event->setAccepted(KUrl::List::canDecode(event->mimeData()));
}

void KWrite::dropEvent( QDropEvent *event )
{
  slotDropEvent(event);
}

void KWrite::slotDropEvent( QDropEvent *event )
{
  const KUrl::List textlist = KUrl::List::fromMimeData(event->mimeData());

  if (textlist.isEmpty())
    return;

  for (KUrl::List::ConstIterator i=textlist.begin(); i != textlist.end(); ++i)
    slotOpen (*i);
}

void KWrite::slotEnableActions( bool enable )
{
  QList<QAction *> actions = actionCollection()->actions();
  QList<QAction *>::ConstIterator it = actions.constBegin();
  QList<QAction *>::ConstIterator end = actions.constEnd();

  for (; it != end; ++it )
      (*it)->setEnabled( enable );

  actions = m_view->actionCollection()->actions();
  it = actions.constBegin();
  end = actions.constEnd();

  for (; it != end; ++it )
      (*it)->setEnabled( enable );
}

//common config
void KWrite::readConfig(const KConfigGroup &config)
{
  m_paShowStatusBar->setChecked( config.readEntry("ShowStatusBar", false) );
  m_paShowPath->setChecked( config.readEntry("ShowPath", false) );

  m_recentFiles->loadEntries(KConfigGroup(config.config(), "Recent Files"));

  m_view->document()->editor()->readConfig();

  if( m_paShowStatusBar->isChecked() )
    statusBar()->show();
  else
    statusBar()->hide();
}

void KWrite::writeConfig(KConfigGroup &config)
{
  config.writeEntry("ShowStatusBar",m_paShowStatusBar->isChecked());
  config.writeEntry("ShowPath",m_paShowPath->isChecked());

  config.changeGroup("Recent Files");
  m_recentFiles->saveEntries(config);

  m_view->document()->editor()->writeConfig(dynamic_cast<KConfig*>(config.config()));

  config.sync ();
}

//config file
void KWrite::readConfig()
{
  KSharedConfig::Ptr config = KGlobal::config();
  readConfig(config->group("General Options"));
}

void KWrite::writeConfig()
{
  KSharedConfig::Ptr config = KGlobal::config();
  KConfigGroup go(config, "General Options");
  writeConfig(go);
}

// session management
void KWrite::restore(KConfig *config, int n)
{
  readPropertiesInternal(config, n);
}

void KWrite::readProperties(const KConfigGroup &config)
{
  readConfig(config);

  if (KTextEditor::SessionConfigInterface *iface = qobject_cast<KTextEditor::SessionConfigInterface *>(m_view))
    iface->readSessionConfig(KConfigGroup(config.config(), "SOMEGROUP"));
}

void KWrite::saveProperties(KConfigGroup & config)
{
  writeConfig();
  config.writeEntry("DocumentNumber",docList.indexOf(m_view->document()) + 1);

  KConfigGroup cg(config.config(), "SOMEGROUP");
  if (KTextEditor::SessionConfigInterface *iface = qobject_cast<KTextEditor::SessionConfigInterface *>(m_view))
    iface->writeSessionConfig(cg);
}

void KWrite::saveGlobalProperties(KConfig *_config) //save documents
{
  KConfigGroup config(_config, "Number");
  config.writeEntry("NumberOfDocuments",docList.count());

  for (int z = 1; z <= docList.count(); z++)
  {
     QString buf = QString("Document %1").arg(z);
     KConfigGroup newGroup(_config,buf);

     KTextEditor::Document *doc = docList.at(z - 1);

     if (KTextEditor::SessionConfigInterface *iface = qobject_cast<KTextEditor::SessionConfigInterface *>(doc))
       iface->writeSessionConfig(newGroup);
  }

  for (int z = 1; z <= winList.count(); z++)
  {
     QString buf = QString("Window %1").arg(z);
     KConfigGroup newGroup(_config,buf);

     newGroup.writeEntry("DocumentNumber",docList.indexOf(winList.at(z-1)->view()->document()) + 1);
  }
}

//restore session
void KWrite::restore()
{
  KConfig *config = kapp->sessionConfig();

  if (!config)
    return;

  KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();

  if ( !editor )
  {
    KMessageBox::error(0, i18n("A KDE text-editor component could not be found;\n"
                                  "please check your KDE installation."));
    kapp->exit(1);
  }

  int docs, windows;
  QString buf;
  KTextEditor::Document *doc;
  KWrite *t;

  KConfigGroup cg(config, "Number");
  docs = cg.readEntry("NumberOfDocuments", 0);
  windows = cg.readEntry("NumberOfWindows", 0);

  for (int z = 1; z <= docs; z++)
  {
     buf = QString("Document %1").arg(z);
     KConfigGroup newGroup(config,buf);
     doc=editor->createDocument(0);

     if (KTextEditor::SessionConfigInterface *iface = qobject_cast<KTextEditor::SessionConfigInterface *>(doc))
       iface->readSessionConfig(newGroup);
     docList.append(doc);
  }

  for (int z = 1; z <= windows; z++)
  {
    buf = QString("Window %1").arg(z);
    KConfigGroup cg( config, buf);
    t = new KWrite(docList.at(cg.readEntry("DocumentNumber", 0) - 1));
    t->restore(config,z);
  }
}

void KWrite::aboutEditor()
{
  KAboutApplicationDialog *ad = new KAboutApplicationDialog (m_view->document()->editor()->aboutData(), this);

  ad->exec();

  delete ad;
}

void KWrite::updateStatus ()
{
  viewModeChanged (m_view);
  cursorPositionChanged (m_view);
  selectionChanged (m_view);
  modifiedChanged ();
  documentNameChanged ();
}

void KWrite::viewModeChanged ( KTextEditor::View *view )
{
  m_insertModeLabel->setText( view->viewMode() );
}

void KWrite::cursorPositionChanged ( KTextEditor::View *view )
{
  KTextEditor::Cursor position (view->cursorPositionVirtual());

  m_lineColLabel->setText(
    i18n(" Line: %1 Col: %2 ", KGlobal::locale()->formatNumber(position.line()+1, 0),
                               KGlobal::locale()->formatNumber(position.column()+1, 0)) );
}

void KWrite::selectionChanged (KTextEditor::View *view)
{
  m_selectModeLabel->setText( view->blockSelection() ? i18n(" BLOCK ") : i18n(" LINE ") );
}

void KWrite::informationMessage (KTextEditor::View *, const QString &message)
{
  m_fileNameLabel->setText( message );

  // timer to reset this after 4 seconds
  QTimer::singleShot(4000, this, SLOT(documentNameChanged()));
}

void KWrite::modifiedChanged()
{
    bool mod = m_view->document()->isModified();

    if (mod && m_modPm.isNull()) {
        m_modPm = KIcon("document-properties").pixmap(16);
    }

   /* const KateDocumentInfo *info
      = KateDocManager::self()->documentInfo ( m_view->document() );
*/
    bool modOnHD = false; //info && info->modifiedOnDisc;

    m_modifiedLabel->setPixmap(
        mod ? m_modPm : QPixmap()
          /*info && modOnHD ?
            m_modmodPm :
            m_modPm :
          info && modOnHD ?
            m_modDiscPm :
        QPixmap()*/
        );
}

void KWrite::documentNameChanged ()
{
  m_fileNameLabel->setText( KStringHandler::lsqueeze(m_view->document()->documentName (), 64) );

  if (m_view->document()->url().isEmpty()) {
    setCaption(i18n("Untitled"),m_view->document()->isModified());
  }
  else
  {
    QString c;
    if (!m_paShowPath->isChecked())
    {
      c = m_view->document()->url().fileName();

      //File name shouldn't be too long - Maciek
      if (c.length() > 64)
        c = c.left(64) + "...";
    }
    else
    {
      c = m_view->document()->url().prettyUrl();

      //File name shouldn't be too long - Maciek
      if (c.length() > 64)
        c = "..." + c.right(64);
    }

    setCaption (c, m_view->document()->isModified());
  }
}

extern "C" KDE_EXPORT int main(int argc, char **argv)
{
  KCmdLineOptions options;
  options.add("stdin", ki18n("Read the contents of stdin"));
  options.add("encoding <argument>", ki18n("Set encoding for the file to open"));
  options.add("line <argument>", ki18n("Navigate to this line"));
  options.add("column <argument>", ki18n("Navigate to this column"));
  options.add("+[URL]", ki18n("Document to open"));

  // here we go, construct the KWrite version
  QString kWriteVersion  = QString ("%1.%2.%3").arg(KDE::versionMajor() + 1).arg(KDE::versionMinor()).arg(KDE::versionRelease());

  KAboutData aboutData ( "kwrite", "kate",
                         ki18n("KWrite"),
                         kWriteVersion.toLatin1(),
                         ki18n( "KWrite - Text Editor" ), KAboutData::License_LGPL_V2,
                         ki18n( "(c) 2000-2005 The Kate Authors" ), KLocalizedString(), "http://kate.kde.org" );

  aboutData.addAuthor (ki18n("Christoph Cullmann"), ki18n("Maintainer"), "cullmann@kde.org", "http://www.babylon2k.de");
  aboutData.addAuthor (ki18n("Anders Lund"), ki18n("Core Developer"), "anders@alweb.dk", "http://www.alweb.dk");
  aboutData.addAuthor (ki18n("Joseph Wenninger"), ki18n("Core Developer"), "jowenn@kde.org","http://stud3.tuwien.ac.at/~e9925371");
  aboutData.addAuthor (ki18n("Hamish Rodda"),ki18n("Core Developer"), "rodda@kde.org");
  aboutData.addAuthor (ki18n("Waldo Bastian"), ki18n( "The cool buffersystem" ), "bastian@kde.org" );
  aboutData.addAuthor (ki18n("Charles Samuels"), ki18n("The Editing Commands"), "charles@kde.org");
  aboutData.addAuthor (ki18n("Matt Newell"), ki18n("Testing, ..."), "newellm@proaxis.com");
  aboutData.addAuthor (ki18n("Michael Bartl"), ki18n("Former Core Developer"), "michael.bartl1@chello.at");
  aboutData.addAuthor (ki18n("Michael McCallum"), ki18n("Core Developer"), "gholam@xtra.co.nz");
  aboutData.addAuthor (ki18n("Jochen Wilhemly"), ki18n( "KWrite Author" ), "digisnap@cs.tu-berlin.de" );
  aboutData.addAuthor (ki18n("Michael Koch"),ki18n("KWrite port to KParts"), "koch@kde.org");
  aboutData.addAuthor (ki18n("Christian Gebauer"), KLocalizedString(), "gebauer@kde.org" );
  aboutData.addAuthor (ki18n("Simon Hausmann"), KLocalizedString(), "hausmann@kde.org" );
  aboutData.addAuthor (ki18n("Glen Parker"),ki18n("KWrite Undo History, Kspell integration"), "glenebob@nwlink.com");
  aboutData.addAuthor (ki18n("Scott Manson"),ki18n("KWrite XML Syntax highlighting support"), "sdmanson@alltel.net");
  aboutData.addAuthor (ki18n("John Firebaugh"),ki18n("Patches and more"), "jfirebaugh@kde.org");

  aboutData.addCredit (ki18n("Matteo Merli"),ki18n("Highlighting for RPM Spec-Files, Perl, Diff and more"), "merlim@libero.it");
  aboutData.addCredit (ki18n("Rocky Scaletta"),ki18n("Highlighting for VHDL"), "rocky@purdue.edu");
  aboutData.addCredit (ki18n("Yury Lebedev"),ki18n("Highlighting for SQL"),"");
  aboutData.addCredit (ki18n("Chris Ross"),ki18n("Highlighting for Ferite"),"");
  aboutData.addCredit (ki18n("Nick Roux"),ki18n("Highlighting for ILERPG"),"");
  aboutData.addCredit (ki18n("Carsten Niehaus"), ki18n("Highlighting for LaTeX"),"");
  aboutData.addCredit (ki18n("Per Wigren"), ki18n("Highlighting for Makefiles, Python"),"");
  aboutData.addCredit (ki18n("Jan Fritz"), ki18n("Highlighting for Python"),"");
  aboutData.addCredit (ki18n("Daniel Naber"));
  aboutData.addCredit (ki18n("Roland Pabel"),ki18n("Highlighting for Scheme"),"");
  aboutData.addCredit (ki18n("Cristi Dumitrescu"),ki18n("PHP Keyword/Datatype list"),"");
  aboutData.addCredit (ki18n("Carsten Pfeiffer"), ki18n("Very nice help"), "");
  aboutData.addCredit (ki18n("All people who have contributed and I have forgotten to mention"));

  aboutData.setTranslator(ki18nc("NAME OF TRANSLATORS","Your names"), ki18nc("EMAIL OF TRANSLATORS","Your emails"));

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication a;

  KGlobal::locale()->insertCatalog("katepart4");
#if 0
  DCOPClient *client = KApplication::dcopClient();
  if (!client->isRegistered())
  {
    client->attach();
    client->registerAs("kwrite");
  }
#endif
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if (qApp->isSessionRestored())
  {
    KWrite::restore();
  }
  else
  {
    bool nav = false;
    int line = 0, column = 0;

    QTextCodec *codec = args->isSet("encoding") ? QTextCodec::codecForName(args->getOption("encoding").toUtf8()) : 0;

    if (args->isSet ("line"))
    {
      line = args->getOption ("line").toInt();
      nav = true;
    }

    if (args->isSet ("column"))
    {
      column = args->getOption ("column").toInt();
      nav = true;
    }

    if ( args->count() == 0 )
    {
        KWrite *t = new KWrite;

        if( args->isSet( "stdin" ) )
        {
          QTextStream input(stdin);

          // set chosen codec
          if (codec)
            input.setCodec (codec);

          QString line;
          QString text;

          do
          {
            line = input.readLine();
            text.append( line + '\n' );
          } while( !line.isNull() );


          KTextEditor::Document *doc = t->view()->document();
          if( doc )
              doc->setText( text );
        }

        if (nav && t->view())
          t->view()->setCursorPosition (KTextEditor::Cursor (line, column));
    }
    else
    {
      for ( int z = 0; z < args->count(); z++ )
      {
        KWrite *t = new KWrite();

        // this file is no local dir, open it, else warn
        bool noDir = !args->url(z).isLocalFile() || !QDir (args->url(z).toLocalFile()).exists();

        if (noDir)
        {
//          if (Kate::document (t->view()->document()))
  //          KTextEditor::Document::setOpenErrorDialogsActivated (false);

          if (codec)
            t->view()->document()->setEncoding(codec->name());

          t->loadURL( args->url( z ) );

    //      if (Kate::document (t->view()->document()))
      //      KTextEditor::Document::setOpenErrorDialogsActivated (true);

          if (nav)
            t->view()->setCursorPosition (KTextEditor::Cursor (line, column));
        }
        else
          KMessageBox::sorry( t, i18n("The file '%1' could not be opened: it is not a normal file, it is a folder.", args->url(z).url()) );
      }
    }
  }

  // no window there, uh, ohh, for example borked session config !!!
  // create at least one !!
  if (KWrite::noWindows())
    new KWrite();

  return a.exec ();
}


KWriteEditorChooser::KWriteEditorChooser(QWidget *parent):
	KDialog(parent)
{
  setCaption( i18n("Choose Editor Component") );
  setButtons( KDialog::Ok | KDialog::Cancel );
  setDefaultButton( KDialog::Cancel );

  QWidget *widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  m_chooser=new KTextEditor::EditorChooser(widget);
  layout->addWidget(m_chooser);
  setMainWidget(widget);
  m_chooser->readAppSetting();
}

KWriteEditorChooser::~KWriteEditorChooser() {
;
}

void KWriteEditorChooser::slotOk() {
	m_chooser->writeAppSetting();
	KDialog::slotButtonClicked( Ok );
}
