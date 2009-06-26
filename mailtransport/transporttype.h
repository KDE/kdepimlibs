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

#ifndef MAILTRANSPORT_TRANSPORTTYPE_H
#define MAILTRANSPORT_TRANSPORTTYPE_H

#include "mailtransport_export.h"
#include "transport.h"

#include <QtCore/QString>

#include <akonadi/agenttype.h>

namespace MailTransport {

class AddTransportDialog;
class TransportManager;

/**
  A representation of a transport type.

  TODO docu look at Akonadi::AgentType
*/
class MAILTRANSPORT_EXPORT TransportType
{
  friend class AddTransportDialog;
  friend class Transport;
  friend class TransportManager;

  public:
    typedef QList<TransportType> List;

    TransportType();
    TransportType( const TransportType &other );
    ~TransportType();
    bool operator==( const TransportType &other ) const;

    bool isValid() const;

    // TODO should this be Transport::EnumType::type instead of int?
    int type() const;
    QString name() const;
    QString description() const;

    /// only valid if this is an Akonadi transport
    Akonadi::AgentType agentType() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;

};

}

Q_DECLARE_METATYPE( MailTransport::TransportType )

#endif
