/*
    This file is part of libkresources.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
  ConfigWidget abstract base class.

  @author Tobias Koenig
  @author Jan-Pascal van Best
*/

#ifndef KRESOURCES_CONFIGWIDGET_H
#define KRESOURCES_CONFIGWIDGET_H

#include <QtGui/QWidget>

#include <kconfig.h>

#include "resource.h"

namespace KRES {

/**
  @brief
  An abstract base class for deriving resource configuration dialogs.

  This class provides an abstract base class for configuration dialog.
*/
class KRESOURCES_EXPORT ConfigWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit ConfigWidget( QWidget *parent = 0 );

    /**
      Sets the widget to 'edit' mode. Reimplement this method if you are
      interested in the mode change (to disable some GUI element for
      example). By default the widget is in 'create new' mode.
    */
    virtual void setInEditMode( bool value );

  public Q_SLOTS:
    virtual void loadSettings( Resource *resource ) = 0;
    virtual void saveSettings( Resource *resource ) = 0;

  Q_SIGNALS:
    void setReadOnly( bool value );

  protected:
    Resource *mResource;
};

}
#endif
