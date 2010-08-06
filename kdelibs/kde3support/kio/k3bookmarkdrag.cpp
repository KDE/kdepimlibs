//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "k3bookmarkdrag.h"
#include <k3urldrag.h>
#include <kdebug.h>
#include <Qt3Support/Q3CString>

K3BookmarkDrag * K3BookmarkDrag::newDrag( const Q3ValueList<KBookmark> & bookmarks, QWidget * dragSource, const char * name )
{
    KUrl::List urls;

    for ( Q3ValueListConstIterator<KBookmark> it = bookmarks.constBegin(); it != bookmarks.constEnd(); ++it ) {
       urls.append( (*it).url() );
    }

    // See KURLDrag::newDrag
    Q3StrList uris;
    KUrl::List::ConstIterator uit = urls.constBegin();
    KUrl::List::ConstIterator uEnd = urls.constEnd();
    // Get each URL encoded in utf8 - and since we get it in escaped
    // form on top of that, .toLatin1().constData() is fine.
    for ( ; uit != uEnd ; ++uit )
        uris.append( K3URLDrag::urlToString(*uit).toLatin1() );

    return new K3BookmarkDrag( bookmarks, uris, dragSource, name );
}

K3BookmarkDrag * K3BookmarkDrag::newDrag( const KBookmark & bookmark, QWidget * dragSource, const char * name )
{
    Q3ValueList<KBookmark> bookmarks;
    bookmarks.append( KBookmark(bookmark) );
    return newDrag(bookmarks, dragSource, name);
}

K3BookmarkDrag::K3BookmarkDrag( const Q3ValueList<KBookmark> & bookmarks, const Q3StrList & urls,
                  QWidget * dragSource, const char * name )
    : Q3UriDrag( urls, dragSource, name ), m_bookmarks( bookmarks ), m_doc("xbel")
{
    // We need to create the XML for this drag right now and not
    // in encodedData because when cutting a folder, the children
    // wouldn't be part of the bookmarks anymore, when encodedData
    // is requested.
    QDomElement elem = m_doc.createElement("xbel");
    m_doc.appendChild( elem );
    for ( Q3ValueListConstIterator<KBookmark> it = bookmarks.begin(); it != bookmarks.end(); ++it ) {
       elem.appendChild( (*it).internalElement().cloneNode( true /* deep */ ) );
    }
    //kDebug(7043) << "K3BookmarkDrag::K3BookmarkDrag " << m_doc.toString();
}

const char* K3BookmarkDrag::format( int i ) const
{
    if ( i == 0 )
        return "application/x-xbel";
    else if ( i == 1 )
	return "text/uri-list";
    else if ( i == 2 )
	return "text/plain";
    else return 0;
}

QByteArray K3BookmarkDrag::encodedData( const char* mime ) const
{
    QByteArray a;
    Q3CString mimetype( mime );
    if ( mimetype == "text/uri-list" )
        return Q3UriDrag::encodedData( mime );
    else if ( mimetype == "application/x-xbel" )
    {
        a = m_doc.toByteArray();
        //kDebug(7043) << "K3BookmarkDrag::encodedData " << m_doc.toCString();
    }
    else if ( mimetype == "text/plain" )
    {
        KUrl::List m_lstDragURLs;
        if ( K3URLDrag::decode( this, m_lstDragURLs ) )
        {
            QStringList uris;
            KUrl::List::ConstIterator uit = m_lstDragURLs.constBegin();
            KUrl::List::ConstIterator uEnd = m_lstDragURLs.constEnd();
            for ( ; uit != uEnd ; ++uit )
                uris.append( (*uit).prettyUrl() );

            Q3CString s = uris.join( "\n" ).toLocal8Bit();
            a.resize( s.length() + 1 ); // trailing zero
            memcpy( a.data(), s.data(), s.length() + 1 );
        }
    }
    return a;
}

bool K3BookmarkDrag::canDecode( const QMimeSource * e )
{
    return e->provides("text/uri-list") || e->provides("application/x-xbel") ||
           e->provides("text/plain");
}

Q3ValueList<KBookmark> K3BookmarkDrag::decode( const QMimeSource * e )
{
    Q3ValueList<KBookmark> bookmarks;
    if ( e->provides("application/x-xbel") )
    {
        QByteArray s( e->encodedData("application/x-xbel") );
        //kDebug(7043) << "K3BookmarkDrag::decode s=" << QCString(s);
        QDomDocument doc;
        doc.setContent( s );
        QDomElement elem = doc.documentElement();
        QDomNodeList children = elem.childNodes();
        for ( int childno = 0; childno < children.count(); childno++) 
        {
           bookmarks.append( KBookmark( children.item(childno).cloneNode(true).toElement() ));
        }
        return bookmarks;
    }
    if ( e->provides("text/uri-list") )
    {
        KUrl::List m_lstDragURLs;
        //kDebug(7043) << "K3BookmarkDrag::decode uri-list";
        if ( K3URLDrag::decode( e, m_lstDragURLs ) )
        {
            KUrl::List::ConstIterator uit = m_lstDragURLs.constBegin();
            KUrl::List::ConstIterator uEnd = m_lstDragURLs.constEnd();
            for ( ; uit != uEnd ; ++uit )
            {
                //kDebug(7043) << "K3BookmarkDrag::decode url=" << (*uit).url();
                bookmarks.append( KBookmark::standaloneBookmark( 
                                        (*uit).prettyUrl(), (*uit) ));
            }
            return bookmarks;
        }
    }
    if( e->provides("text/plain") )
    {        
        //kDebug(7043) << "K3BookmarkDrag::decode text/plain";
        QString s;
        if(Q3TextDrag::decode( e, s ))
        {
            
            QStringList listDragURLs = s.split(QChar('\n'), QString::SkipEmptyParts);
            QStringList::ConstIterator it = listDragURLs.constBegin();
            QStringList::ConstIterator end = listDragURLs.constEnd();
            for( ; it!=end; ++it)
            {
                //kDebug(7043)<<"K3BookmarkDrag::decode string"<<(*it);
                bookmarks.append( KBookmark::standaloneBookmark( KUrl(*it).prettyUrl(), KUrl(*it)));
            }
            return bookmarks;
        }
    }
    bookmarks.append( KBookmark() );
    return bookmarks;
}
