/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef KATEARGUMENTHINTTREE_H
#define KATEARGUMENTHINTTREE_H

#include "expandingtree/expandingtree.h"

class KateCompletionWidget;
class KateArgumentHintModel;
class QRect;

class KateArgumentHintTree : public ExpandingTree {
  Q_OBJECT
  public:
    KateArgumentHintTree( KateCompletionWidget* parent );

    // Navigation
    bool nextCompletion();
    bool previousCompletion();
    bool pageDown();
    bool pageUp();
    void top();
    void bottom();

    //Returns the total size of all columns
    int resizeColumns();
    
    void  clearCompletion();
  public slots:
    void updateGeometry();
    void updateGeometry(QRect rect);
  protected:
    virtual void paintEvent ( QPaintEvent * event );
    virtual void rowsInserted ( const QModelIndex & parent, int start, int end );
    virtual void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    virtual void currentChanged ( const QModelIndex & current, const QModelIndex & previous );
  private:
    uint rowHeight(const QModelIndex& index) const;
    KateArgumentHintModel* model() const;
    virtual int sizeHintForColumn ( int column ) const;
    
    KateCompletionWidget* m_parent;
};

#endif
