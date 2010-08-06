/* This file is part of the KDE libraries
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2006 Dominik Haumann <dhdev@gmx.de>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2009 Michel Ludwig <michel.ludwig@kdemail.net>
   Copyright (C) 2009 Erlend Hamberg <ehamberg@gmail.com>

   Based on work of:
     Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

//BEGIN Includes
#include "katedialogs.h"
#include "katedialogs.moc"

#include "kateautoindent.h"
#include "katebuffer.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "kateviglobal.h"
#include "katevikeyparser.h"
#include "kateschema.h"
#include "katesyntaxdocument.h"
#include "katemodeconfigpage.h"
#include "kateview.h"
#include "katepartpluginmanager.h"
#include "kpluginselector.h"
#include "spellcheck/spellcheck.h"

// auto generated ui files
#include "ui_modonhdwidget.h"
#include "ui_appearanceconfigwidget.h"
#include "ui_cursorconfigwidget.h"
#include "ui_editconfigwidget.h"
#include "ui_indentationconfigwidget.h"
#include "ui_completionconfigtab.h"
#include "ui_opensaveconfigwidget.h"
#include "ui_opensaveconfigadvwidget.h"
#include "ui_viinputmodeconfigwidget.h"
#include "ui_spellcheckconfigwidget.h"

#include <ktexteditor/plugin.h>

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>

#include <kapplication.h>
#include <kcharsets.h>
#include <kcolorbutton.h>
#include <kcolorcombo.h>
#include <kcolordialog.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kshortcutsdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetypechooser.h>
#include <knuminput.h>
#include <kmenu.h>
#include <kprocess.h>
#include <krun.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kpushbutton.h>
#include <kvbox.h>
#include <kactioncollection.h>
#include <kplugininfo.h>
#include <ktabwidget.h>

//#include <knewstuff/knewstuff.h>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtCore/QFile>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtCore/QStringList>
#include <QtGui/QTabWidget>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtGui/QToolButton>
#include <QtGui/QWhatsThis>
#include <QtGui/QKeyEvent>
#include <QtXml/QDomDocument>

// trailing slash is important
#define HLDOWNLOADPATH "http://kate.kde.org/syntax/"

//END

//BEGIN KateConfigPage
KateConfigPage::KateConfigPage ( QWidget *parent, const char * )
  : KTextEditor::ConfigPage (parent)
  , m_changed (false)
{
  connect (this, SIGNAL(changed()), this, SLOT(somethingHasChanged ()));
}

KateConfigPage::~KateConfigPage ()
{
}

void KateConfigPage::slotChanged()
{
  emit changed();
}

void KateConfigPage::somethingHasChanged ()
{
  m_changed = true;
  kDebug (13000) << "TEST: something changed on the config page: " << this;
}
//END KateConfigPage

//BEGIN KateIndentConfigTab
KateIndentConfigTab::KateIndentConfigTab(QWidget *parent)
  : KateConfigPage(parent)
{
  // This will let us have more separation between this page and
  // the KTabWidget edge (ereslibre)
  QVBoxLayout *layout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(this);

  ui = new Ui::IndentationConfigWidget();
  ui->setupUi( newWidget );

  ui->cmbMode->addItems (KateAutoIndent::listModes());

  ui->label->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
  connect(ui->label, SIGNAL(linkActivated(const QString&)), this, SLOT(showWhatsThis(const QString&)));

  // What's This? help can be found in the ui file

  reload ();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(ui->cmbMode, SIGNAL(activated(int)), this, SLOT(slotChanged()));

  connect(ui->chkKeepExtraSpaces, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkIndentPaste, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkBackspaceUnindents, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

  connect(ui->sbIndentWidth, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));

  connect(ui->rbTabAdvances, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->rbTabIndents, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->rbTabSmart, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

  layout->addWidget(newWidget);
  setLayout(layout);
}

KateIndentConfigTab::~KateIndentConfigTab()
{
  delete ui;
}

void KateIndentConfigTab::showWhatsThis(const QString& text)
{
  QWhatsThis::showText(QCursor::pos(), text);
}

void KateIndentConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateDocumentConfig::global()->configStart ();

  uint configFlags = KateDocumentConfig::global()->configFlags();

  configFlags &= ~KateDocumentConfig::cfKeepExtraSpaces;
  configFlags &= ~KateDocumentConfig::cfIndentPastedText;
  configFlags &= ~KateDocumentConfig::cfBackspaceIndents;

  if (ui->chkKeepExtraSpaces->isChecked()) configFlags |= KateDocumentConfig::cfKeepExtraSpaces;
  if (ui->chkIndentPaste->isChecked()) configFlags |= KateDocumentConfig::cfIndentPastedText;
  if (ui->chkBackspaceUnindents->isChecked()) configFlags |= KateDocumentConfig::cfBackspaceIndents;

  KateDocumentConfig::global()->setConfigFlags(configFlags);
  KateDocumentConfig::global()->setIndentationWidth(ui->sbIndentWidth->value());
  KateDocumentConfig::global()->setIndentationMode(KateAutoIndent::modeName(ui->cmbMode->currentIndex()));

  if (ui->rbTabAdvances->isChecked())
    KateDocumentConfig::global()->setTabHandling( KateDocumentConfig::tabInsertsTab );
  else if (ui->rbTabIndents->isChecked())
    KateDocumentConfig::global()->setTabHandling( KateDocumentConfig::tabIndents );
  else
    KateDocumentConfig::global()->setTabHandling( KateDocumentConfig::tabSmart );

  KateDocumentConfig::global()->configEnd ();
}

void KateIndentConfigTab::reload ()
{
  uint configFlags = KateDocumentConfig::global()->configFlags();

  ui->sbIndentWidth->setSuffix(ki18np(" character", " characters"));
  ui->sbIndentWidth->setValue(KateDocumentConfig::global()->indentationWidth());
  ui->chkKeepExtraSpaces->setChecked(configFlags & KateDocumentConfig::cfKeepExtraSpaces);
  ui->chkIndentPaste->setChecked(configFlags & KateDocumentConfig::cfIndentPastedText);
  ui->chkBackspaceUnindents->setChecked(configFlags & KateDocumentConfig::cfBackspaceIndents);

  ui->rbTabAdvances->setChecked( KateDocumentConfig::global()->tabHandling() == KateDocumentConfig::tabInsertsTab );
  ui->rbTabIndents->setChecked( KateDocumentConfig::global()->tabHandling() == KateDocumentConfig::tabIndents );
  ui->rbTabSmart->setChecked( KateDocumentConfig::global()->tabHandling() == KateDocumentConfig::tabSmart );

  ui->cmbMode->setCurrentIndex (KateAutoIndent::modeNumber (KateDocumentConfig::global()->indentationMode()));
}
//END KateIndentConfigTab

//BEGIN KateCompletionConfigTab
KateCompletionConfigTab::KateCompletionConfigTab(QWidget *parent)
  : KateConfigPage(parent)
{
  // This will let us have more separation between this page and
  // the KTabWidget edge (ereslibre)
  QVBoxLayout *layout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(this);

  ui = new Ui::CompletionConfigTab ();
  ui->setupUi( newWidget );

  // What's This? help can be found in the ui file

  reload ();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(ui->chkAutoCompletionEnabled, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->gbWordCompletion, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->minimalWordLength, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));

  layout->addWidget(newWidget);
  setLayout(layout);
}

KateCompletionConfigTab::~KateCompletionConfigTab()
{
  delete ui;
}

void KateCompletionConfigTab::showWhatsThis(const QString& text)
{
  QWhatsThis::showText(QCursor::pos(), text);
}

void KateCompletionConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateViewConfig::global()->setAutomaticCompletionInvocation (ui->chkAutoCompletionEnabled->isChecked());
  KateViewConfig::global()->setWordCompletion (ui->gbWordCompletion->isChecked());
  KateViewConfig::global()->setWordCompletionMinimalWordLength (ui->minimalWordLength->value());
  KateViewConfig::global()->configEnd ();
}

void KateCompletionConfigTab::reload ()
{
  ui->chkAutoCompletionEnabled->setChecked( KateViewConfig::global()->automaticCompletionInvocation () );
  ui->gbWordCompletion->setChecked( KateViewConfig::global()->wordCompletion () );
  ui->minimalWordLength->setValue (KateViewConfig::global()->wordCompletionMinimalWordLength ());
}
//END KateCompletionConfigTab

//BEGIN KateViInputModeConfigTab
KateViInputModeConfigTab::KateViInputModeConfigTab(QWidget *parent)
  : KateConfigPage(parent)
{
  // This will let us have more separation between this page and
  // the KTabWidget edge (ereslibre)
  QVBoxLayout *layout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(this);

  ui = new Ui::ViInputModeConfigWidget ();
  ui->setupUi( newWidget );

  // What's This? help can be found in the ui file

  reload ();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(ui->chkViInputModeDefault, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkViCommandsOverride, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkViStatusBarHide, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->tblNormalModeMappings, SIGNAL(cellChanged(int, int)), this, SLOT(slotChanged()));
  connect(ui->btnAddNewNormal, SIGNAL(clicked()), this, SLOT(addNewNormalModeMappingRow()));
  connect(ui->btnRemoveSelectedNormal, SIGNAL(clicked()), this, SLOT(removeSelectedNormalMappingRow()));

  layout->addWidget(newWidget);
  setLayout(layout);
}

KateViInputModeConfigTab::~KateViInputModeConfigTab()
{
  delete ui;
}

void KateViInputModeConfigTab::showWhatsThis(const QString& text)
{
  QWhatsThis::showText(QCursor::pos(), text);
}

void KateViInputModeConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateViewConfig::global()->setViInputMode (ui->chkViInputModeDefault->isChecked());
  KateViewConfig::global()->setViInputModeStealKeys (ui->chkViCommandsOverride->isChecked());
  KateViewConfig::global()->setViInputModeHideStatusBar (ui->chkViStatusBarHide->isChecked());
  KateGlobal::self()->viInputModeGlobal()->clearMappings( NormalMode );
  for ( int i = 0; i < ui->tblNormalModeMappings->rowCount(); i++ ) {
    QTableWidgetItem* from = ui->tblNormalModeMappings->item( i, 0 );
    QTableWidgetItem* to = ui->tblNormalModeMappings->item( i, 1 );

    if ( from && to ) {
      KateGlobal::self()->viInputModeGlobal()->addMapping( NormalMode, from->text(), to->text() );
    }
  }
  KateViewConfig::global()->configEnd ();
}

void KateViInputModeConfigTab::reload ()
{
  ui->chkViInputModeDefault->setChecked( KateViewConfig::global()->viInputMode () );
  ui->chkViCommandsOverride->setChecked( KateViewConfig::global()->viInputModeStealKeys () );
  ui->chkViStatusBarHide->setChecked( KateViewConfig::global()->viInputModeHideStatusBar () );

  ui->chkViCommandsOverride->setEnabled(ui->chkViInputModeDefault->isChecked());
  ui->chkViStatusBarHide->setEnabled(ui->chkViInputModeDefault->isChecked());

  QStringList l = KateGlobal::self()->viInputModeGlobal()->getMappings( NormalMode );
  ui->tblNormalModeMappings->setRowCount( l.size() );

  // make the two columns fill the entire table width
  ui->tblNormalModeMappings->setColumnWidth( 0, ui->tblNormalModeMappings->width()/3 );
  ui->tblNormalModeMappings->horizontalHeader()->setStretchLastSection( true );

  int i = 0;
  foreach( const QString &f, l ) {
    QTableWidgetItem *from
      = new QTableWidgetItem( KateViKeyParser::getInstance()->decodeKeySequence( f ) );
    QString s = KateGlobal::self()->viInputModeGlobal()->getMapping( NormalMode, f );
    QTableWidgetItem *to =
      new QTableWidgetItem( KateViKeyParser::getInstance()->decodeKeySequence( s ) );

    ui->tblNormalModeMappings->setItem(i, 0, from);
    ui->tblNormalModeMappings->setItem(i++, 1, to);
  }
}

void KateViInputModeConfigTab::addNewNormalModeMappingRow()
{
  int rows = ui->tblNormalModeMappings->rowCount();
  ui->tblNormalModeMappings->insertRow( rows );
  ui->tblNormalModeMappings->setCurrentCell( rows, 0 );
  ui->tblNormalModeMappings->editItem( ui->tblNormalModeMappings->currentItem() );
}

void KateViInputModeConfigTab::removeSelectedNormalMappingRow()
{
  QList<QTableWidgetSelectionRange> l = ui->tblNormalModeMappings->selectedRanges();

  foreach( const QTableWidgetSelectionRange &range, l ) {
    for ( int i = 0; i < range.bottomRow()-range.topRow()+1; i++ ) {
      ui->tblNormalModeMappings->removeRow( range.topRow() );
    }
  }
}
//END KateViInputModeConfigTab

//BEGIN KateSpellCheckConfigTab
KateSpellCheckConfigTab::KateSpellCheckConfigTab(QWidget *parent)
  : KateConfigPage(parent)
{
  // This will let us have more separation between this page and
  // the KTabWidget edge (ereslibre)
  QVBoxLayout *layout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(this);

  ui = new Ui::SpellCheckConfigWidget();
  ui->setupUi(newWidget);

  // What's This? help can be found in the ui file
  reload();

  //
  // after initial reload, connect the stuff for the changed () signal

  m_sonnetConfigWidget = new Sonnet::ConfigWidget(KGlobal::config().data(), this);
  connect(m_sonnetConfigWidget, SIGNAL(configChanged()), this, SLOT(slotChanged()));
  layout->addWidget(m_sonnetConfigWidget);

  layout->addWidget(newWidget);
  setLayout(layout);
}

KateSpellCheckConfigTab::~KateSpellCheckConfigTab()
{
  delete ui;
}

void KateSpellCheckConfigTab::showWhatsThis(const QString& text)
{
  QWhatsThis::showText(QCursor::pos(), text);
}

void KateSpellCheckConfigTab::apply()
{
  if (!hasChanged()) {
    // nothing changed, no need to apply stuff
    return;
  }
  m_changed = false;

  KateDocumentConfig::global()->configStart();
  m_sonnetConfigWidget->save();
  KateDocumentConfig::global()->configEnd();
  foreach (KateDocument *doc, KateGlobal::self()->kateDocuments()) {
    doc->refreshOnTheFlyCheck();
  }
}

void KateSpellCheckConfigTab::reload()
{
  // does nothing
}
//END KateSpellCheckConfigTab

//BEGIN KateSelectConfigTab
KateSelectConfigTab::KateSelectConfigTab(QWidget *parent)
  : KateConfigPage(parent)
{
  // This will let us having more separation between this page and
  // the KTabWidget edge (ereslibre)
  QVBoxLayout *layout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(this);

  ui = new Ui::CursorConfigWidget();
  ui->setupUi( newWidget );

  // What's This? Help is in the ui-files

  reload ();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(ui->rbNormal, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->rbPersistent, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkSmartHome, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkWrapCursor, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkPagingMovesCursor, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->sbAutoCenterCursor, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));

  layout->addWidget(newWidget);
  setLayout(layout);
}

KateSelectConfigTab::~KateSelectConfigTab()
{
  delete ui;
}

void KateSelectConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateDocumentConfig::global()->configStart ();

  uint configFlags = KateDocumentConfig::global()->configFlags();

  configFlags &= ~KateDocumentConfig::cfSmartHome;
  configFlags &= ~KateDocumentConfig::cfWrapCursor;

  if (ui->chkSmartHome->isChecked()) configFlags |= KateDocumentConfig::cfSmartHome;
  if (ui->chkWrapCursor->isChecked()) configFlags |= KateDocumentConfig::cfWrapCursor;

  KateDocumentConfig::global()->setConfigFlags(configFlags);

  KateViewConfig::global()->setAutoCenterLines(qMax(0, ui->sbAutoCenterCursor->value()));
  KateDocumentConfig::global()->setPageUpDownMovesCursor(ui->chkPagingMovesCursor->isChecked());

  KateViewConfig::global()->setPersistentSelection (ui->rbPersistent->isChecked());

  KateDocumentConfig::global()->configEnd ();
  KateViewConfig::global()->configEnd ();
}

void KateSelectConfigTab::reload ()
{
  ui->rbNormal->setChecked( ! KateViewConfig::global()->persistentSelection() );
  ui->rbPersistent->setChecked( KateViewConfig::global()->persistentSelection() );

  const uint configFlags = KateDocumentConfig::global()->configFlags();

  ui->chkSmartHome->setChecked(configFlags & KateDocumentConfig::cfSmartHome);
  ui->chkWrapCursor->setChecked(configFlags & KateDocumentConfig::cfWrapCursor);
  ui->chkPagingMovesCursor->setChecked(KateDocumentConfig::global()->pageUpDownMovesCursor());
  ui->sbAutoCenterCursor->setValue(KateViewConfig::global()->autoCenterLines());
}
//END KateSelectConfigTab

//BEGIN KateEditGeneralConfigTab
KateEditGeneralConfigTab::KateEditGeneralConfigTab(QWidget *parent)
  : KateConfigPage(parent)
{
  QVBoxLayout *layout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(this);
  ui = new Ui::EditConfigWidget();
  ui->setupUi( newWidget );

  reload();

  connect( ui->chkReplaceTabs, SIGNAL(toggled(bool)), this, SLOT(slotChanged()) );
  connect(ui->chkShowTabs, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkShowSpaces, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->sbTabWidth, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect(ui->chkStaticWordWrap, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkShowStaticWordWrapMarker, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->sbWordWrap, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect( ui->chkRemoveTrailingSpaces, SIGNAL(toggled(bool)), this, SLOT(slotChanged()) );
  connect(ui->chkAutoBrackets, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkSmartCopyCut, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkScrollPastEnd, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));

  // "What's this?" help is in the ui-file

  layout->addWidget(newWidget);
  setLayout(layout);
}

KateEditGeneralConfigTab::~KateEditGeneralConfigTab()
{
  delete ui;
}

void KateEditGeneralConfigTab::apply ()
{
  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateDocumentConfig::global()->configStart ();

  uint configFlags = KateDocumentConfig::global()->configFlags();

  configFlags &= ~KateDocumentConfig::cfAutoBrackets;
  configFlags &= ~KateDocumentConfig::cfShowTabs;
  configFlags &= ~KateDocumentConfig::cfShowSpaces;
  configFlags &= ~KateDocumentConfig::cfReplaceTabsDyn;
  configFlags &= ~KateDocumentConfig::cfRemoveTrailingDyn;

  if (ui->chkAutoBrackets->isChecked()) configFlags |= KateDocumentConfig::cfAutoBrackets;
  if (ui->chkShowTabs->isChecked()) configFlags |= KateDocumentConfig::cfShowTabs;
  if (ui->chkShowSpaces->isChecked()) configFlags |= KateDocumentConfig::cfShowSpaces;
  if (ui->chkReplaceTabs->isChecked()) configFlags |= KateDocumentConfig::cfReplaceTabsDyn;
  if (ui->chkRemoveTrailingSpaces->isChecked()) configFlags |= KateDocumentConfig::cfRemoveTrailingDyn;

  KateDocumentConfig::global()->setConfigFlags(configFlags);

  KateDocumentConfig::global()->setWordWrapAt(ui->sbWordWrap->value());
  KateDocumentConfig::global()->setWordWrap(ui->chkStaticWordWrap->isChecked());
  KateDocumentConfig::global()->setTabWidth(ui->sbTabWidth->value());

  KateRendererConfig::global()->setWordWrapMarker (ui->chkShowStaticWordWrapMarker->isChecked());

  KateDocumentConfig::global()->configEnd ();
  KateViewConfig::global()->setSmartCopyCut(ui->chkSmartCopyCut->isChecked());
  KateViewConfig::global()->setScrollPastEnd(ui->chkScrollPastEnd->isChecked());
  KateViewConfig::global()->configEnd ();
}

void KateEditGeneralConfigTab::reload ()
{
  uint configFlags = KateDocumentConfig::global()->configFlags();

  ui->chkReplaceTabs->setChecked( configFlags & KateDocumentConfig::cfReplaceTabsDyn );
  ui->chkShowTabs->setChecked( configFlags & KateDocumentConfig::cfShowTabs );
  ui->chkShowSpaces->setChecked( configFlags & KateDocumentConfig::cfShowSpaces );
  ui->sbTabWidth->setSuffix(ki18np(" character", " characters"));
  ui->sbTabWidth->setValue( KateDocumentConfig::global()->tabWidth() );
  ui->chkStaticWordWrap->setChecked(KateDocumentConfig::global()->wordWrap());
  ui->chkShowStaticWordWrapMarker->setChecked( KateRendererConfig::global()->wordWrapMarker() );
  ui->sbWordWrap->setSuffix(ki18np(" character", " characters"));
  ui->sbWordWrap->setValue( KateDocumentConfig::global()->wordWrapAt() );
  ui->chkRemoveTrailingSpaces->setChecked( configFlags & KateDocumentConfig::cfRemoveTrailingDyn );
  ui->chkAutoBrackets->setChecked( configFlags & KateDocumentConfig::cfAutoBrackets );
  ui->chkSmartCopyCut->setChecked( KateViewConfig::global()->smartCopyCut() );
  ui->chkScrollPastEnd->setChecked( KateViewConfig::global()->scrollPastEnd() );
}
//END KateEditGeneralConfigTab


//BEGIN KateEditConfigTab
KateEditConfigTab::KateEditConfigTab(QWidget *parent)
  : KateConfigPage(parent)
  , editConfigTab(new KateEditGeneralConfigTab(this))
  , selectConfigTab(new KateSelectConfigTab(this))
  , indentConfigTab(new KateIndentConfigTab(this))
  , completionConfigTab (new KateCompletionConfigTab(this))
  , viInputModeConfigTab(new KateViInputModeConfigTab(this))
  , spellCheckConfigTab(new KateSpellCheckConfigTab(this))
{
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  KTabWidget *tabWidget = new KTabWidget(this);

  // add all tabs
  tabWidget->insertTab(0, editConfigTab, i18n("General"));
  tabWidget->insertTab(1, selectConfigTab, i18n("Cursor && Selection"));
  tabWidget->insertTab(2, indentConfigTab, i18n("Indentation"));
  tabWidget->insertTab(3, completionConfigTab, i18n("Auto Completion"));
  tabWidget->insertTab(4, viInputModeConfigTab, i18n("Vi Input Mode"));
  tabWidget->insertTab(5, spellCheckConfigTab, i18n("Spellcheck"));

  connect(editConfigTab, SIGNAL(changed()), this, SLOT(slotChanged()));
  connect(selectConfigTab, SIGNAL(changed()), this, SLOT(slotChanged()));
  connect(indentConfigTab, SIGNAL(changed()), this, SLOT(slotChanged()));
  connect(completionConfigTab, SIGNAL(changed()), this, SLOT(slotChanged()));
  connect(viInputModeConfigTab, SIGNAL(changed()), this, SLOT(slotChanged()));
  connect(spellCheckConfigTab, SIGNAL(changed()), this, SLOT(slotChanged()));

  layout->addWidget(tabWidget);
  setLayout(layout);
}

KateEditConfigTab::~KateEditConfigTab()
{
}

void KateEditConfigTab::apply ()
{
  // try to update the rest of tabs
  editConfigTab->apply();
  selectConfigTab->apply();
  indentConfigTab->apply();
  completionConfigTab->apply();
  viInputModeConfigTab->apply();
  spellCheckConfigTab->apply();
}

void KateEditConfigTab::reload ()
{
  editConfigTab->reload();
  selectConfigTab->reload();
  indentConfigTab->reload();
  completionConfigTab->reload();
  viInputModeConfigTab->reload();
  spellCheckConfigTab->reload();
}

void KateEditConfigTab::reset ()
{
  editConfigTab->reset();
  selectConfigTab->reset();
  indentConfigTab->reset();
  completionConfigTab->reset();
  viInputModeConfigTab->reset();
  spellCheckConfigTab->reset();
}

void KateEditConfigTab::defaults ()
{
  editConfigTab->defaults();
  selectConfigTab->defaults();
  indentConfigTab->defaults();
  completionConfigTab->defaults();
  viInputModeConfigTab->defaults();
  spellCheckConfigTab->defaults();
}
//END KateEditConfigTab

//BEGIN KateViewDefaultsConfig
KateViewDefaultsConfig::KateViewDefaultsConfig(QWidget *parent)
  :KateConfigPage(parent)
{
  ui = new Ui::AppearanceConfigWidget();
  ui->setupUi( this );

  if (KateDocument::simpleMode ())
    ui->gbSortBookmarks->hide ();

  ui->cmbDynamicWordWrapIndicator->addItem( i18n("Off") );
  ui->cmbDynamicWordWrapIndicator->addItem( i18n("Follow Line Numbers") );
  ui->cmbDynamicWordWrapIndicator->addItem( i18n("Always On") );

  // hide power user mode if activated anyway
  if (!KateGlobal::self()->simpleMode ())
    ui->chkDeveloperMode->hide ();

  // What's This? help is in the ui-file

  reload();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect(ui->gbWordWrap, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->cmbDynamicWordWrapIndicator, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect(ui->sbDynamicWordWrapDepth, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect(ui->chkIconBorder, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkScrollbarMarks, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkLineNumbers, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkShowFoldingMarkers, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->rbSortBookmarksByPosition, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->rbSortBookmarksByCreation, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkShowIndentationLines, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkShowWholeBracketExpression, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect(ui->chkDeveloperMode, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
}

KateViewDefaultsConfig::~KateViewDefaultsConfig()
{
  delete ui;
}

void KateViewDefaultsConfig::apply ()
{
  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateViewConfig::global()->configStart ();
  KateRendererConfig::global()->configStart ();

  KateViewConfig::global()->setDynWordWrap (ui->gbWordWrap->isChecked());
  KateViewConfig::global()->setDynWordWrapIndicators (ui->cmbDynamicWordWrapIndicator->currentIndex ());
  KateViewConfig::global()->setDynWordWrapAlignIndent(ui->sbDynamicWordWrapDepth->value());
  KateViewConfig::global()->setLineNumbers (ui->chkLineNumbers->isChecked());
  KateViewConfig::global()->setIconBar (ui->chkIconBorder->isChecked());
  KateViewConfig::global()->setScrollBarMarks (ui->chkScrollbarMarks->isChecked());
  KateViewConfig::global()->setFoldingBar (ui->chkShowFoldingMarkers->isChecked());

  KateViewConfig::global()->setBookmarkSort (ui->rbSortBookmarksByPosition->isChecked()?0:1);
  KateRendererConfig::global()->setShowIndentationLines(ui->chkShowIndentationLines->isChecked());
  KateRendererConfig::global()->setShowWholeBracketExpression(ui->chkShowWholeBracketExpression->isChecked());

  // warn user that he needs restart the application
  if (!ui->chkDeveloperMode->isChecked() != KateDocumentConfig::global()->allowSimpleMode())
  {
    // inform...
    KMessageBox::information(
                this,
                i18n("Changing the power user mode affects only newly opened / created documents. In KWrite a restart is recommended."),
                i18n("Power user mode changed"));

    KateDocumentConfig::global()->setAllowSimpleMode (!ui->chkDeveloperMode->isChecked());
  }

  KateRendererConfig::global()->configEnd ();
  KateViewConfig::global()->configEnd ();
}

void KateViewDefaultsConfig::reload ()
{
  ui->gbWordWrap->setChecked(KateViewConfig::global()->dynWordWrap());
  ui->cmbDynamicWordWrapIndicator->setCurrentIndex( KateViewConfig::global()->dynWordWrapIndicators() );
  ui->sbDynamicWordWrapDepth->setValue(KateViewConfig::global()->dynWordWrapAlignIndent());
  ui->chkLineNumbers->setChecked(KateViewConfig::global()->lineNumbers());
  ui->chkIconBorder->setChecked(KateViewConfig::global()->iconBar());
  ui->chkScrollbarMarks->setChecked(KateViewConfig::global()->scrollBarMarks());
  ui->chkShowFoldingMarkers->setChecked(KateViewConfig::global()->foldingBar());
  ui->rbSortBookmarksByPosition->setChecked(KateViewConfig::global()->bookmarkSort()==0);
  ui->rbSortBookmarksByCreation->setChecked(KateViewConfig::global()->bookmarkSort()==1);
  ui->chkShowIndentationLines->setChecked(KateRendererConfig::global()->showIndentationLines());
  ui->chkShowWholeBracketExpression->setChecked(KateRendererConfig::global()->showWholeBracketExpression());
  ui->chkDeveloperMode->setChecked(!KateDocumentConfig::global()->allowSimpleMode());
}

void KateViewDefaultsConfig::reset () {;}

void KateViewDefaultsConfig::defaults (){;}
//END KateViewDefaultsConfig

//BEGIN KateSaveConfigTab
KateSaveConfigTab::KateSaveConfigTab( QWidget *parent )
  : KateConfigPage( parent )
  , modeConfigPage( new ModeConfigPage( this ) )
{
  // FIXME: Is really needed to move all this code below to another class,
  // since it is another tab itself on the config dialog. This means we should
  // initialize, add and work with as we do with modeConfigPage (ereslibre)
  QVBoxLayout *layout = new QVBoxLayout;
  layout->setMargin(0);
  KTabWidget *tabWidget = new KTabWidget(this);

  QWidget *tmpWidget = new QWidget(tabWidget);
  QVBoxLayout *internalLayout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(tabWidget);
  ui = new Ui::OpenSaveConfigWidget();
  ui->setupUi( newWidget );

  QWidget *tmpWidget2 = new QWidget(tabWidget);
  QVBoxLayout *internalLayout2 = new QVBoxLayout;
  QWidget *newWidget2 = new QWidget(tabWidget);
  uiadv = new Ui::OpenSaveConfigAdvWidget();
  uiadv->setupUi( newWidget2 );

  // What's this help is added in ui/opensaveconfigwidget.ui
  reload();

  //
  // after initial reload, connect the stuff for the changed () signal
  //

  connect( ui->cmbEncoding, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect( ui->cmbEncodingDetection, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect( ui->cmbEncodingFallback, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect( ui->cmbEOL, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect( ui->chkDetectEOL, SIGNAL( toggled(bool) ), this, SLOT( slotChanged() ) );
  connect( ui->chkEnableBOM, SIGNAL( toggled(bool) ), this, SLOT( slotChanged() ) );
  connect( ui->chkRemoveTrailingSpaces, SIGNAL(toggled(bool)), this, SLOT(slotChanged()));
  connect( uiadv->chkBackupLocalFiles, SIGNAL( toggled(bool) ), this, SLOT( slotChanged() ) );
  connect( uiadv->chkBackupRemoteFiles, SIGNAL( toggled(bool) ), this, SLOT( slotChanged() ) );
  connect( uiadv->sbConfigFileSearchDepth, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect( uiadv->edtBackupPrefix, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotChanged() ) );
  connect( uiadv->edtBackupSuffix, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotChanged() ) );

  internalLayout->addWidget(newWidget);
  tmpWidget->setLayout(internalLayout);
  internalLayout2->addWidget(newWidget2);
  tmpWidget2->setLayout(internalLayout2);

  // add all tabs
  tabWidget->insertTab(0, tmpWidget, i18n("General"));
  tabWidget->insertTab(1, tmpWidget2, i18n("Advanced"));
  tabWidget->insertTab(2, modeConfigPage, i18n("Modes && Filetypes"));

  connect(modeConfigPage, SIGNAL(changed()), this, SLOT(slotChanged()));

  layout->addWidget(tabWidget);
  setLayout(layout);
}

KateSaveConfigTab::~KateSaveConfigTab()
{
  delete ui;
}

void KateSaveConfigTab::apply()
{
  modeConfigPage->apply();

  // nothing changed, no need to apply stuff
  if (!hasChanged())
    return;
  m_changed = false;

  KateGlobalConfig::global()->configStart ();
  KateDocumentConfig::global()->configStart ();

  if ( uiadv->edtBackupSuffix->text().isEmpty() && uiadv->edtBackupPrefix->text().isEmpty() ) {
    KMessageBox::information(
                this,
                i18n("You did not provide a backup suffix or prefix. Using default suffix: '~'"),
                i18n("No Backup Suffix or Prefix")
                        );
    uiadv->edtBackupSuffix->setText( "~" );
  }

  uint f( 0 );
  if ( uiadv->chkBackupLocalFiles->isChecked() )
    f |= KateDocumentConfig::LocalFiles;
  if ( uiadv->chkBackupRemoteFiles->isChecked() )
    f |= KateDocumentConfig::RemoteFiles;

  KateDocumentConfig::global()->setBackupFlags(f);
  KateDocumentConfig::global()->setBackupPrefix(uiadv->edtBackupPrefix->text());
  KateDocumentConfig::global()->setBackupSuffix(uiadv->edtBackupSuffix->text());

  KateDocumentConfig::global()->setSearchDirConfigDepth(uiadv->sbConfigFileSearchDepth->value());

  uint configFlags = KateDocumentConfig::global()->configFlags();

  configFlags &= ~KateDocumentConfig::cfRemoveSpaces; // clear flag
  if (ui->chkRemoveTrailingSpaces->isChecked()) configFlags |= KateDocumentConfig::cfRemoveSpaces; // set flag if checked

  KateDocumentConfig::global()->setConfigFlags(configFlags);

  // set both standard and fallback encoding
  KateDocumentConfig::global()->setEncoding((ui->cmbEncoding->currentIndex() == 0) ? "" : KGlobal::charsets()->encodingForName(ui->cmbEncoding->currentText()));

  KateGlobalConfig::global()->setProberType((KEncodingProber::ProberType)ui->cmbEncodingDetection->currentIndex());
  KateGlobalConfig::global()->setFallbackEncoding(KGlobal::charsets()->encodingForName(ui->cmbEncodingFallback->currentText()));

  KateDocumentConfig::global()->setEol(ui->cmbEOL->currentIndex());
  KateDocumentConfig::global()->setAllowEolDetection(ui->chkDetectEOL->isChecked());
  KateDocumentConfig::global()->setBom(ui->chkEnableBOM->isChecked());

  KateDocumentConfig::global()->configEnd ();
  KateGlobalConfig::global()->configEnd ();
}

void KateSaveConfigTab::reload()
{
  modeConfigPage->reload();

  // encodings
  ui->cmbEncoding->clear ();
  ui->cmbEncoding->addItem (i18n("KDE Default"));
  ui->cmbEncoding->setCurrentIndex(0);
  ui->cmbEncodingFallback->clear ();
  QStringList encodings (KGlobal::charsets()->descriptiveEncodingNames());
  int insert = 1;
  for (int i=0; i < encodings.count(); i++)
  {
    bool found = false;
    QTextCodec *codecForEnc = KGlobal::charsets()->codecForName(KGlobal::charsets()->encodingForName(encodings[i]), found);

    if (found)
    {
      ui->cmbEncoding->addItem (encodings[i]);
      ui->cmbEncodingFallback->addItem (encodings[i]);

      if ( codecForEnc->name() == KateDocumentConfig::global()->encoding() )
      {
        ui->cmbEncoding->setCurrentIndex(insert);
      }

      if ( codecForEnc == KateGlobalConfig::global()->fallbackCodec() )
      {
        // adjust index for fallback config, has no default!
        ui->cmbEncodingFallback->setCurrentIndex(insert-1);
      }

      insert++;
    }
  }

  // encoding detection
  ui->cmbEncodingDetection->clear ();
  bool found = false;
  for (int i = 0; !KEncodingProber::nameForProberType ((KEncodingProber::ProberType) i).isEmpty(); ++i) {
    ui->cmbEncodingDetection->addItem (KEncodingProber::nameForProberType ((KEncodingProber::ProberType) i));
    if (i == KateGlobalConfig::global()->proberType()) {
      ui->cmbEncodingDetection->setCurrentIndex(ui->cmbEncodingDetection->count()-1);
      found = true;
    }
  }
  if (!found)
      ui->cmbEncodingDetection->setCurrentIndex(KEncodingProber::Universal);


  // eol
  ui->cmbEOL->setCurrentIndex(KateDocumentConfig::global()->eol());
  ui->chkDetectEOL->setChecked(KateDocumentConfig::global()->allowEolDetection());
  ui->chkEnableBOM->setChecked(KateDocumentConfig::global()->bom());

  const uint configFlags = KateDocumentConfig::global()->configFlags();
  ui->chkRemoveTrailingSpaces->setChecked(configFlags & KateDocumentConfig::cfRemoveSpaces);
  uiadv->sbConfigFileSearchDepth->setValue(KateDocumentConfig::global()->searchDirConfigDepth());

  // other stuff
  uint f ( KateDocumentConfig::global()->backupFlags() );
  uiadv->chkBackupLocalFiles->setChecked( f & KateDocumentConfig::LocalFiles );
  uiadv->chkBackupRemoteFiles->setChecked( f & KateDocumentConfig::RemoteFiles );
  uiadv->edtBackupPrefix->setText( KateDocumentConfig::global()->backupPrefix() );
  uiadv->edtBackupSuffix->setText( KateDocumentConfig::global()->backupSuffix() );
}

void KateSaveConfigTab::reset()
{
  modeConfigPage->reset();
}

void KateSaveConfigTab::defaults()
{
  modeConfigPage->defaults();

  uiadv->chkBackupLocalFiles->setChecked( true );
  uiadv->chkBackupRemoteFiles->setChecked( false );
  uiadv->edtBackupPrefix->setText( "" );
  uiadv->edtBackupSuffix->setText( "~" );
}

//END KateSaveConfigTab

//BEGIN KatePartPluginConfigPage
KatePartPluginConfigPage::KatePartPluginConfigPage (QWidget *parent)
  : KateConfigPage (parent, "")
  , scriptConfigPage (new KateScriptConfigPage(this))
{
  // FIXME: Is really needed to move all this code below to another class,
  // since it is another tab itself on the config dialog. This means we should
  // initialize, add and work with as we do with scriptConfigPage (ereslibre)
  QVBoxLayout *generalLayout = new QVBoxLayout;
  generalLayout->setMargin(0);
  KTabWidget *tabWidget = new KTabWidget(this);

  QWidget *tmpWidget = new QWidget(tabWidget);
  QVBoxLayout *internalLayout = new QVBoxLayout;
  QWidget *newWidget = new QWidget(tabWidget);
  QVBoxLayout *layout = new QVBoxLayout;
  newWidget->setLayout(layout);
  layout->setMargin(0);

  plugins.clear();

  int i = 0;
  foreach (const KatePartPluginInfo &info, KatePartPluginManager::self()->pluginList())
  {
    KPluginInfo it(info.service());
    it.setPluginEnabled(info.load);
    plugins.append(it);
    i++;
  }

  selector = new KPluginSelector(0);

  connect(selector, SIGNAL(changed(bool)), this, SLOT(slotChanged()));
  connect(selector, SIGNAL(configCommitted(QByteArray)), this, SLOT(slotChanged()));

  selector->addPlugins(plugins, KPluginSelector::IgnoreConfigFile, i18n("Editor Plugins"), "Editor");
  layout->addWidget(selector);

  internalLayout->addWidget(newWidget);
  tmpWidget->setLayout(internalLayout);

  // add all tabs
  tabWidget->insertTab(0, tmpWidget, i18n("Plugins"));
  tabWidget->insertTab(1, scriptConfigPage, i18n("Scripts"));

  generalLayout->addWidget(tabWidget);
  setLayout(generalLayout);
}

KatePartPluginConfigPage::~KatePartPluginConfigPage ()
{
}

void KatePartPluginConfigPage::apply ()
{
  scriptConfigPage->apply();

  selector->updatePluginsState();

  KatePartPluginList &katePluginList = KatePartPluginManager::self()->pluginList();
  for (int i=0; i < plugins.count(); i++) {
    if (plugins[i].isPluginEnabled()) {
      if (!katePluginList[i].load) {
        KatePartPluginManager::self()->loadPlugin(katePluginList[i]);
        KatePartPluginManager::self()->enablePlugin(katePluginList[i]);
      }
    } else {
      if (katePluginList[i].load) {
        KatePartPluginManager::self()->disablePlugin(katePluginList[i]);
        KatePartPluginManager::self()->unloadPlugin(katePluginList[i]);
      }
    }
  }
}

void KatePartPluginConfigPage::reload ()
{
  scriptConfigPage->reload();

  selector->load();
}

void KatePartPluginConfigPage::reset ()
{
  scriptConfigPage->reset();

  selector->load();
}

void KatePartPluginConfigPage::defaults ()
{
  scriptConfigPage->defaults();

  selector->defaults();
}
//END KatePartPluginConfigPage

class KateScriptNewStuff {};

/*
class KateScriptNewStuff: public KNewStuff {
  public:
    KateScriptNewStuff(QWidget *parent):KNewStuff("kate/scripts",parent) {}
    virtual ~KateScriptNewStuff() {}
    virtual bool install( const QString &fileName ) {return false;}
    virtual bool createUploadFile( const QString &fileName ) {return false;}
};
*/
//BEGIN KateScriptConfigPage
KateScriptConfigPage::KateScriptConfigPage(QWidget *parent): KateConfigPage(parent,""), m_newStuff(new KateScriptNewStuff())
{
  // TODO: Please look at KateSelectConfigTab or ModeConfigPage to add
  // a layout like we do there, to be consistent and have on all config
  // pages the same distance to the KTabWidget edge (ereslibre)

  //m_newStuff->download();
}

