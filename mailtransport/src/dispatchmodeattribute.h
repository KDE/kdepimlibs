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

#ifndef MAILTRANSPORT_DISPATCHMODEATTRIBUTE_H
#define MAILTRANSPORT_DISPATCHMODEATTRIBUTE_H

#include <mailtransport_export.h>

#include <QtCore/QDateTime>

#include <attribute.h>

namespace MailTransport
{

/**
  Attribute determining how and when a message from the outbox should be
  dispatched.  Messages can be sent immediately, sent only when the user
  explicitly requests it, or sent automatically at a certain date and time.

  @author Constantin Berzan <exit3219@gmail.com>
  @since 4.4
*/
class MAILTRANSPORT_EXPORT DispatchModeAttribute : public Akonadi::Attribute
{
public:
    /**
      Determines how the message is sent.
    */
    enum DispatchMode {
        Automatic,    ///< Send message as soon as possible, but no earlier than
        ///  specified by setSendAfter()
        Manual        ///< Send message only when the user requests so.
    };

    /**
      Creates a new DispatchModeAttribute.
    */
    explicit DispatchModeAttribute(DispatchMode mode = Automatic);

    /**
      Destroys the DispatchModeAttribute.
    */
    virtual ~DispatchModeAttribute();

    /* reimpl */
    virtual DispatchModeAttribute *clone() const;
    virtual QByteArray type() const;
    virtual QByteArray serialized() const;
    virtual void deserialize(const QByteArray &data);

    /**
      Returns the dispatch mode for the message.
      @see DispatchMode.
    */
    DispatchMode dispatchMode() const;

    /**
      Sets the dispatch mode for the message.
      @param mode the dispatch mode to set
      @see DispatchMode.
    */
    void setDispatchMode(DispatchMode mode);

    /**
      Returns the date and time when the message should be sent.
      Only valid if dispatchMode() is Automatic.
    */
    QDateTime sendAfter() const;

    /**
      Sets the date and time when the message should be sent.
      @param date the date and time to set
      @see setDispatchMode.
    */
    void setSendAfter(const QDateTime &date);

private:
    class Private;
    Private *const d;

};

} // namespace MailTransport

#endif // MAILTRANSPORT_DISPATCHMODEATTRIBUTE_H
