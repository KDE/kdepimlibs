/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the KDE resource framework and defines the
  ConfigPage class.

  @brief
  A resource configuration page.

  @author Tobias Koenig
  @author Jan-Pascal van Best
  @author Cornelius Schumacher
*/

#include "configpage.h"

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <kdialogbuttonbox.h>
#include <kservicetypetrader.h>
#include <kinputdialog.h>
#include <QtCore/QList>

#include "resource.h"
#include "configdialog.h"

namespace KRES {

class ResourcePageInfo::Private
{
};

ResourcePageInfo::ResourcePageInfo() : d( new KRES::ResourcePageInfo::Private )
{
  mManager = 0;
  mConfig = 0;
}

ResourcePageInfo::~ResourcePageInfo()
{
  //delete mManager;
  mManager = 0;
  //delete mConfig;
  mConfig = 0;
  delete d;
}

class ConfigViewItem : public QTreeWidgetItem
{
  public:
    ConfigViewItem( QTreeWidget *parent, Resource *resource ) :
      QTreeWidgetItem( parent ), mResource( resource ), mIsStandard( false )
    {
      updateItem();
    }

    void setStandard( bool value )
    {
      setText( 2, ( value ? i18nc( "yes, a standard resource", "Yes" ) : QString() ) );
      mIsStandard = value;
    }

    bool standard() const { return mIsStandard; }
    bool readOnly() const { return mResource->readOnly(); }

    Resource *resource() { return mResource; }

    void updateItem()
    {
      setCheckState( 0, mResource->isActive() ? Qt::Checked : Qt::Unchecked );
      setText( 0, mResource->resourceName() );
      setText( 1, mResource->type() );
      setText( 2, mIsStandard ? i18nc( "yes, a standard resource", "Yes" ) : QString() );
    }

    bool isOn()
    {
      return checkState( 0 ) == Qt::Checked;
    }

  private:
    Resource *mResource;
    bool mIsStandard;
};

class ConfigPage::Private
{
  public:
    void loadManager( const QString &family, ConfigPage *page );
    void saveResourceSettings( ConfigPage *page );

    Manager<Resource> *mCurrentManager;
    KConfig *mCurrentConfig;
    KConfigGroup *mConfigGroup;
    QString mFamily;
    QStringList mFamilyMap;
    QList<KSharedPtr<ResourcePageInfo> > mInfoMap;

    KComboBox *mFamilyCombo;
    QTreeWidget *mListView;
    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mEditButton;
    QPushButton *mStandardButton;

