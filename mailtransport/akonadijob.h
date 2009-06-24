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

#ifndef MAILTRANSPORT_AKONADIJOB_H
#define MAILTRANSPORT_AKONADIJOB_H

#include <mailtransport/transportjob.h>

#include <akonadi/item.h>

class AkonadiJobPrivate;

namespace MailTransport {

/**
  Mail transport job for an Akonadi resource.

  TODO docu... apps need to call setItem if it's an Akonadi job...
*/
class MAILTRANSPORT_EXPORT AkonadiJob : public TransportJob
{
  Q_OBJECT
  public:
    /**
      Creates an AkonadiJob.
      @param transport The transport settings.
      @param parent The parent object.
    */
    explicit AkonadiJob( Transport *transport, QObject *parent = 0 );

    /**
      Destroys this job.
    */
    virtual ~AkonadiJob();

    // TODO have these here or in TransportJob?  They are useless in {SMTP,Sendmail}Job...
    /**
      The id of the item to send.
    */
    Akonadi::Item::Id itemId() const;

    /**
      Set the id of the item to send.
      @param itemId id of the item to send
    */
    void setItemId( Akonadi::Item::Id id );

  protected:
    virtual void doStart();

  private Q_SLOTS:
    void resourceResult( bool success, const QString &message );

  private:
    AkonadiJobPrivate *const d;

};

}

#endif
