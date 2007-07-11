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

  @author Tobias Koenig
  @author Jan-Pascal van Best
  @author Cornelius Schumacher
*/

#ifndef KRESOURCES_CONFIGPAGE_H
#define KRESOURCES_CONFIGPAGE_H

#include <QtCore/QStringList>
#include <QtGui/QWidget>
#include <QtCore/QList>

#include <ksharedptr.h>

#include "manager.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace KRES {

class KRESOURCES_EXPORT ResourcePageInfo : public KShared
{
  public:
    ResourcePageInfo();
    ~ResourcePageInfo();
    Manager<Resource> *mManager;
    KConfig *mConfig;

  private:
    class Private;
    Private *const d;
};

class Resource;
class ConfigViewItem;

/**
  @brief
  A resource configuration page.

  This class provides a page for a resource configuration dialog.
*/
class KRESOURCES_EXPORT ConfigPage : public QWidget, public ManagerObserver<Resource>
{
  Q_OBJECT

  public:
    ConfigPage( QWidget *parent = 0 );
    virtual ~ConfigPage();

    void load();
    void save();
    virtual void defaults();

  public Q_SLOTS:
    void slotFamilyChanged( int pos );
    void slotAdd();
    void slotRemove();
    void slotEdit();
    void slotStandard();
    void slotSelectionChanged();

  public:
    // From ManagerObserver<Resource>
    virtual void resourceAdded( Resource *resource );
    virtual void resourceModified( Resource *resource );
    virtual void resourceDeleted( Resource *resource );

  protected:
    ConfigViewItem *findItem( Resource *resource );

  protected Q_SLOTS:
    void slotItemClicked( QTreeWidgetItem *item );

  Q_SIGNALS:
    void changed( bool );

  private:
    class Private;
    Private *const d;
};

}

#endif
