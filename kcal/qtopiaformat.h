/*
    This file is part of the kcal library.

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
#ifndef KCAL_QTOPIAFORMAT_H
#define KCAL_QTOPIAFORMAT_H

#include <QtCore/QString>

#include "scheduler.h"

#include "kcal_export.h"
#include "calformat.h"

namespace KCal {

/**
  This class implements the calendar format used by Qtopia.
*/
class KCAL_EXPORT QtopiaFormat : public CalFormat
{
  public:
    QtopiaFormat();
    virtual ~QtopiaFormat();

    bool load( Calendar *calendar, const QString &fileName );
    bool save( Calendar *calendar, const QString &fileName );

    bool fromString( Calendar *calendar, const QString & );
    bool fromRawString( Calendar *calendar, const QByteArray &string );
    QString toString( Calendar *calendar );

  private:
    Q_DISABLE_COPY( QtopiaFormat )
    class Private;
    Private *const d;
};

}

#endif
