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

#include "kateargumenthinttree.h"

#include <QHeaderView>
#include <QApplication>
#include <QDesktopWidget>
#include <QScrollBar>

#include "kateargumenthintmodel.h"
#include "katecompletionwidget.h"
#include "expandingtree/expandingwidgetmodel.h"
#include "katecompletiondelegate.h"
#include "kateview.h"
#include <QModelIndex>


KateArgumentHintTree::KateArgumentHintTree( KateCompletionWidget* parent ) : ExpandingTree(0), m_parent(parent) { //Do not use the completion-widget as widget-parent, because the argument-hint-tree will be rendered separately

  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth( 1 );
  
  connect( parent, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()) );
  setFrameStyle(QFrame::NoFrame);
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setFocusPolicy(Qt::NoFocus);
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
  setUniformRowHeights(false);
  setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  header()->hide();
  setRootIsDecorated(false);
  setIndentation(0);
  setAllColumnsShowFocus(true);
  setAlternatingRowColors(true);
  setItemDelegate(new KateCompletionDelegate(parent->argumentHintModel(), parent));
}

void KateArgumentHintTree::clearCompletion() {
  setCurrentIndex(QModelIndex());
}

KateArgumentHintModel* KateArgumentHintTree::model() const {
  return m_parent->argumentHintModel();
}

void KateArgumentHintTree::paintEvent ( QPaintEvent * event ) {
  QTreeView::paintEvent(event);
  updateGeometry(); ///@todo delay this. It is needed here, because visualRect(...) returns an invalid rect in updateGeometry before the content is painted
}

void KateArgumentHintTree::dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight ) {
  QTreeView::dataChanged(topLeft,bottomRight);
  //updateGeometry();
}

void KateArgumentHintTree::currentChanged ( const QModelIndex & current, const QModelIndex & previous ) {
/*  kDebug( 13035 ) << "currentChanged()";*/
  static_cast<ExpandingWidgetModel*>(model())->rowSelected(current);
  QTreeView::currentChanged(current, previous);
}

void KateArgumentHintTree::rowsInserted ( const QModelIndex & parent, int start, int end ) {
  QTreeView::rowsInserted(parent, start, end);
  updateGeometry();
}

int KateArgumentHintTree::sizeHintForColumn(int column) const {
  return QTreeView::sizeHintForColumn(column);
}

unsigned int KateArgumentHintTree::rowHeight(const QModelIndex& index) const {
  uint max = sizeHintForIndex(index).height();

  for(int a = 0; a < index.model()->columnCount(index.parent()); ++a) {
    QModelIndex i = index.sibling(index.row(), a);
    uint cSize = sizeHintForIndex(i).height();
    if(cSize > max)
      max = cSize;
  }
  return max;
}

