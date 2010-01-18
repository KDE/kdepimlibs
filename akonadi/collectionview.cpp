/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "collectionview.h"

#include "collection.h"
#include "collectionmodel.h"
#include "control.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kxmlguifactory.h>
#include <kxmlguiwindow.h>

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>

using namespace Akonadi;

/**
 * @internal
 */
class CollectionView::Private
{
  public:
    Private( CollectionView *parent )
      : mParent( parent ),
        xmlGuiClient( 0 )
    {
    }

    void init();
    void dragExpand();
    void itemClicked( const QModelIndex& );
    void itemCurrentChanged( const QModelIndex& );
    bool hasParent( const QModelIndex& idx, Collection::Id parentId );

    CollectionView *mParent;
    QModelIndex dragOverIndex;
    QTimer dragExpandTimer;

    KXMLGUIClient *xmlGuiClient;
};

void CollectionView::Private::init()
{
  mParent->header()->setClickable( true );
  mParent->header()->setStretchLastSection( false );

  mParent->setSortingEnabled( true );
  mParent->sortByColumn( 0, Qt::AscendingOrder );
  mParent->setEditTriggers( QAbstractItemView::EditKeyPressed );
  mParent->setAcceptDrops( true );
  mParent->setDropIndicatorShown( true );
  mParent->setDragDropMode( DragDrop );
  mParent->setDragEnabled( true );

  dragExpandTimer.setSingleShot( true );
  mParent->connect( &dragExpandTimer, SIGNAL(timeout()), SLOT(dragExpand()) );

  mParent->connect( mParent, SIGNAL( clicked( const QModelIndex& ) ),
                    mParent, SLOT( itemClicked( const QModelIndex& ) ) );

  Control::widgetNeedsAkonadi( mParent );
}

bool CollectionView::Private::hasParent( const QModelIndex& idx, Collection::Id parentId )
{
  QModelIndex idx2 = idx;
  while( idx2.isValid() ) {
    if ( mParent->model()->data( idx2, CollectionModel::CollectionIdRole).toLongLong() == parentId )
      return true;

    idx2 = idx2.parent();
  }
  return false;
}

void CollectionView::Private::dragExpand()
{
  mParent->setExpanded( dragOverIndex, true );
  dragOverIndex = QModelIndex();
}

void CollectionView::Private::itemClicked( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const Collection collection = index.model()->data( index, CollectionModel::CollectionRole ).value<Collection>();
  if ( !collection.isValid() )
    return;

  emit mParent->clicked( collection );
}

void CollectionView::Private::itemCurrentChanged( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const Collection collection = index.model()->data( index, CollectionModel::CollectionRole ).value<Collection>();
  if ( !collection.isValid() )
    return;

  emit mParent->currentChanged( collection );
}

CollectionView::CollectionView( QWidget * parent )
  : QTreeView( parent ),
    d( new Private( this ) )
{
  d->init();
}

CollectionView::CollectionView( KXMLGUIClient *xmlGuiClient, QWidget * parent )
  : QTreeView( parent ),
    d( new Private( this ) )
{
  d->xmlGuiClient = xmlGuiClient;
  d->init();
}

CollectionView::CollectionView( KXmlGuiWindow *xmlGuiWindow, QWidget * parent )
  : QTreeView( parent ),
    d( new Private( this ) )
{
  d->xmlGuiClient = static_cast<KXMLGUIClient*>( xmlGuiWindow );
  d->init();
}

CollectionView::~CollectionView()
{
  delete d;
}

void CollectionView::setModel( QAbstractItemModel * model )
{
  QTreeView::setModel( model );
  header()->setStretchLastSection( true );

  connect( selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( itemCurrentChanged( const QModelIndex& ) ) );
}

void CollectionView::dragMoveEvent( QDragMoveEvent * event )
{
  QModelIndex index = indexAt( event->pos() );
  if ( d->dragOverIndex != index ) {
    d->dragExpandTimer.stop();
    if ( index.isValid() && !isExpanded( index ) && itemsExpandable() ) {
      d->dragExpandTimer.start( QApplication::startDragTime() );
      d->dragOverIndex = index;
    }
  }

  // Check if the collection under the cursor accepts this data type
  const QStringList supportedContentTypes = model()->data( index, CollectionModel::CollectionRole ).value<Collection>().contentMimeTypes();
  const QMimeData *mimeData = event->mimeData();
  const KUrl::List urls = KUrl::List::fromMimeData( mimeData );
  foreach( const KUrl &url, urls ) {

    const Collection collection = Collection::fromUrl( url );
    if ( collection.isValid() ) {
      if ( !supportedContentTypes.contains( QString::fromLatin1( "inode/directory" ) ) )
        break;

      // Check if we don't try to drop on one of the children
      if ( d->hasParent( index, collection.id() ) )
        break;
    } else {
      const QString type = url.queryItems()[ QString::fromLatin1( "type" ) ];
      if ( !supportedContentTypes.contains( type ) )
        break;
    }

    QTreeView::dragMoveEvent( event );
    return;
  }

  event->setDropAction( Qt::IgnoreAction );
}

void CollectionView::dragLeaveEvent( QDragLeaveEvent * event )
{
  d->dragExpandTimer.stop();
  d->dragOverIndex = QModelIndex();
  QTreeView::dragLeaveEvent( event );
}

void CollectionView::dropEvent( QDropEvent * event )
{
  d->dragExpandTimer.stop();
  d->dragOverIndex = QModelIndex();

  // open a context menu offering different drop actions (move, copy and cancel)
  // TODO If possible, hide non available actions ...
  QMenu popup( this );
  QAction* moveDropAction = popup.addAction( KIcon( QString::fromLatin1( "edit-rename" ) ), i18n( "&Move here" ) );
  QAction* copyDropAction = popup.addAction( KIcon( QString::fromLatin1( "edit-copy" ) ), i18n( "&Copy here" ) );
  popup.addSeparator();
  popup.addAction( KIcon( QString::fromLatin1( "process-stop" ) ), i18n( "Cancel" ) );

  QAction *activatedAction = popup.exec( QCursor::pos() );
  if ( activatedAction == moveDropAction ) {
    event->setDropAction( Qt::MoveAction );
  } else if ( activatedAction == copyDropAction ) {
    event->setDropAction( Qt::CopyAction );
  } else {
    return;
  }

  QTreeView::dropEvent( event );
}

void CollectionView::contextMenuEvent( QContextMenuEvent * event )
{
  if ( !d->xmlGuiClient )
    return;
  QMenu *popup = static_cast<QMenu*>( d->xmlGuiClient->factory()->container(
                                      QLatin1String( "akonadi_collectionview_contextmenu" ), d->xmlGuiClient ) );
  if ( popup )
    popup->exec( event->globalPos() );
}

void CollectionView::setXmlGuiClient( KXMLGUIClient * xmlGuiClient )
{
  d->xmlGuiClient = xmlGuiClient;
}

void CollectionView::setXmlGuiWindow( KXmlGuiWindow * xmlGuiWindow )
{
  d->xmlGuiClient = static_cast<KXMLGUIClient*>( xmlGuiWindow );
}

#include "collectionview.moc"
