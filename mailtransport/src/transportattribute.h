/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MAILTRANSPORT_TRANSPORTATTRIBUTE_H
#define MAILTRANSPORT_TRANSPORTATTRIBUTE_H

#include <mailtransport_export.h>

#include <attribute.h>

namespace MailTransport
{

class Transport;

/**
  Attribute determining which transport to use for sending a message.

  @see mailtransport
  @see TransportManager.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_EXPORT TransportAttribute : public Akonadi::Attribute
{
public:
    /**
      Creates a new TransportAttribute.
    */
    explicit TransportAttribute(int id = -1);

    /**
      Destroys this TransportAttribute.
    */
    virtual ~TransportAttribute();

    /* reimpl */
    virtual TransportAttribute *clone() const;
    virtual QByteArray type() const;
    virtual QByteArray serialized() const;
    virtual void deserialize(const QByteArray &data);

    /**
      Returns the transport id to use for sending this message.
      @see TransportManager.
    */
    int transportId() const;

    /**
      Returns the transport object corresponding to the transport id contained
      in this attribute.
      @see Transport.
    */
    Transport *transport() const;

    /**
      Sets the transport id to use for sending this message.
    */
    void setTransportId(int id);

private:
    class Private;
    Private *const d;

};

} // namespace MailTransport

#endif // MAILTRANSPORT_TRANSPORTATTRIBUTE_H