void KateArgumentHintTree::updateGeometry(QRect geom) {
  //Avoid recursive calls of updateGeometry
  static bool updatingGeometry = false;
  if( updatingGeometry ) return;
  updatingGeometry = true;
  
  if( model()->rowCount(QModelIndex()) == 0 ) {
/*  kDebug( 13035 ) << "KateArgumentHintTree:: empty model";*/
    hide();
    setGeometry(geom);
    updatingGeometry = false;
    return;
  }

  int bottom = geom.bottom();
  int totalWidth = resizeColumns();
  int totalHeight = 0;
  for(int a = 0; a < model()->rowCount( QModelIndex() ); ++a) {
    QModelIndex index(model()->index(a, 0));
    totalHeight += rowHeight(index);
    for(int b = 0; b < model()->rowCount(index); ++b) {
      QModelIndex childIndex = index.child(b, 0);
      totalHeight += rowHeight(childIndex);
    }
  }
  
  totalHeight += frameWidth()*2;
  
  QRect topRect = visualRect(model()->index(0, 0));
  QRect contentRect = visualRect(model()->index(model()->rowCount(QModelIndex())-1, 0));
  
  geom.setHeight(totalHeight);

  geom.moveBottom(bottom);
//   if( totalWidth > geom.width() )
    geom.setWidth(totalWidth);

  bool enableScrollBars = false;
  
  //Resize and move so it fits the screen horizontally
  int maxWidth = (QApplication::desktop()->screenGeometry(m_parent->view()).width()*3)/4;
  if( geom.width() > maxWidth ) {
    geom.setWidth(maxWidth);
    geom.setHeight(geom.height() + horizontalScrollBar()->height() +2);
    geom.moveBottom(bottom);
    enableScrollBars = true;
  }
  
  if (geom.right() > QApplication::desktop()->screenGeometry(m_parent->view()).right())
    geom.moveRight( QApplication::desktop()->screenGeometry(m_parent->view()).right() );

  if( geom.left() < QApplication::desktop()->screenGeometry(m_parent->view()).left() )
    geom.moveLeft(QApplication::desktop()->screenGeometry(m_parent->view()).left());

  //Resize and move so it fits the screen vertically
  bool resized = false;
  if( geom.top() < QApplication::desktop()->screenGeometry(this).top() ) {
    int offset = QApplication::desktop()->screenGeometry(this).top() - geom.top();
    geom.setBottom( geom.bottom() - offset );
    geom.moveTo(geom.left(), QApplication::desktop()->screenGeometry(this).top());
    resized = true;
  }

  if(geom != geometry())
  {
    setUpdatesEnabled(false);
    setAnimated(false);
    
    setHorizontalScrollBarPolicy( enableScrollBars ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff );
    
  /*  kDebug( 13035 ) << "KateArgumentHintTree::updateGeometry: updating geometry to " << geom;*/
    setGeometry(geom);
    
    if( resized && currentIndex().isValid() )
      scrollTo(currentIndex());
    
    setUpdatesEnabled(true);
  }
  
  updatingGeometry = false;
}

int KateArgumentHintTree::resizeColumns() {
  int totalSize = 0;
  for( int a  = 0; a < header()->count(); a++ ) {
    int columnSize = sizeHintForColumn(a);
    setColumnWidth(a, columnSize);
    totalSize += columnSize;
  }
  return totalSize;
}

void KateArgumentHintTree::updateGeometry() {
  updateGeometry( geometry() );
}

bool KateArgumentHintTree::nextCompletion()
{
  QModelIndex current;
  QModelIndex firstCurrent = currentIndex();

  do {
    QModelIndex oldCurrent = currentIndex();

    current = moveCursor(MoveDown, Qt::NoModifier);

    if (current != oldCurrent && current.isValid()) {
      setCurrentIndex(current);

    } else {
      if (firstCurrent.isValid())
        setCurrentIndex(firstCurrent);
      return false;
    }

  } while (!model()->indexIsItem(current));

  return true;
}

bool KateArgumentHintTree::previousCompletion()
{
  QModelIndex current;
  QModelIndex firstCurrent = currentIndex();

  do {
    QModelIndex oldCurrent = currentIndex();

    current = moveCursor(MoveUp, Qt::NoModifier);

    if (current != oldCurrent && current.isValid()) {
      setCurrentIndex(current);

    } else {
      if (firstCurrent.isValid())
        setCurrentIndex(firstCurrent);
      return false;
    }

  } while (!model()->indexIsItem(current));

  return true;
}

bool KateArgumentHintTree::pageDown( )
{
  QModelIndex old = currentIndex();
  QModelIndex current = moveCursor(MovePageDown, Qt::NoModifier);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!model()->indexIsItem(current))
      if (!nextCompletion())
        previousCompletion();
  }

  return current != old;
}

bool KateArgumentHintTree::pageUp( )
{
  QModelIndex old = currentIndex();
  QModelIndex current = moveCursor(MovePageUp, Qt::NoModifier);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!model()->indexIsItem(current))
      if (!previousCompletion())
        nextCompletion();
  }
  return current != old;
}

void KateArgumentHintTree::top( )
{
  QModelIndex current = moveCursor(MoveHome, Qt::NoModifier);
  setCurrentIndex(current);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!model()->indexIsItem(current))
      nextCompletion();
  }
}

void KateArgumentHintTree::bottom( )
{
  QModelIndex current = moveCursor(MoveEnd, Qt::NoModifier);
  setCurrentIndex(current);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!model()->indexIsItem(current))
      previousCompletion();
  }
}

#include "kateargumenthinttree.moc"
