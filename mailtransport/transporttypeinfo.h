/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MAILTRANSPORT_TRANSPORTTYPEINFO_H
#define MAILTRANSPORT_TRANSPORTTYPEINFO_H

#include <QString>

class QObject;
class QWidget;

namespace MailTransport {

class Transport;
class TransportConfigWidget;
class TransportJob;

/**
  A class providing central information about the types supported by
  Mailtransport.  It avoids having to switch(type) all over the place.

  TODO export?

  TODO docu
*/
class TransportTypeInfo
{
  public:
    static int typeCount();

    static QString nameForType( int type );
    static QString descriptionForType( int type );
    static TransportJob *jobForTransport( Transport *transport, QObject *parent = 0 );
    static TransportConfigWidget *configWidgetForTransport( Transport *transport, QWidget *parent = 0 );

  private:
    TransportTypeInfo();
    ~TransportTypeInfo();

};

}

#endif
