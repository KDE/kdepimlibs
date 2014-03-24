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

#ifndef NETWORKACCESSHELPER_H
#define NETWORKACCESSHELPER_H

#include "kpimutils_export.h"

#include <QtCore/QObject>

namespace KPIMUtils {

class NetworkAccessHelperPrivate;

/**
 * Wrapper around Solid::NetworkingControl (komobranch)
 *
 * This can be used for all platforms, but is just implemented for Windows CE
 * Does nothing on other platforms
 *
 * Basically this is to prevent ifdef'ing all the classes that make use of
 * the NetworkingControl class that is only available in kdelibs from the
 * komobranch at the moment
 *
 * @deprecated This class is non-functional and will be removed in KF 5.
 */
class KPIMUTILS_DEPRECATED_EXPORT NetworkAccessHelper
  : public QObject
{
  Q_OBJECT
  Q_DECLARE_PRIVATE( NetworkAccessHelper )

  public:
    explicit NetworkAccessHelper( QObject *parent = 0 );
    virtual ~NetworkAccessHelper();

    void establishConnection();
    void releaseConnection();

  private:
    NetworkAccessHelperPrivate *d_ptr;
};

}

#endif // NETWORKACCESSHELPER_H
