/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
 *  Copyright (C) 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include "katecompletiontree.h"

#include <QtGui/QHeaderView>
#include <QtGui/QScrollBar>
#include <QVector>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>

#include "kateview.h"
#include "katerenderer.h"
#include "kateconfig.h"

#include "katecompletionwidget.h"
#include "katecompletiondelegate.h"
#include "katecompletionmodel.h"

KateCompletionTree::KateCompletionTree(KateCompletionWidget* parent)
  : ExpandingTree(parent)
{
  m_scrollingEnabled = true;
  header()->hide();
  setRootIsDecorated(false);
  setIndentation(0);
  setFrameStyle(QFrame::NoFrame);
  setAllColumnsShowFocus(true);
  setAlternatingRowColors(true);
  //We need ScrollPerItem, because ScrollPerPixel is too slow with a very large competion-list(see KDevelop).
  setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

  m_resizeTimer = new QTimer(this);
  m_resizeTimer->setSingleShot(true);

  connect(m_resizeTimer, SIGNAL(timeout()), this, SLOT(resizeColumnsSlot()));
  
  // Provide custom highlighting to completion entries
  setItemDelegate(new KateCompletionDelegate(widget()->model(), widget()));
  
  ///@todo uncomment this once we're sure it isn't called too often, or maybe use a timer.
  //connect(widget()->model(), SIGNAL(contentGeometryChanged()), this, SLOT(resizeColumnsSlot()));

  // Prevent user from expanding / collapsing with the mouse
  setItemsExpandable(false);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void KateCompletionTree::currentChanged ( const QModelIndex & current, const QModelIndex & previous ) {
  widget()->model()->rowSelected(current);
  ExpandingTree::currentChanged(current, previous);
}

void KateCompletionTree::setScrollingEnabled(bool enabled) {
  m_scrollingEnabled = enabled;
}

void KateCompletionTree::scrollContentsBy( int dx, int dy )
{
  if(m_scrollingEnabled)
    QTreeView::scrollContentsBy(dx, dy);

  if (isVisible())
    m_resizeTimer->start(300);
}

int KateCompletionTree::columnTextViewportPosition ( int column ) const {
  int ret = columnViewportPosition(column);
  QModelIndex i = model()->index(0, column, QModelIndex());
  QModelIndex base = model()->index(0, 0, QModelIndex());
  
  //If it's just a group header, use the first child
  if(base.isValid() && model()->rowCount(base))
    i = base.child(0, column);
    
  if(i.isValid()) {
    QIcon icon = i.data(Qt::DecorationRole).value<QIcon>();
    if(!icon.isNull())
      ret += icon.actualSize(sizeHintForIndex(i)).width();
  }
  return ret;
}

KateCompletionWidget * KateCompletionTree::widget( ) const
{
  return static_cast<KateCompletionWidget*>(const_cast<QObject*>(parent()));
}

void KateCompletionTree::resizeColumnsSlot()
{
  if(model())
    resizeColumns();
}

void KateCompletionTree::resizeColumns(bool firstShow, bool forceResize)
{
  static bool preventRecursion = false;
  if (preventRecursion)
    return;

  if(firstShow)
    forceResize = true;

  preventRecursion = true;

  widget()->setUpdatesEnabled(false);

  int modelIndexOfName = kateModel()->translateColumn(KTextEditor::CodeCompletionModel::Name);
  int oldIndentWidth = columnViewportPosition(modelIndexOfName);

  ///Step 1: Compute the needed column-sizes for the visible content

  int numColumns = model()->columnCount();
  
  QVector<int> columnSize(numColumns, 5);

  int currentYPos = 0;

  QModelIndex current = indexAt(QPoint(1,1));
  if( current.child(0,0).isValid() ) { //If the index has children, it is a group-label. Then we should start with it's first child.
    currentYPos += sizeHintForIndex(current).height();
    current = current.child(0,0);
  }

  int num = 0;
  bool changed = false;
  
  while( current.isValid() && currentYPos < height() )
  {
//     kDebug() << current.row() << "out of" << model()->rowCount(current.parent()) << "in" << current.parent().data(Qt::DisplayRole);
    currentYPos += sizeHintForIndex(current).height();
//     itemDelegate()->sizeHint(QStyleOptionViewItem(), current).isValid() && itemDelegate()->sizeHint(QStyleOptionViewItem(), current).intersects(visibleViewportRect)
    changed = true;
    num++;
    for( int a = 0; a < numColumns; a++ )
    {
      QSize s = sizeHintForIndex (current.sibling(current.row(), a));
//       kDebug() << "size-hint for" << current.row() << a << ":" << s << current.sibling(current.row(), a).data(Qt::DisplayRole);
      if( s.width() > columnSize[a] && s.width() < 2000 )
        columnSize[a] = s.width();
      else if( s.width() > 2000 )
        kDebug( 13035 ) << "got invalid size-hint of width " << s.width();
    }

    QModelIndex oldCurrent = current;
    current = current.sibling(current.row()+1, 0);
    
    //Are we at the end of a group? If yes, move on into the next group
    if( !current.isValid() && oldCurrent.parent().isValid() ) {
      current = oldCurrent.parent().sibling( oldCurrent.parent().row()+1, 0 );
      if( current.isValid() && current.child(0,0).isValid() ) {
	currentYPos += sizeHintForIndex(current).height();
        current = current.child(0,0);
      }
    }
  }

  int totalColumnsWidth = 0, originalViewportWidth = viewport()->width();
  
  int maxWidth = (QApplication::desktop()->screenGeometry(widget()->view()).width()*3) / 4;

  ///Step 2: Update column-sizes
  //This contains several hacks to reduce the amount of resizing that happens. Generally,
  //resizes only happen if a) More than a specific amount of space is saved by the resize, or
  //b) the resizing is required so the list can show all of its contents.
  int minimumResize = 0;
  int maximumResize = 0;
  
  if( changed ) {

    for( int n = 0; n < numColumns; n++ ) {
      totalColumnsWidth += columnSize[n];
      
      int diff = columnSize[n] - columnWidth(n);
      if( diff < minimumResize )
        minimumResize = diff;
      if( diff > maximumResize )
        maximumResize = diff;
    }
    
    int noReduceTotalWidth = 0; //The total width of the widget of no columns are reduced
    for( int n = 0; n < numColumns; n++ ) {
      if(columnSize[n] < columnWidth(n))
        noReduceTotalWidth += columnWidth(n);
      else
        noReduceTotalWidth += columnSize[n];
    }

    //Check whether we can afford to reduce none of the columns
    //Only reduce size if we widget would else be too wide.
  bool noReduce = noReduceTotalWidth < maxWidth && !forceResize;

    if(noReduce) {
      totalColumnsWidth = 0;
      for( int n = 0; n < numColumns; n++ ) {
      	if(columnSize[n] < columnWidth(n))
          columnSize[n] = columnWidth(n);

        totalColumnsWidth += columnSize[n];
      }
    }

    if( minimumResize > -40 && maximumResize == 0 && !forceResize ) {
      //No column needs to be exanded, and no column needs to be reduced by more than 40 pixels.
      //To prevent flashing, do not resize at all.
      totalColumnsWidth = 0;
      for( int n = 0; n < numColumns; n++ ) {
        columnSize[n] = columnWidth(n);
        totalColumnsWidth += columnSize[n];
      }
    } else {
//       viewport()->resize( 5000, viewport()->height() );
      for( int n = 0; n < numColumns; n++ ) {
        setColumnWidth(n, columnSize[n]);
      }
//       kDebug() << "resizing viewport to" << totalColumnsWidth;
      viewport()->resize( totalColumnsWidth, viewport()->height() );
    }
  }

  ///Step 3: Update widget-size and -position
  
  int scrollBarWidth = verticalScrollBar()->width();
  
  int newIndentWidth = columnViewportPosition(modelIndexOfName);

  int newWidth = qMin(maxWidth, qMax(75, totalColumnsWidth));
  
  if(newWidth == maxWidth)
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  else
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
  if(maximumResize > 0 || forceResize || oldIndentWidth != newIndentWidth) {
  
    //   kDebug() << geometry() << "newWidth" << newWidth << "current width" << width() << "target width" << newWidth + scrollBarWidth;
    
    if((newWidth + scrollBarWidth) != width() && originalViewportWidth != totalColumnsWidth)
    {
        widget()->resize(newWidth + scrollBarWidth + 2, widget()->height());
        resize(newWidth + scrollBarWidth, widget()->height()- (2*widget()->frameWidth()));
    }
    
    //   kDebug() << "created geometry:" << widget()->geometry() << geometry() << "newWidth" << newWidth << "viewport" << viewport()->width();
    
    if( viewport()->width() > totalColumnsWidth ) //Set the size of the last column to fill the whole rest of the widget
    setColumnWidth(numColumns-1, viewport()->width() - columnViewportPosition(numColumns-1));
    
    /*  for(int a = 0; a < numColumns; ++a)
        kDebug() << "column" << a << columnWidth(a) << "target:" << columnSize[a];*/
    
    if (oldIndentWidth != newIndentWidth)
        if(widget()->updatePosition() && !forceResize) {
        preventRecursion = false;
        resizeColumns(true, true);
        }
  }
    
  widget()->setUpdatesEnabled(true);
  
  preventRecursion = false;
}

QStyleOptionViewItem KateCompletionTree::viewOptions( ) const
{
  QStyleOptionViewItem opt = QTreeView::viewOptions();

  opt.font = widget()->view()->renderer()->config()->font();

  return opt;
}

KateCompletionModel * KateCompletionTree::kateModel( ) const
{
  return static_cast<KateCompletionModel*>(model());
}

bool KateCompletionTree::nextCompletion()
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

  } while (!kateModel()->indexIsItem(current));

  return true;
}