KateScriptConfigPage::~KateScriptConfigPage()
{
  delete m_newStuff;
  m_newStuff=0;
}

void KateScriptConfigPage::apply () {
}
void KateScriptConfigPage::reload () {
}

//END KateScriptConfigPage

//BEGIN KateHlDownloadDialog
KateHlDownloadDialog::KateHlDownloadDialog(QWidget *parent, const char *name, bool modal)
  : KDialog( parent )
{
  setCaption( i18n("Highlight Download") );
  setButtons( User1 | Close );
  setButtonGuiItem( User1, KGuiItem(i18n("&Install")) );
  setDefaultButton( User1 );
  setObjectName( name );
  setModal( modal );

  KVBox* vbox = new KVBox(this);
  setMainWidget(vbox);
  vbox->setSpacing(-1);
  new QLabel(i18n("Select the syntax highlighting files you want to update:"), vbox);
  list = new QTreeWidget(vbox);
  list->setColumnCount(4);
  list->setHeaderLabels(QStringList() << "" << i18n("Name") << i18n("Installed") << i18n("Latest"));
  list->setSelectionMode(QAbstractItemView::MultiSelection);
  list->setAllColumnsShowFocus(true);
  list->setRootIsDecorated(false);
  list->setColumnWidth(0, 22);

  new QLabel(i18n("<b>Note:</b> New versions are selected automatically."), vbox);
  setButtonIcon(User1, KIcon("dialog-ok"));

  transferJob = KIO::get(
    KUrl(QString(HLDOWNLOADPATH)
       + QString("update-")
       + KateGlobal::katePartVersion()
       + QString(".xml")), KIO::Reload );
  connect(transferJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
    this, SLOT(listDataReceived(KIO::Job *, const QByteArray &)));
//        void data( KIO::Job *, const QByteArray &data);
  resize(450, 400);
  connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
}

