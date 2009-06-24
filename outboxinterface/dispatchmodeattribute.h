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

#ifndef OUTBOXINTERFACE_DISPATCHMODEATTRIBUTE_H
#define OUTBOXINTERFACE_DISPATCHMODEATTRIBUTE_H

#include <outboxinterface/outboxinterface_export.h>

#include <QtCore/QDateTime>

#include <akonadi/attribute.h>

namespace OutboxInterface {

/**
  Attribute determining how and when a message from the outbox should be
  dispatched.  Messages can be sent immediately, sent only when the user
  explicitly requests it, or sent automatically at a certain date and time.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class OUTBOXINTERFACE_EXPORT DispatchModeAttribute : public Akonadi::Attribute
{
  public:
    /**
      Determines how the message is sent.
    */
    enum DispatchMode
    {
      Immediately,  ///< Send message as soon as possible.
      AfterDueDate, ///< Send message at a certain date/time.
      Never         ///< Send message only when the user requests so.
    };

    /**
      Creates a new DispatchModeAttribute.
    */
    explicit DispatchModeAttribute( DispatchMode mode = Immediately, const QDateTime &date = QDateTime() );

    /**
      Destroys the DispatchModeAttribute.
    */
    virtual ~DispatchModeAttribute();

    /* reimpl */
    virtual DispatchModeAttribute* clone() const;
    virtual QByteArray type() const;
    virtual QByteArray serialized() const;
    virtual void deserialize( const QByteArray &data );

    /**
      Returns the dispatch mode for the message.
      @see DispatchMode.
    */
    DispatchMode dispatchMode() const;

    /**
      Sets the dispatch mode for the message.
      @see DispatchMode.
    */
    void setDispatchMode( DispatchMode mode );

    /**
      Returns the date and time when the message should be sent.
      Only valid if dispatchMode() is AfterDueDate.
    */
    QDateTime dueDate() const;

    /**
      Sets the date and time when the message should be sent.
      Make sure you set the DispatchMode to AfterDueDate first.
      @see setDispatchMode.
    */
    void setDueDate( const QDateTime &date );

  private:
    DispatchMode mMode;
    QDateTime mDueDate;

};

} // namespace OutboxInterface

#endif // OUTBOXINTERFACE_DISPATCHMODEATTRIBUTE_H
