/*
  This file is part of the kcal library.

  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCECACHEDCONFIG_H
#define KCAL_RESOURCECACHEDCONFIG_H

#include <QtGui/QWidget>
#include "kcal_export.h"

namespace KCal {

class ResourceCached;

/**
  Configuration widget for reload policy

  @see ResourceCached
*/
class KCAL_EXPORT ResourceCachedReloadConfig : public QWidget
{
  Q_OBJECT
  public:
    explicit ResourceCachedReloadConfig( QWidget *parent = 0 );
    ~ResourceCachedReloadConfig();
  public Q_SLOTS:
    void loadSettings( ResourceCached *resource );
    void saveSettings( ResourceCached *resource );

  protected Q_SLOTS:
    void slotIntervalToggled( bool );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( ResourceCachedReloadConfig )
    class Private;
    Private *const d;
    //@endcond
};

/**
  Configuration widget for save policy

  @see ResourceCached
*/
class KCAL_EXPORT ResourceCachedSaveConfig : public QWidget
{
    Q_OBJECT
  public:
    explicit ResourceCachedSaveConfig( QWidget *parent = 0 );
    ~ResourceCachedSaveConfig();

  public Q_SLOTS:
    void loadSettings( ResourceCached *resource );
    void saveSettings( ResourceCached *resource );

  protected Q_SLOTS:
    void slotIntervalToggled( bool );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( ResourceCachedSaveConfig )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