KateHlDownloadDialog::~KateHlDownloadDialog(){}

void KateHlDownloadDialog::listDataReceived(KIO::Job *, const QByteArray &data)
{
  if (!transferJob || transferJob->isErrorPage())
  {
    enableButton( User1, false );
    if (data.size()==0)
      KMessageBox::error(this,i18n("The list of highlightings could not be found on / retrieved from the server"));
    return;
  }

  listData+=QString(data);
  kDebug(13000)<<QString("CurrentListData: ")<<listData;
  kDebug(13000)<<QString("Data length: %1").arg(data.size());
  kDebug(13000)<<QString("listData length: %1").arg(listData.length());
  if (data.size()==0)
  {
    if (listData.length()>0)
    {
      QString installedVersion;
      KateHlManager *hlm=KateHlManager::self();
      QDomDocument doc;
      doc.setContent(listData);
      QDomElement DocElem=doc.documentElement();
      QDomNode n=DocElem.firstChild();
      KateHighlighting *hl = 0;

      if (n.isNull()) kDebug(13000)<<"There is no usable childnode";
      while (!n.isNull())
      {
        installedVersion="    --";

        QDomElement e=n.toElement();
        if (!e.isNull())
        kDebug(13000)<<QString("NAME: ")<<e.tagName()<<QString(" - ")<<e.attribute("name");
        n=n.nextSibling();

        QString Name=e.attribute("name");

        for (int i=0;i<hlm->highlights();i++)
        {
          hl=hlm->getHl(i);
          if (hl && hl->name()==Name)
          {
            installedVersion="    "+hl->version();
            break;
          }
          else hl = 0;
        }

        // autoselect entry if new or updated.
        QTreeWidgetItem* entry = new QTreeWidgetItem(list);
        entry->setText(0, "");
        entry->setText(1, e.attribute("name"));
        entry->setText(2, installedVersion);
        entry->setText(3, e.attribute("version"));
        entry->setText(4, e.attribute("url"));

        if (!hl || hl->version() < e.attribute("version"))
        {
          entry->treeWidget()->setItemSelected(entry, true);
          entry->setIcon(0, SmallIcon(("get-hot-new-stuff")));
        }
      }
      list->resizeColumnToContents(1);
    }
  }
}