bool KateCompletionTree::previousCompletion()
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

  } while (!kateModel()->indexIsItem(current));

  return true;
}

bool KateCompletionTree::pageDown( )
{
  QModelIndex old = currentIndex();
  
  QModelIndex current = moveCursor(MovePageDown, Qt::NoModifier);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!kateModel()->indexIsItem(current))
      if (!nextCompletion())
        previousCompletion();
  }

  return current != old;
}

bool KateCompletionTree::pageUp( )
{
  QModelIndex old = currentIndex();
  QModelIndex current = moveCursor(MovePageUp, Qt::NoModifier);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!kateModel()->indexIsItem(current))
      if (!previousCompletion())
        nextCompletion();
  }
  return current != old;
}

void KateCompletionTree::top( )
{
  QModelIndex current = moveCursor(MoveHome, Qt::NoModifier);
  setCurrentIndex(current);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!kateModel()->indexIsItem(current))
      nextCompletion();
  }
}

void KateCompletionTree::scheduleUpdate()
{
    m_resizeTimer->start(300);
}

void KateCompletionTree::bottom( )
{
  QModelIndex current = moveCursor(MoveEnd, Qt::NoModifier);
  setCurrentIndex(current);

  if (current.isValid()) {
    setCurrentIndex(current);
    if (!kateModel()->indexIsItem(current))
      previousCompletion();
  }
}

#include "katecompletiontree.moc"
