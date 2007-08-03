/*
  This file is part of the kblog library.

  Copyright (c) 2006-2007 Christian Weilbach <christian_weilbach@web.de>

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

#ifndef KBLOG_TEST_DATA_H_
#define KBLOG_TEST_DATA_H_

#include <QtCore/QString>
#include <kurl.h>
#include <ktimezone.h>
#include <kdatetime.h>

  KUrl mUrl( "http://soctest.wordpress.com/xmlrpc.php" );
  QString mUsername( "socapitest" );
  QString mPassword( "k0nt4ctbl0g" );
  QString mBlogId( "1" );

  QString mTitle( "TestBlogger1" );
  QString mContent( "TestBlogger1: <strong>posted</strong> content." );
  QString mModifiedContent( "TestBlogger1: <strong>modified</strong> <em>content</em>." );
  bool mPublished = true;
  QString mPostingId( QString( "113" ) );

#endif