void KateHlDownloadDialog::slotUser1()
{
  QString destdir=KGlobal::dirs()->saveLocation("data","katepart/syntax/");
  foreach (QTreeWidgetItem *it, list->selectedItems())
  {
    KUrl src(it->text(4));
    QString filename=src.fileName(KUrl::ObeyTrailingSlash);
    QString dest = destdir+filename;

    KIO::NetAccess::download(src,dest, this);
  }

  // update Config !!
  // this rewrites the cache....
  KateSyntaxDocument doc (KateHlManager::self()->getKConfig(), true);
}
//END KateHlDownloadDialog

//BEGIN KateGotoBar
KateGotoBar::KateGotoBar(KTextEditor::View *view, QWidget *parent)
  : KateViewBarWidget( true, parent )
  , m_view( view )
{
  Q_ASSERT( m_view != 0 );  // this bar widget is pointless w/o a view

  QHBoxLayout *topLayout = new QHBoxLayout( centralWidget() );
  topLayout->setMargin(0);
  gotoRange = new KIntSpinBox(centralWidget());

  QLabel *label = new QLabel(i18n("&Go to line:"), centralWidget() );
  label->setBuddy(gotoRange);

  QToolButton *btnOK = new QToolButton(centralWidget());
  btnOK->setAutoRaise(true);
  btnOK->setIcon(QIcon(SmallIcon("go-jump")));
  btnOK->setText(i18n("Go"));
  btnOK->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  connect(btnOK, SIGNAL(clicked()), this, SLOT(gotoLine()));

  topLayout->addWidget(label);
  topLayout->addWidget(gotoRange, 1);
  topLayout->setStretchFactor( gotoRange, 0 );
  topLayout->addWidget(btnOK);
  topLayout->addStretch();
}

