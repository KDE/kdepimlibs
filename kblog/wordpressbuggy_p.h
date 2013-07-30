/*
  This file is part of the kblog library.

  Copyright (c) 2007-2009 Christian Weilbach <christian_weilbach@web.de>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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

#ifndef WORDPRESSBUGGY_P_H
#define WORDPRESSBUGGY_P_H

#include "wordpressbuggy.h"
#include "movabletype_p.h"

#include <kxmlrpcclient/client.h>

class KJob;
class QByteArray;
template <class T,class S>class QMap;

namespace KIO
{
  class Job;
}

namespace KBlog {

class WordpressBuggyPrivate : public MovableTypePrivate
{
  public:
    QMap<KJob *,KBlog::BlogPost *> mCreatePostMap;
    QMap<KJob *,KBlog::BlogPost *> mModifyPostMap;
    WordpressBuggyPrivate();
    virtual ~WordpressBuggyPrivate();
    virtual QList<QVariant> defaultArgs( const QString &id = QString() );

    //adding these two lines prevents the symbols from MovableTypePrivate
    //to be hidden by the symbols below that.
    using MovableTypePrivate::slotCreatePost;
    using MovableTypePrivate::slotModifyPost;
    virtual void slotCreatePost( KJob * );
    virtual void slotModifyPost( KJob * );
    Q_DECLARE_PUBLIC( WordpressBuggy )
};

}

#endif
