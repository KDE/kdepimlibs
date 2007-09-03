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

#include <QtCore/QString>
#include <QtCore/QList>

#include "kcal_export.h"

namespace KCal {

class IncidenceBase;
class Calendar;
class ICalFormat;
class FreeBusyCache;

/**
  This class provides an encapsulation of a scheduling message. It associates an
  incidence with a method and status information. This class is used by the
  Scheduler class.

  @short A Scheduling message
*/
class KCAL_EXPORT ScheduleMessage
{
  public:
    /**
      Message status.
    */
    enum Status {
      PublishNew,      /**< */
      PublishUpdate,   /**< */
      Obsolete,        /**< */
      RequestNew,      /**< */
      RequestUpdate,   /**< */
      Unknown          /**< */
    };

    /**
      Creates a scheduling message with method as defined in Scheduler::Method
      and a status.
    */
    ScheduleMessage( IncidenceBase *, int method, Status status );
    ~ScheduleMessage() {}

    /**
      Returns the event associated with this message.
    */
    IncidenceBase *event();

    /**
      Returns the iTIP method associated with this message.
    */
    int method();

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
    IncidenceBase *mIncidence;
    int mMethod;
    Status mStatus;
    QString mError;

    struct Private;
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
      iTIP methods.
    */
    enum Method {
      Publish,         /**< Event, to-do, journal or freebusy posting */
      Request,         /**< Event, to-do or freebusy scheduling request */
      Reply,           /**< Event, to-do or freebusy reply to request */
      Add,             /**< Event, to-do or journal additional properties request */
      Cancel,          /**< Event, to-do or journal cancellation notice */
      Refresh,         /**< Event or to-do description update request */
      Counter,         /**< Event or to-do description counter proposal submission */
      Declinecounter,  /**< Event or to-do decline a counter proposal */
      NoMethod         /**< No method */
    };

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

      @param incidence the incidence for the transaction
      @param method the iTIP transaction method to use
    */
    virtual bool performTransaction( IncidenceBase *incidence,
                                     Method method ) = 0;

    /**
      Performs iTIP transaction on incidence to specified recipient(s). The
      method is specified as the method argumanet and can be any valid iTIP
      method.

      @param incidence the incidence for the transaction
      @param method the iTIP transaction method to use
      @param recipients the receipients of the transaction
    */
    virtual bool performTransaction( IncidenceBase *incidence, Method method,
                                     const QString &recipients ) = 0;

    /**
      Retrieves incoming iTIP transactions.
    */
    virtual QList<ScheduleMessage*> retrieveTransactions() = 0;

    /**
      Accepts the transaction. The incidence argument specifies the iCal
      component on which the transaction acts. The status is the result of
      processing a iTIP message with the current calendar and specifies the
      action to be taken for this incidence.

      @param method iTIP transaction method to check
      @param status scheduling status
    */
    bool acceptTransaction( IncidenceBase *, Method method,
                            ScheduleMessage::Status status );

    /**
      Returns a machine-readable name for a iTIP method.
    */
    static QString methodName( Method );

    /**
      Returns a translated human-readable name for a iTIP method.
    */
    static QString translatedMethodName( Method );

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
    bool acceptPublish( IncidenceBase *, ScheduleMessage::Status status, Method method );
    bool acceptRequest( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptAdd( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptCancel( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptDeclineCounter( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptReply( IncidenceBase *, ScheduleMessage::Status status, Method method );
    bool acceptRefresh( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptCounter( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptFreeBusy( IncidenceBase *, Method method );

    Calendar *mCalendar;
    ICalFormat *mFormat;

  private:
    struct Private;
    Private *const d;
};

}

#endif