void KateGotoBar::updateData()
{
  gotoRange->setMaximum(m_view->document()->lines());
  if (!isVisible())
  {
    gotoRange->setValue(m_view->cursorPosition().line() + 1);
    gotoRange->adjustSize(); // ### does not respect the range :-(
  }
  gotoRange->setFocus(Qt::OtherFocusReason);
  gotoRange->selectAll();
}

void KateGotoBar::keyPressEvent(QKeyEvent* event)
{
  int key = event->key();
  if (key == Qt::Key_Return || key == Qt::Key_Enter) {
    gotoLine();
    return;
  }
  KateViewBarWidget::keyPressEvent(event);
}

void KateGotoBar::gotoLine()
{
  m_view->setCursorPosition( KTextEditor::Cursor(gotoRange->value() - 1, 0) );
  m_view->setFocus();
  emit hideMe();
}
//END KateGotoBar

//BEGIN KateDictionaryBar
KateDictionaryBar::KateDictionaryBar(KateView* view, QWidget *parent)
  : KateViewBarWidget( true, parent )
  , m_view( view )
{
  Q_ASSERT(m_view != 0); // this bar widget is pointless w/o a view

  QHBoxLayout *topLayout = new QHBoxLayout(centralWidget());
  topLayout->setMargin(0);
  //topLayout->setSpacing(spacingHint());
  m_dictionaryComboBox = new Sonnet::DictionaryComboBox(centralWidget());
  connect(m_dictionaryComboBox, SIGNAL(dictionaryChanged(const QString&)),
          this, SLOT(dictionaryChanged(const QString&)));
  connect(view->doc(), SIGNAL(defaultDictionaryChanged(KateDocument*)),
          this, SLOT(updateData()));
  QLabel *label = new QLabel(i18n("Dictionary:"), centralWidget());
  label->setBuddy(m_dictionaryComboBox);

  topLayout->addWidget(label);
  topLayout->addWidget(m_dictionaryComboBox, 1);
  topLayout->setStretchFactor(m_dictionaryComboBox, 0);
  topLayout->addStretch();
}

