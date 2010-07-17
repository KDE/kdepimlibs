/*
  This file is part of the kcalcore library.

  Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalFilter class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/

#ifndef KCALCORE_CALFILTER_H
#define KCALCORE_CALFILTER_H

#include "kcalcore_export.h"
#include "event.h"
#include "journal.h"
#include "todo.h"

namespace KCalCore {

/**
  @brief
  Provides a filter for calendars.

  This class provides a means for filtering calendar incidences by
  a list of email addresses, a list of categories, or other #Criteria.

  The following #Criteria are available:
  - remove recurring Incidences
  - keep Incidences with a matching category (see setCategoryList())
  - remove completed To-dos (see setCompletedTimeSpan())
  - remove inactive To-dos
  - remove To-dos without a matching attendee (see setEmailList())
*/
class KCALCORE_EXPORT CalFilter
{
  public:
    /**
      Filtering Criteria.
    */
    enum Criteria {
      HideRecurring = 1,     /**< Remove incidences that recur */
      HideCompletedTodos = 2,/**< Remove completed to-dos */
      ShowCategories = 4,    /**< Show incidences with at least one matching category */
      HideInactiveTodos = 8, /**< Remove to-dos that haven't started yet */
      HideNoMatchingAttendeeTodos = 16 /**< Remove to-dos without a matching attendee */
    };

    /**
      Constructs an empty filter -- a filter without a name or criteria.
    */
    CalFilter();

    /**
      Constructs a filter with @p name.

      @param name is the name of this filter.
    */
    explicit CalFilter( const QString &name );

    /**
      Destroys this filter.
    */
    ~CalFilter();

    /**
      Sets the filter name.

      @param name is the name of this filter.
      @see name().
    */
    void setName( const QString &name );

    /**
      Returns the filter name.
      @see setName().
    */
    QString name() const;

    /**
      Sets the criteria which must be fulfilled for an Incidence to pass
      the filter.

      @param criteria is a combination of #Criteria.
      @see criteria().
    */
    void setCriteria( int criteria );

    /**
      Returns the inclusive filter criteria.
      @see setCriteria().
    */
    int criteria() const;

    /**
      Applies the filter to a list of Events. All events not matching the
      filter criteria are removed from the list.

      @param eventList is a list of Events to filter.
    */
    void apply( Event::List *eventList ) const;

    /**
      Applies the filter to a list of To-dos. All to-dos not matching the
      filter criterias are removed from the list.

      @param todoList is a list of To-dos to filter.
    */
    void apply( Todo::List *todoList ) const;

    /**
      Applies the filter to a list of Journals. All journals not matching the
      filter criterias are removed from the list.

      @param journalList is a list of Journals to filter.
    */
    void apply( Journal::List *journalList ) const;

    /**
      Applies the filter criteria to the specified Incidence.

      @param incidence is the Incidence to filter.
      @return true if the Incidence passes the criteria; false otherwise.
    */
    bool filterIncidence( Incidence::Ptr incidence ) const;

    /**
      Enables or disables the filter.

      @param enabled is true if the filter is to be enabled; false otherwise.
      @see isEnabled().
    */
    void setEnabled( bool enabled );

    /**
      Returns whether the filter is enabled or not.
      @see setEnabled().
    */
    bool isEnabled() const;

    /**
      Sets the list of categories to be considered when filtering incidences
      according to the #ShowCategories criteria.

      @param categoryList is a QStringList of categories.
      @see categoryList().
    */
    void setCategoryList( const QStringList &categoryList );

    /**
      Returns the category list for this filter.
      @see setCategoryList().
    */
    QStringList categoryList() const;

    /**
      Sets the list of email addresses to be considered when filtering
      incidences according ot the #HideNoMatchingAttendeeTodos criteria.

      @param emailList is a QStringList of email addresses.
      @see emailList().
    */
    void setEmailList( const QStringList &emailList );

    /**
      Returns the email list for this filter.
      @see setEmailList().
    */
    QStringList emailList() const;

    /**
      Sets the number of days for the #HideCompletedTodos criteria.
      If a to-do has been completed within the recent @p timespan days,
      then that to-do will be removed during filtering. If a time span is
      not specified in the filter, then all completed to-dos will be removed
      if the #HideCompletedTodos criteria is set.

      @param timespan is an integer representing a time span in days.
      @see completedTimeSpan().
     */
    void setCompletedTimeSpan( int timespan );

    /**
      Returns the completed time span for this filter.
      @see setCompletedTimeSpan()
     */
    int completedTimeSpan() const;

    /**
      Compares this with @p filter for equality.

      @param filter the CalFilter to compare.
    */
    bool operator==( const CalFilter &filter ) const;

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( CalFilter )
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
