/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_CACHEPOLICYPAGE_P_H
#define AKONADI_CACHEPOLICYPAGE_P_H

#include <akonadi/collectionpropertiespage.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(4,5,74)
#include "ui_cachepolicypage.h"
#else
#include "ui_cachepolicypage-4.5.h"
#endif

namespace Akonadi {

//@cond PRIVATE

/**
  @internal

  Cache policy configuration page.
*/
class CachePolicyPage : public CollectionPropertiesPage
{
  Q_OBJECT
  public:
    explicit CachePolicyPage( QWidget * parent );

    bool canHandle( const Collection &collection ) const;
    void load( const Collection &collection );
    void save( Collection &collection );

  private:
    Ui::CachePolicyPage ui;

  private slots:
    void slotIntervalValueChanged( int );
    void slotCacheValueChanged( int );
};

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CachePolicyPageFactory, CachePolicyPage)

//@endcond

}

#endif