KateDictionaryBar::~KateDictionaryBar()
{
}

void KateDictionaryBar::updateData()
{
  KateDocument *document = m_view->doc();
  QString dictionary = document->defaultDictionary();
  if(dictionary.isEmpty()) {
    dictionary = Sonnet::Speller().defaultLanguage();
  }
  m_dictionaryComboBox->setCurrentByDictionary(dictionary);
}

void KateDictionaryBar::dictionaryChanged(const QString& dictionary)
{
  KTextEditor::Range selection = m_view->selectionRange();
  if(selection.isValid() && !selection.isEmpty()) {
    m_view->doc()->setDictionary(dictionary, selection);
  }
  else {
    m_view->doc()->setDefaultDictionary(dictionary);
  }
}

//END KateGotoBar


//BEGIN KateModOnHdPrompt
KateModOnHdPrompt::KateModOnHdPrompt( KateDocument *doc,
                                      KTextEditor::ModificationInterface::ModifiedOnDiskReason modtype,
                                      const QString &reason,
                                      QWidget *parent )
  : KDialog( parent ),
    m_doc( doc ),
    m_modtype ( modtype ),
    m_proc( 0 ),
    m_diffFile( 0 )
{
  setButtons( Ok | Apply | Cancel | User1 );

  QString title, btnOK, whatisok;
  if ( modtype == KTextEditor::ModificationInterface::OnDiskDeleted )
  {
    title = i18n("File Was Deleted on Disk");
    btnOK = i18n("&Save File As...");
    whatisok = i18n("Lets you select a location and save the file again.");
  } else {
    title = i18n("File Changed on Disk");
    btnOK = i18n("&Reload File");
    whatisok = i18n("Reload the file from disk. If you have unsaved changes, "
        "they will be lost.");
  }

  setButtonText( Ok, btnOK );
  setButtonText( Apply, i18n("&Ignore") );

  setButtonWhatsThis( Ok, whatisok );
  setButtonWhatsThis( Apply, i18n("Ignore the changes. You will not be prompted again.") );
  setButtonWhatsThis( Cancel, i18n("Do nothing. Next time you focus the file, "
      "or try to save it or close it, you will be prompted again.") );

  setCaption( title );

  QWidget *w = new QWidget(this);
  ui = new Ui::ModOnHdWidget();
  ui->setupUi( w );
  setMainWidget( w );

  ui->lblIcon->setPixmap( DesktopIcon("dialog-warning" ) );
  ui->lblText->setText( reason + "\n\n" + i18n("What do you want to do?") );

  // If the file isn't deleted, present a diff button, and a overwrite action.
  if ( modtype != KTextEditor::ModificationInterface::OnDiskDeleted )
  {
    setButtonText( User1, i18n("Overwrite") );
    setButtonWhatsThis( User1, i18n("Overwrite the disk file with the editor content.") );
    connect( ui->btnDiff, SIGNAL(clicked()), this, SLOT(slotDiff()) );
  }
  else
  {
    ui->chkIgnoreWhiteSpaces->setVisible( false );
    ui->btnDiff->setVisible( false );
    showButton( User1, false );
  }
}