    QTreeWidgetItem *mLastItem;
};

ConfigPage::ConfigPage( QWidget *parent )
  : QWidget( parent ), d( new KRES::ConfigPage::Private )
{
  setWindowTitle( i18n( "Resource Configuration" ) );

  QVBoxLayout *mainLayout = new QVBoxLayout( this );

  QGroupBox *groupBox = new QGroupBox( i18n( "Resources" ), this );
  QGridLayout *groupBoxLayout = new QGridLayout();
  groupBox->setLayout( groupBoxLayout );
  groupBoxLayout->setSpacing( 6 );
  groupBoxLayout->setMargin( 11 );

  d->mFamilyCombo = new KComboBox( false, groupBox );
  groupBoxLayout->addWidget( d->mFamilyCombo, 0, 0, 1, 2 );

  d->mCurrentManager = 0;
  d->mCurrentConfig = 0;
  d->mListView = new QTreeWidget( groupBox );
  d->mListView->setColumnCount( 3 );
  QStringList headerLabels;
  headerLabels << i18nc( "@title:column resource name", "Name" )
               << i18nc( "@title:column resource type", "Type" )
               << i18nc( "@title:column a standard resource?", "Standard" );
  d->mListView->setHeaderItem( new QTreeWidgetItem( headerLabels ) );

  groupBoxLayout->addWidget( d->mListView, 1, 0 );
  connect( d->mListView, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           this, SLOT( slotEdit() ) );

  KDialogButtonBox *buttonBox = new KDialogButtonBox( groupBox, Qt::Vertical );
  d->mAddButton = buttonBox->addButton( i18n( "&Add..." ),
                                        KDialogButtonBox::ActionRole,
                                        this, SLOT(slotAdd()) );

  d->mRemoveButton = buttonBox->addButton( i18n( "&Remove" ),
                                           KDialogButtonBox::ActionRole,
                                           this, SLOT(slotRemove()) );
  d->mRemoveButton->setEnabled( false );

  d->mEditButton = buttonBox->addButton( i18n( "&Edit..." ),
                                         KDialogButtonBox::ActionRole,
                                         this, SLOT(slotEdit()) );
  d->mEditButton->setEnabled( false );

  d->mStandardButton = buttonBox->addButton( i18n( "&Use as Standard" ),
                                             KDialogButtonBox::ActionRole,
                                             this, SLOT(slotStandard()) );
  d->mStandardButton->setEnabled( false );

  buttonBox->layout();
  groupBoxLayout->addWidget( buttonBox, 1, 1 );

  mainLayout->addWidget( groupBox );

  connect( d->mFamilyCombo, SIGNAL( activated( int ) ),
           SLOT( slotFamilyChanged( int ) ) );
  connect( d->mListView, SIGNAL( itemSelectionChanged() ),
           SLOT( slotSelectionChanged() ) );
  connect( d->mListView, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
           SLOT( slotItemClicked( QTreeWidgetItem * ) ) );

  d->mLastItem = 0;

  d->mConfigGroup = new KConfigGroup( new KConfig( "kcmkresourcesrc" ), "General" );

  load();
}

ConfigPage::~ConfigPage()
{
  QList<KSharedPtr<ResourcePageInfo> >::Iterator it;
  for ( it = d->mInfoMap.begin(); it != d->mInfoMap.end(); ++it ) {
    (*it)->mManager->removeObserver( this );
  }

  d->mConfigGroup->writeEntry( "CurrentFamily", d->mFamilyCombo->currentIndex() );
  delete d->mConfigGroup->config();
  delete d->mConfigGroup;
  d->mConfigGroup = 0;
  delete d;
}

void ConfigPage::load()
{
  kDebug();

  d->mListView->clear();
  d->mFamilyMap.clear();
  d->mInfoMap.clear();
  QStringList familyDisplayNames;

  // KDE-3.3 compatibility code: get families from the plugins
  QStringList compatFamilyNames;
  const KService::List plugins = KServiceTypeTrader::self()->query( "KResources/Plugin" );
  KService::List::ConstIterator it = plugins.begin();
  KService::List::ConstIterator end = plugins.end();
  for ( ; it != end; ++it ) {
    const QString family = (*it)->property( "X-KDE-ResourceFamily" ).toString();
    if ( compatFamilyNames.indexOf( family ) == -1 ) {
      compatFamilyNames.append( family );
    }
  }

  const KService::List managers = KServiceTypeTrader::self()->query( "KResources/Manager" );
  KService::List::ConstIterator m_it;
  for ( m_it = managers.begin(); m_it != managers.end(); ++m_it ) {
    QString displayName = (*m_it)->property( "Name" ).toString();
    familyDisplayNames.append( displayName );
    QString family = (*m_it)->property( "X-KDE-ResourceFamily" ).toString();
    if ( !family.isEmpty() ) {
      compatFamilyNames.removeAll( family );
      d->mFamilyMap.append( family );
      d->loadManager( family, this );
    }
  }

  // Rest of the kde-3.3 compat code
  QStringList::ConstIterator cfit = compatFamilyNames.begin();
  for ( ; cfit != compatFamilyNames.end(); ++cfit ) {
    d->mFamilyMap.append( *cfit );
    familyDisplayNames.append( *cfit );
    d->loadManager( *cfit, this );
  }

  d->mCurrentManager = 0;

  d->mFamilyCombo->clear();
  d->mFamilyCombo->insertItems( 0, familyDisplayNames );

  int currentFamily = d->mConfigGroup->readEntry( "CurrentFamily", 0 );
  d->mFamilyCombo->setCurrentIndex( currentFamily );
  slotFamilyChanged( currentFamily );
  emit changed( false );
}

void ConfigPage::Private::loadManager( const QString &family, ConfigPage *page )
{
  mCurrentManager = new Manager<Resource>( family );
  if ( mCurrentManager ) {
    mCurrentManager->addObserver( page );

    ResourcePageInfo *info = new ResourcePageInfo;
    info->mManager = mCurrentManager;
    info->mConfig = new KConfig( KRES::ManagerImpl::defaultConfigFile( family ) );
    info->mManager->readConfig( info->mConfig );

    mInfoMap.append( KSharedPtr<ResourcePageInfo>( info ) );
  }
}

void ConfigPage::save()
{
  d->saveResourceSettings( this );

  QList<KSharedPtr<ResourcePageInfo> >::Iterator it;
  for ( it = d->mInfoMap.begin(); it != d->mInfoMap.end(); ++it ) {
    (*it)->mManager->writeConfig( (*it)->mConfig );
  }

  emit changed( false );
}

void ConfigPage::defaults()
{
}

void ConfigPage::slotFamilyChanged( int pos )
{
  if ( pos < 0 || pos >= (int)d->mFamilyMap.count() ) {
    return;
  }

  d->saveResourceSettings( this );

  d->mFamily = d->mFamilyMap[ pos ];

  d->mCurrentManager = d->mInfoMap[ pos ]->mManager;
  d->mCurrentConfig = d->mInfoMap[ pos ]->mConfig;

  if ( !d->mCurrentManager ) {
    kDebug() << "ERROR: cannot create ResourceManager<Resource>( mFamily )";
  }

  d->mListView->clear();

  if ( d->mCurrentManager->isEmpty() ) {
    defaults();
  }

  Resource *standardResource = d->mCurrentManager->standardResource();

  Manager<Resource>::Iterator it;
  for ( it = d->mCurrentManager->begin(); it != d->mCurrentManager->end(); ++it ) {
    ConfigViewItem *item = new ConfigViewItem( d->mListView, *it );
    if ( *it == standardResource ) {
      item->setStandard( true );
    }
  }

  if ( d->mListView->topLevelItemCount() == 0 ) {
    defaults();
    emit changed( true );
    d->mCurrentManager->writeConfig( d->mCurrentConfig );
  } else {
    if ( !standardResource ) {
      KMessageBox::sorry( this, i18n( "There is no standard resource. Please select one." ) );
    }

    emit changed( false );
  }
}

void ConfigPage::slotAdd()
{
  if ( !d->mCurrentManager ) {
    return;
  }

  QStringList types = d->mCurrentManager->resourceTypeNames();
  QStringList descs = d->mCurrentManager->resourceTypeDescriptions();
  bool ok = false;
  QString desc = KInputDialog::getItem( i18n( "Resource Configuration" ),
                                        i18n( "Please select type of the new resource:" ), descs,
                                        0, false, &ok, this );
  if ( !ok ) {
    return;
  }

  QString type = types[ descs.indexOf( desc ) ];

  // Create new resource
  Resource *resource = d->mCurrentManager->createResource( type );
  if ( !resource ) {
    KMessageBox::error(
      this, i18n( "Unable to create resource of type '%1'.", type ) );
    return;
  }

  resource->setResourceName( type + "-resource" );

  ConfigDialog dlg( this, d->mFamily, resource );

  if ( dlg.exec() ) {
    d->mCurrentManager->add( resource );

    ConfigViewItem *item = new ConfigViewItem( d->mListView, resource );

    d->mLastItem = item;

    // if there are only read-only resources we'll set this resource
    // as standard resource
    if ( !resource->readOnly() ) {
      bool onlyReadOnly = true;
      for ( int i = 0; i < d->mListView->topLevelItemCount(); ++i ) {
        ConfigViewItem *confIt = static_cast<ConfigViewItem*>( d->mListView->topLevelItem( i ) );
        if ( !confIt->readOnly() && confIt != item ) {
          onlyReadOnly = false;
        }
      }

      if ( onlyReadOnly ) {
        item->setStandard( true );
      }
    }

    emit changed( true );
  } else {
    delete resource;
    resource = 0;
  }
}

void ConfigPage::slotRemove()
{
  if ( !d->mCurrentManager ) {
    return;
  }

  QTreeWidgetItem *item = d->mListView->currentItem();
  ConfigViewItem *confItem = static_cast<ConfigViewItem*>( item );

  if ( !confItem ) {
    return;
  }

  if ( confItem->standard() ) {
    KMessageBox::sorry( this,
                        i18n( "You cannot remove your standard resource. "
                              "Please select a new standard resource first." ) );
    return;
  }

  d->mCurrentManager->remove( confItem->resource() );

  if ( item == d->mLastItem ) {
    d->mLastItem = 0;
  }

  d->mListView->takeTopLevelItem( d->mListView->indexOfTopLevelItem( item ) );
  delete item;

  emit changed( true );
}

void ConfigPage::slotEdit()
{
  if ( !d->mCurrentManager ) {
    return;
  }

  QTreeWidgetItem *item = d->mListView->currentItem();
  ConfigViewItem *configItem = static_cast<ConfigViewItem*>( item );
  if ( !configItem ) {
    return;
  }

  Resource *resource = configItem->resource();

  ConfigDialog dlg( this, d->mFamily, resource );

  if ( dlg.exec() ) {
    configItem->setText( 0, resource->resourceName() );
    configItem->setText( 1, resource->type() );

    if ( configItem->standard() && configItem->readOnly() ) {
      KMessageBox::sorry( this, i18n( "You cannot use a read-only resource as standard." ) );
      configItem->setStandard( false );
    }

    d->mCurrentManager->change( resource );
    emit changed( true );
  }
}

void ConfigPage::slotStandard()
{
  if ( !d->mCurrentManager ) {
    return;
  }

  ConfigViewItem *item = static_cast<ConfigViewItem*>( d->mListView->currentItem() );
  if ( !item ) {
    return;
  }

  if ( item->readOnly() ) {
    KMessageBox::sorry( this, i18n( "You cannot use a read-only resource as standard." ) );
    return;
  }

  if ( !item->isOn() ) {
    KMessageBox::sorry( this, i18n( "You cannot use an inactive resource as standard." ) );
    return;
  }

  for ( int i = 0; i < d->mListView->topLevelItemCount(); ++i ) {
    ConfigViewItem *configItem = static_cast<ConfigViewItem*>( d->mListView->topLevelItem( i ) );
    if ( configItem->standard() ) {
      configItem->setStandard( false );
    }
  }

  item->setStandard( true );
  d->mCurrentManager->setStandardResource( item->resource() );

  emit changed( true );
}

void ConfigPage::slotSelectionChanged()
{
  bool state = ( d->mListView->currentItem() != 0 );

  d->mRemoveButton->setEnabled( state );
  d->mEditButton->setEnabled( state );
  d->mStandardButton->setEnabled( state );
}

void ConfigPage::resourceAdded( Resource *resource )
{
  kDebug() << resource->resourceName();

  ConfigViewItem *item = new ConfigViewItem( d->mListView, resource );

  item->setCheckState( 0, resource->isActive()? Qt::Checked : Qt::Unchecked );

  d->mLastItem = item;

  emit changed( true );
}

void ConfigPage::resourceModified( Resource *resource )
{
  kDebug() << resource->resourceName();
  ConfigViewItem *item = findItem( resource );
  if ( !item ) {
    return;
  }

  // TODO: Reread resource config. Otherwise we won't see the modification.

  item->updateItem();
}

void ConfigPage::resourceDeleted( Resource *resource )
{
  kDebug() << resource->resourceName();

  ConfigViewItem *item = findItem( resource );
  if ( !item ) {
    return;
  }

  delete item;
}

ConfigViewItem *ConfigPage::findItem( Resource *resource )
{
  for ( int i = 0; i < d->mListView->topLevelItemCount(); ++i ) {
    ConfigViewItem *item = static_cast<ConfigViewItem *>( d->mListView->topLevelItem( i ) );
    if ( item->resource() == resource ) {
      return item;
    }
  }
  return 0;
}

void ConfigPage::slotItemClicked( QTreeWidgetItem *item )
{
  ConfigViewItem *configItem = static_cast<ConfigViewItem *>( item );
  if ( !configItem ) {
    return;
  }

  if ( configItem->standard() && !configItem->isOn() ) {
    KMessageBox::sorry( this,
                        i18n( "You cannot deactivate the standard resource. "
                              "Choose another standard resource first." ) );
    configItem->setCheckState( 0, Qt::Checked );
    return;
  }

  if ( configItem->isOn() != configItem->resource()->isActive() ) {
    emit changed( true );
  }
}

void ConfigPage::Private::saveResourceSettings( ConfigPage *page )
{
  if ( mCurrentManager ) {
    for ( int i = 0; i < mListView->topLevelItemCount(); ++i ) {
      ConfigViewItem *configItem = static_cast<ConfigViewItem *>( mListView->topLevelItem( i ) );
      // check if standard resource
      if ( configItem->standard() && !configItem->readOnly() &&
           configItem->isOn() ) {
        mCurrentManager->setStandardResource( configItem->resource() );
      }

      // check if active or passive resource
      configItem->resource()->setActive( configItem->isOn() );
    }
    mCurrentManager->writeConfig( mCurrentConfig );

    if ( !mCurrentManager->standardResource() ) {
      KMessageBox::sorry( page,
                          i18n( "There is no valid standard resource. "
                                "Please select one which is neither read-only nor inactive." ) );
    }
  }
}

}

#include "configpage.moc"

