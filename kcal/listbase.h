/*
  This file is part of the kcal library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
  defines the ListBase class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCAL_LISTBASE_H
#define KCAL_LISTBASE_H

#include "kcal_export.h"
#include <QtCore/QList>

namespace KCal {

/**
  @brief
  This class provides a template for lists of pointers.

  It extends QList<T *> with an "auto-delete" functionality.
*/
template<class T>
class ListBase : public QList<T *>
{
  public:
    /**
      Constructor.
    */
    ListBase()
      : QList<T *>(), mAutoDelete( false )
    {
    }

    /**
      Copy constructor.
      @param other is the ListBase to copy.
    */
    ListBase( const ListBase &other )
      : QList<T *>( other ), mAutoDelete( false )
    {
    }

    /**
      Destructor.
    */
    ~ListBase()
    {
      if ( mAutoDelete ) {
        qDeleteAll( *this );
      }
    }

    /**
      Assigns @p l to this listbase.
      @param l is the ListBase to copy.
    */
    ListBase &operator=( const ListBase &l )
    {
      if ( this == &l ) {
        return *this;
      }
      QList<T *>::operator=( l );
      return *this;
    }

    /**
      Sets this list to operate in "auto-delete" mode.
      This mode deletes the memory pointed at by all members of the list
      in the destructor.
      @param autoDelete if true, puts the list into "auto-delete" mode.
    */
    void setAutoDelete( bool autoDelete )
    {
      mAutoDelete = autoDelete;
    }

    /**
      Clears the list.
      Memory is also freed if the list is set to "auto-delete" mode.
    */
    void clearAll()
    {
      if ( mAutoDelete ) {
        qDeleteAll( *this );
      }
      QList<T*>::clear();
    }

    /**
      Removes all the members from the list with the specified address.
      Memory is also freed if the list is set to "auto-delete" mode.
      @param t is the pointer to remove from the list.
      @return true if successful; otherwise false (no such address @p t found).
    */
    bool removeRef( T *t )
    {
      if ( !contains( t ) ) {
        return false;
      } else {
        if ( mAutoDelete ) {
          delete t;
        }
        this->removeAll( t );
        return true;
      }
    }

    /**
      Removes the specified member from the list.
      Memory is also freed if the list is set to "auto-delete" mode.
      @param it the iterator to remove from the list.
    */
    void removeRef( typename QList<T*>::iterator it )
    {
      if ( mAutoDelete ) {
        delete *it;
      }
      QList<T*>::erase( it );
    }

    bool operator==( const ListBase &l2 )
    {
      int sz = QList<T*>::size();

      if ( sz != l2.size() ) {
        return false;
      } else {
        for ( int i=0; i<sz; ++i ) {
          if ( !( *QList<T*>::value( i ) == *l2.value( i ) ) ) {
            return false;
          }
        }
      }
      return true;
    }

  private:
    //@cond PRIVATE
    bool mAutoDelete;
    //@endcond
};

}

#endif
