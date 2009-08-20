/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_SCHEDULER_H
#define KCAL_SCHEDULER_H

#include "kcal_export.h"

#include <QtCore/QString>
#include <QtCore/QList>

namespace KCal {

/**
   iTIP methods.
*/
enum iTIPMethod {
  iTIPPublish,       /**< Event, to-do, journal or freebusy posting */
  iTIPRequest,       /**< Event, to-do or freebusy scheduling request */
  iTIPReply,         /**< Event, to-do or freebusy reply to request */
  iTIPAdd,           /**< Event, to-do or journal additional property request */
  iTIPCancel,        /**< Event, to-do or journal cancellation notice */
  iTIPRefresh,       /**< Event or to-do description update request */
  iTIPCounter,       /**< Event or to-do submit counter proposal */
  iTIPDeclineCounter,/**< Event or to-do decline a counter proposal */
  iTIPNoMethod       /**< No method */
};

class IncidenceBase;
class Calendar;
class ICalFormat;
class FreeBusyCache;

/**
  @brief
  A Scheduling message class.

  This class provides an encapsulation of a scheduling message.
  It associates an incidence with an iTIPMethod and status information.
*/
class KCAL_EXPORT ScheduleMessage
{
  public:
    /**
      Message status.
    */
    enum Status {
      PublishNew,      /**< New message posting */
      PublishUpdate,   /**< Updated message */
      Obsolete,        /**< obsolete */
      RequestNew,      /**< Request new message posting */
      RequestUpdate,   /**< Request updated message */
      Unknown          /**< No status */
    };

    /**
      Creates a scheduling message with method as defined in iTIPMethod
      and a status.
    */
    ScheduleMessage( IncidenceBase *incidence, iTIPMethod method, Status status );

    /**
      Destructor.
    */
    ~ScheduleMessage();

    /**
      Returns the event associated with this message.
    */
    IncidenceBase *event();

    /**
      Returns the iTIP method associated with this message.
    */
    iTIPMethod method();

    /**
      Returns the status of this message.
    */
    Status status();

    /**
      Returns a human-readable name for an iTIP message status.
    */
    static QString statusName( Status status );

    /**
      Returns the error message if there is any.
    */
    QString error();

  private:
    Q_DISABLE_COPY( ScheduleMessage )
    class Private;
    Private *const d;
};

/**
  This class provides an encapsulation of iTIP transactions (RFC 2446).
  It is an abstract base class for inheritance by implementations of the
  iTIP scheme like iMIP or iRIP.
*/
class KCAL_EXPORT Scheduler
{
  public:
    /**
      Creates a scheduler for calendar specified as argument.
    */
    explicit Scheduler( Calendar *calendar );
    virtual ~Scheduler();

    /**
      iTIP publish action
    */
    virtual bool publish( IncidenceBase *incidence,
                          const QString &recipients ) = 0;
    /**
      Performs iTIP transaction on incidence. The method is specified as the
      method argument and can be any valid iTIP method.

      @param incidence the incidence for the transaction.
      @param method the iTIP transaction method to use.
    */
    virtual bool performTransaction( IncidenceBase *incidence,
                                     iTIPMethod method ) = 0;

    /**
      Performs iTIP transaction on incidence to specified recipient(s). The
      method is specified as the method argumanet and can be any valid iTIP
      method.

      @param incidence the incidence for the transaction.
      @param method the iTIP transaction method to use.
      @param recipients the receipients of the transaction.
    */
    virtual bool performTransaction( IncidenceBase *incidence,
                                     iTIPMethod method,
                                     const QString &recipients ) = 0;

    /**
      Retrieves incoming iTIP transactions.
    */
    virtual QList<ScheduleMessage*> retrieveTransactions() = 0;

    /**
      @deprecated: Use the other acceptTransaction() instead
      KDE5: Remove me, make email an optional argument in the other overload
     */
    bool KDE_DEPRECATED acceptTransaction( IncidenceBase *incidence, iTIPMethod method,
                                           ScheduleMessage::Status status );

    /**
      Accepts the transaction. The incidence argument specifies the iCal
      component on which the transaction acts. The status is the result of
      processing a iTIP message with the current calendar and specifies the
      action to be taken for this incidence.

      @param incidence the incidence for the transaction.
      @param method iTIP transaction method to check.
      @param status scheduling status.
      @param email the email address of the person for whom this
      transaction is to be performed.
    */
    bool acceptTransaction( IncidenceBase *incidence,
                            iTIPMethod method,
                            ScheduleMessage::Status status,
                            const QString &email );

    /**
      Returns a machine-readable name for a iTIP method.
    */
    static QString methodName( iTIPMethod method );

    /**
      Returns a translated human-readable name for a iTIP method.
    */
    static QString translatedMethodName( iTIPMethod method );

    virtual bool deleteTransaction( IncidenceBase *incidence );

    /**
      Returns the directory where the free-busy information is stored.
    */
    virtual QString freeBusyDir() = 0;

    /**
      Sets the free/busy cache used to store free/busy information.
    */
    void setFreeBusyCache( FreeBusyCache * );

    /**
      Returns the free/busy cache.
    */
    FreeBusyCache *freeBusyCache() const;

  protected:
    bool acceptPublish( IncidenceBase *, ScheduleMessage::Status status, iTIPMethod method );
    /**
      @deprecated: Use the other overload instead
      KDE5: remove me
     */
    bool KDE_DEPRECATED acceptRequest( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptRequest( IncidenceBase *, ScheduleMessage::Status status,
                        const QString &email );
    bool acceptAdd( IncidenceBase *, ScheduleMessage::Status status );
    KDE_DEPRECATED bool acceptCancel( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptCancel( IncidenceBase *, ScheduleMessage::Status status,
                       const QString & attendee );
    bool acceptDeclineCounter( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptReply( IncidenceBase *, ScheduleMessage::Status status, iTIPMethod method );
    bool acceptRefresh( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptCounter( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptFreeBusy( IncidenceBase *, iTIPMethod method );

    Calendar *mCalendar;
    ICalFormat *mFormat;

  private:
    Q_DISABLE_COPY( Scheduler )
    struct Private;
    Private *const d;
};

}

#endif
