/*
    This file is part of libkdepim.

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
#include <kdebug.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "kresources/resource.h"
#include "kresources/manager.h"

using namespace KRES;

class TestResource : public Resource
{
  public:
    TestResource() : Resource() {}

};

class TestSubResource : public TestResource
{
  public:
    TestSubResource() : TestResource() {}

    void dump() const
    {
      kDebug() << "TestSubResource" << endl;
      TestResource::dump();
    }
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "testresources", 0, ki18n("Kresource Test"), "0" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  Manager<TestResource> manager( "test" );

  TestResource *resource1 = new TestResource;
  resource1->setResourceName( "One" );
  manager.add( resource1 );

  TestResource *resource2 = new TestSubResource;
  resource2->setResourceName( "Two" );
  manager.add( resource2 );

  TestResource *resource3 = new TestSubResource;
  resource3->setResourceName( "Three" );
  manager.add( resource3 );

  kDebug() << "LIST ALL:" << endl;
  Manager<TestResource>::Iterator it;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    (*it)->dump();
  }

  resource2->setActive( false );
  resource3->setActive( true );

  kDebug() << "LIST ACTIVE" << endl;
  Manager<TestResource>::ActiveIterator it2;
  for( it2 = manager.activeBegin(); it2 != manager.activeEnd(); ++it2 ) {
    (*it2)->dump();
  }

  resource1->setActive( false );
  resource2->setActive( true );
  resource3->setActive( true );

  kDebug() << "LIST ACTIVE" << endl;
  for( it2 = manager.activeBegin(); it2 != manager.activeEnd(); ++it2 ) {
    (*it2)->dump();
  }

  kDebug() << "LIST ALL" << endl;
  for( it = manager.begin(); it != manager.end(); ++it ) {
    (*it)->dump();
  }


}
