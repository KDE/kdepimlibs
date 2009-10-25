/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
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
  defines the Journal class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/
#ifndef KCAL_JOURNAL_H
#define KCAL_JOURNAL_H

#include "incidence.h"
#include <kpimutils/supertrait.h>
#include <QtCore/QByteArray>

namespace KCal {

/**
  @brief
  Provides a Journal in the sense of RFC2445.
*/
class KCAL_EXPORT Journal : public Incidence
{
  public:
    /**
      List of journals.
    */
    typedef ListBase<Journal> List;

    /**
      A shared pointer to a Journal object.
    */
    typedef boost::shared_ptr<Journal> Ptr;

    /**
      A shared pointer to a non-mutable Journal object.
    */
    typedef boost::shared_ptr<const Journal> ConstPtr;

    /**
      Constructs an empty journal.
    */
    Journal();

    /**
      Destroys a journal.
    */
    ~Journal();

    /**
      @copydoc
      IncidenceBase::type()
    */
    QByteArray type() const;

    /**
      @copydoc
      IncidenceBase::typeStr()
    */
    //KDE5: QString typeStr() const;

    /**
      Returns an exact copy of this journal. The returned object is owned
      by the caller.
    */
    Journal *clone();

    /**
      Assignment operator.
    */
    Journal &operator=( const Journal &other );

    /**
      Compare this with @p journal for equality.

      @param journal is the journal to compare.
    */
    bool operator==( const Journal &journal ) const;

  private:
    /**
      @copydoc
      IncidenceBase::accept()
    */
    bool accept( Visitor &v ) { return v.visit( this ); }

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

//@cond PRIVATE
// super class trait specialization
namespace KPIMUtils {
  template <> struct SuperClass<KCal::Journal> : public SuperClassTrait<KCal::Incidence>{};
}
//@endcond

#endif
