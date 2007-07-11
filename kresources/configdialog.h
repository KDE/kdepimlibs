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
  ConfigDialog class.

  @author Tobias Koenig
  @author Jan-Pascal van Best
*/

#ifndef KRESOURCES_CONFIGDIALOG_H
#define KRESOURCES_CONFIGDIALOG_H

#include "kresources_export.h"
#include <kdialog.h>

namespace KRES {
  class Resource;

/**
  @brief
  A dialog for configuring a resource.

  This class provides a resource configuration dialog.
*/
class KRESOURCES_EXPORT ConfigDialog : public KDialog
{
    Q_OBJECT
  public:
    // Resource=0: create new resource
    ConfigDialog( QWidget *parent, const QString &resourceFamily,
                  Resource *resource );

    virtual ~ConfigDialog();
    void setInEditMode( bool value );

  protected Q_SLOTS:
    void accept();
    void setReadOnly( bool value );
    void slotNameChanged( const QString &text );

  private:
    class Private;
    Private *const d;
};

}

#endif
