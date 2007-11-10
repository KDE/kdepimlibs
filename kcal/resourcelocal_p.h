/*
    This file is part of the kcal library.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KCAL_RESOURCELOCAL_P_H
#define KCAL_RESOURCELOCAL_P_H

#include <kdatetime.h>
#include <kurl.h>
#include <kdirwatch.h>

namespace KABC { class Lock; }

namespace KCal {

class CalFormat;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class ResourceLocal::Private
{
  public:
    KUrl mURL;
    CalFormat *mFormat;
    KDirWatch mDirWatch;
    KABC::Lock *mLock;
    KDateTime mLastModified;
};
//@endcond

}

#endif
