/*
  Copyright (c) 2010 Kevin Funk <kevin.funk@kdab.com>

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

#include "networkaccesshelper.h"

// from komo-kdelibs
#include <solid/networkingsession.h>
#include <solid/networking.h>

using namespace KPIMUtils;

namespace KPIMUtils {

class NetworkAccessHelperPrivate
{
  public:
    NetworkAccessHelperPrivate( NetworkAccessHelper *helper );
    ~NetworkAccessHelperPrivate();

    Solid::NetworkingSession *m_session;
};

}

NetworkAccessHelperPrivate::NetworkAccessHelperPrivate( NetworkAccessHelper *helper )
  : m_session( new Solid::NetworkingSession( helper ) )
{
}

NetworkAccessHelperPrivate::~NetworkAccessHelperPrivate()
{
  delete m_session;
}

NetworkAccessHelper::NetworkAccessHelper( QObject *parent )
  : QObject(parent),
    d_ptr( new NetworkAccessHelperPrivate( this ) )
{
}

NetworkAccessHelper::~NetworkAccessHelper()
{
  delete d_ptr;
}

void NetworkAccessHelper::establishConnection()
{
  Q_D(NetworkAccessHelper);
  d->m_session->establishConnection();
}

void NetworkAccessHelper::releaseConnection()
{
  Q_D(NetworkAccessHelper);
  d->m_session->releaseConnection();
}

#include "networkaccesshelper.moc"
