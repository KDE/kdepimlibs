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

#ifndef ExpandingTree_H
#define ExpandingTree_H

#include <QTreeView>
#include <QTextDocument>

//A tree that allows drawing additional information
class ExpandingTree : public QTreeView {
 public:
   ExpandingTree(QWidget* parent);
  protected:
    virtual void drawRow ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    virtual int sizeHintForColumn ( int column ) const;
  private:
    mutable QTextDocument m_drawText;
};

#endif
