/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KMIME_CONTENT_P_H
#define KMIME_CONTENT_P_H

//@cond PRIVATE

namespace KMime {

class ContentPrivate
{
  public:
    ContentPrivate( Content *q ) : forceDefaultCS( false ), q_ptr( q )
    {
      defaultCS = KMime::cachedCharset( "ISO-8859-1" );
    }

    ~ContentPrivate()
    {
      qDeleteAll( contents );
      contents.clear();
    }

    QByteArray head;
    QByteArray body;
    Content::List contents;
    QByteArray defaultCS;
    bool forceDefaultCS;

    Content* q_ptr;
    Q_DECLARE_PUBLIC( Content )
};

}

//@endcond

#endif