KateModOnHdPrompt::~KateModOnHdPrompt()
{
  delete m_proc;
  m_proc = 0;
  if (m_diffFile) {
    m_diffFile->setAutoRemove(true);
    delete m_diffFile;
    m_diffFile = 0;
  }
  delete ui;
}

void KateModOnHdPrompt::slotDiff()
{
  if (m_diffFile)
    return;

  m_diffFile = new KTemporaryFile();
  m_diffFile->open();

  // Start a KProcess that creates a diff
  m_proc = new KProcess( this );
  m_proc->setOutputChannelMode( KProcess::MergedChannels );
  *m_proc << "diff" << QString(ui->chkIgnoreWhiteSpaces->isChecked() ? "-ub" : "-u")
     << "-" <<  m_doc->url().toLocalFile();
  connect( m_proc, SIGNAL(readyRead()), this, SLOT(slotDataAvailable()) );
  connect( m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotPDone()) );

  setCursor( Qt::WaitCursor );
  // disable the button and checkbox, to hinder the user to run it twice.
  ui->chkIgnoreWhiteSpaces->setEnabled( false );
  ui->btnDiff->setEnabled( false );

  m_proc->start();

  QTextStream ts(m_proc);
  int lastln = m_doc->lines();
  for ( int l = 0; l < lastln; ++l )
    ts << m_doc->line( l ) << '\n';
  ts.flush();
  m_proc->closeWriteChannel();
}

