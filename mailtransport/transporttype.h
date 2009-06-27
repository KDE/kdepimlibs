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
  @short A representation of a transport type.

  Represents an available transport type.  SMTP and Sendmail are available,
  as well as a number of Akonadi-based types.  Each Akonadi-based type
  corresponds to an Akonadi resource type that supports sending messages.

  This class provides information about the type, such as name and
  description.  Additionally, for Akonadi types, it provides the corresponding
  Akonadi AgentType.

  All available transport types can be retrieved via TransportManager::types().

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_EXPORT TransportType
{
  friend class AddTransportDialog;
  friend class Transport;
  friend class TransportManager;

  public:
    /**
      Describes a list of transport types.
    */
    typedef QList<TransportType> List;

    /**
      Constructs a new TransportType.
    */
    TransportType();

    /**
      Creates a copy of the @p other TransportType.
    */
    TransportType( const TransportType &other );

    /**
      Destroys the TransportType.
    */
    ~TransportType();

    /**
      @internal
    */
    TransportType& operator=( const TransportType &other );
    
    /**
      @internal
      Compares two transport types.
    */
    bool operator==( const TransportType &other ) const;

    /**
      Returns whether the transport type is valid.
    */
    bool isValid() const;

    /**
      @internal
      Returns the numeric type of the transport.  This corresponds to
      Transport::EnumType::type.
    */
    // TODO should this be Transport::EnumType::type instead of int?
    int type() const;

    /**
      Returns the i18n'ed name of the transport type.
    */
    QString name() const;

    /**
      Returns a description of the transport type.
    */
    QString description() const;

    /**
      Returns the corresponding Akonadi::AgentType that this transport type
      represents.  Only valid if type() is Transport::EnumType::Akonadi.
    */
    Akonadi::AgentType agentType() const;

  private:
    class Private;
    QSharedDataPointer<Private> d;

};

} // namespace MailTransport

Q_DECLARE_METATYPE( MailTransport::TransportType )

#endif // MAILTRANSPORT_TRANSPORTTYPE_H