void KateModOnHdPrompt::slotDataAvailable()
{
  m_diffFile->write(m_proc->readAll());
}

void KateModOnHdPrompt::slotPDone()
{
  setCursor( Qt::ArrowCursor );
  ui->chkIgnoreWhiteSpaces->setEnabled( true );
  ui->btnDiff->setEnabled( true );

  const QProcess::ExitStatus es = m_proc->exitStatus();
  delete m_proc;
  m_proc = 0;

  if ( es != QProcess::NormalExit )
  {
    KMessageBox::sorry( this,
                        i18n("The diff command failed. Please make sure that "
                             "diff(1) is installed and in your PATH."),
                        i18n("Error Creating Diff") );
    delete m_diffFile;
    m_diffFile = 0;
    return;
  }

  if ( m_diffFile->size() == 0 )
  {
    KMessageBox::information( this,
                              i18n("Besides white space changes, the files are identical."),
                              i18n("Diff Output") );
    delete m_diffFile;
    m_diffFile = 0;
    return;
  }

  m_diffFile->setAutoRemove(false);
  KUrl url = KUrl::fromPath(m_diffFile->fileName());
  delete m_diffFile;
  m_diffFile = 0;

  // KRun::runUrl should delete the file, once the client exits
  KRun::runUrl( url, "text/x-patch", this, true );
}

void KateModOnHdPrompt::slotButtonClicked(int button)
{
  switch(button)
  {
    case Default:
    case Ok:
      done( (m_modtype == KTextEditor::ModificationInterface::OnDiskDeleted) ?
            Save : Reload );
      break;
    case Apply:
    {
      if ( KMessageBox::warningContinueCancel(
           this,
           i18n("Ignoring means that you will not be warned again (unless "
           "the disk file changes once more): if you save the document, you "
           "will overwrite the file on disk; if you do not save then the disk "
           "file (if present) is what you have."),
           i18n("You Are on Your Own"),
           KStandardGuiItem::cont(),
           KStandardGuiItem::cancel(),
           "kate_ignore_modonhd" ) != KMessageBox::Continue )
        return;
      done( Ignore );
      break;
    }
    case User1:
      done( Overwrite );
      break;
    default:
      KDialog::slotButtonClicked(button);
  }
}

//END KateModOnHdPrompt

// kate: space-indent on; indent-width 2; replace-tabs on;
