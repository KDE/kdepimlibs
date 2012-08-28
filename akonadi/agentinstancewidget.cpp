/*
    Copyright (c) 2006-2008 Tobias Koenig <tokoe@kde.org>

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

#include "agentinstancewidget.h"

#include "agentfilterproxymodel.h"
#include "agentinstance.h"
#include "agentinstancemodel.h"

#include <KIcon>
#include <KGlobal>

#include <QtCore/QUrl>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListView>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>

namespace Akonadi {
namespace Internal {

static void iconsEarlyCleanup();

struct Icons
{
  Icons()
   : readyPixmap( KIcon( QLatin1String( "user-online" ) ).pixmap( QSize( 16, 16 ) ) )
   , syncPixmap( KIcon( QLatin1String( "network-connect" ) ).pixmap( QSize( 16, 16 ) ) )
   , errorPixmap( KIcon( QLatin1String( "dialog-error" ) ).pixmap( QSize( 16, 16 ) ) )
   , offlinePixmap( KIcon( QLatin1String( "network-disconnect" ) ).pixmap( QSize( 16, 16 ) ) )
  {
    qAddPostRoutine( iconsEarlyCleanup );
  }
  QPixmap readyPixmap, syncPixmap, errorPixmap, offlinePixmap;
};

K_GLOBAL_STATIC( Icons, s_icons )

// called as a Qt post routine, to prevent pixmap leaking
void iconsEarlyCleanup() {
  Icons * const ic = s_icons;
  ic->readyPixmap = ic->syncPixmap = ic->errorPixmap = ic->offlinePixmap = QPixmap();
}

/**
 * @internal
 */
class AgentInstanceWidgetDelegate : public QAbstractItemDelegate
{
  public:
    AgentInstanceWidgetDelegate( QObject *parent = 0 );

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

  private:
    void drawFocus( QPainter*, const QStyleOptionViewItem&, const QRect& ) const;

    QTextDocument* document( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

}

using Akonadi::Internal::AgentInstanceWidgetDelegate;

/**
 * @internal
 */
class AgentInstanceWidget::Private
{
  public:
    Private( AgentInstanceWidget *parent )
      : mParent( parent )
    {
    }

    void currentAgentInstanceChanged( const QModelIndex&, const QModelIndex& );
    void currentAgentInstanceDoubleClicked( const QModelIndex& );
    void currentAgentInstanceClicked( const QModelIndex &currentIndex );

    AgentInstanceWidget *mParent;
    QListView *mView;
    AgentInstanceModel *mModel;
    AgentFilterProxyModel *proxy;
};

void AgentInstanceWidget::Private::currentAgentInstanceChanged( const QModelIndex &currentIndex, const QModelIndex &previousIndex )
{
  AgentInstance currentInstance;
  if ( currentIndex.isValid() ) {
    currentInstance = currentIndex.data( AgentInstanceModel::InstanceRole ).value<AgentInstance>();
  }

  AgentInstance previousInstance;
  if ( previousIndex.isValid() ) {
    previousInstance = previousIndex.data( AgentInstanceModel::InstanceRole ).value<AgentInstance>();
  }

  emit mParent->currentChanged( currentInstance, previousInstance );
}

void AgentInstanceWidget::Private::currentAgentInstanceDoubleClicked( const QModelIndex &currentIndex )
{
  AgentInstance currentInstance;
  if ( currentIndex.isValid() ) {
    currentInstance = currentIndex.data( AgentInstanceModel::InstanceRole ).value<AgentInstance>();
  }

  emit mParent->doubleClicked( currentInstance );
}

void AgentInstanceWidget::Private::currentAgentInstanceClicked( const QModelIndex &currentIndex )
{
    AgentInstance currentInstance;
    if ( currentIndex.isValid() ) {
      currentInstance = currentIndex.data( AgentInstanceModel::InstanceRole ).value<AgentInstance>();
    }

    emit mParent->clicked( currentInstance );
}

AgentInstanceWidget::AgentInstanceWidget( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 0 );

  d->mView = new QListView( this );
  d->mView->setContextMenuPolicy( Qt::NoContextMenu );
  d->mView->setItemDelegate( new Internal::AgentInstanceWidgetDelegate( d->mView ) );
  d->mView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  d->mView->setAlternatingRowColors( true );
  d->mView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  layout->addWidget( d->mView );

  d->mModel = new AgentInstanceModel( this );

  d->proxy = new AgentFilterProxyModel( this );
  d->proxy->setSourceModel( d->mModel );
  d->mView->setModel( d->proxy );

  d->mView->selectionModel()->setCurrentIndex( d->mView->model()->index( 0, 0 ), QItemSelectionModel::Select );
  d->mView->scrollTo( d->mView->model()->index( 0, 0 ) );

  connect( d->mView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
           this, SLOT(currentAgentInstanceChanged(QModelIndex,QModelIndex)) );
  connect( d->mView, SIGNAL(doubleClicked(QModelIndex)),
           this, SLOT(currentAgentInstanceDoubleClicked(QModelIndex)) );
  connect( d->mView, SIGNAL(clicked(QModelIndex)),
           this, SLOT(currentAgentInstanceClicked(QModelIndex)) );
}

AgentInstanceWidget::~AgentInstanceWidget()
{
  delete d;
}

AgentInstance AgentInstanceWidget::currentAgentInstance() const
{
  QItemSelectionModel *selectionModel = d->mView->selectionModel();
  if ( !selectionModel ) {
    return AgentInstance();
  }

  QModelIndex index = selectionModel->currentIndex();
  if ( !index.isValid() ) {
    return AgentInstance();
  }

  return index.data( AgentInstanceModel::InstanceRole ).value<AgentInstance>();
}

QList<AgentInstance> AgentInstanceWidget::selectedAgentInstances() const
{
  QList<AgentInstance> list;
  QItemSelectionModel *selectionModel = d->mView->selectionModel();
  if ( !selectionModel ) {
    return list;
  }

  const QModelIndexList indexes = selectionModel->selection().indexes();

  foreach ( const QModelIndex &index, indexes ) {
    list.append( index.data( AgentInstanceModel::InstanceRole ).value<AgentInstance>() );
  }

  return list;
}

QAbstractItemView* AgentInstanceWidget::view() const
{
  return d->mView;
}


AgentFilterProxyModel* AgentInstanceWidget::agentFilterProxyModel() const
{
  return d->proxy;
}

AgentInstanceWidgetDelegate::AgentInstanceWidgetDelegate( QObject *parent )
 : QAbstractItemDelegate( parent )
{
}

QTextDocument* AgentInstanceWidgetDelegate::document( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !index.isValid() ) {
    return 0;
  }

  const QString name = index.model()->data( index, Qt::DisplayRole ).toString();
  int status = index.model()->data( index, AgentInstanceModel::StatusRole ).toInt();
  uint progress = index.model()->data( index, AgentInstanceModel::ProgressRole ).toUInt();
  const QString statusMessage = index.model()->data( index, AgentInstanceModel::StatusMessageRole ).toString();
  const QStringList capabilities = index.model()->data( index, AgentInstanceModel::CapabilitiesRole ).toStringList();

  QTextDocument *document = new QTextDocument( 0 );

  const QVariant data = index.model()->data( index, Qt::DecorationRole );
  if ( data.isValid() && data.type() == QVariant::Icon ) {
    document->addResource( QTextDocument::ImageResource, QUrl( QLatin1String( "agent_icon" ) ),
                           qvariant_cast<QIcon>( data ).pixmap( QSize( 64, 64 ) ) );
  }

  if ( !index.data( AgentInstanceModel::OnlineRole ).toBool() ) {
    document->addResource( QTextDocument::ImageResource, QUrl( QLatin1String( "status_icon" ) ), s_icons->offlinePixmap );
  } else if ( status == AgentInstance::Idle ) {
    document->addResource( QTextDocument::ImageResource, QUrl( QLatin1String( "status_icon" ) ), s_icons->readyPixmap );
  } else if ( status == AgentInstance::Running ) {
    document->addResource( QTextDocument::ImageResource, QUrl( QLatin1String( "status_icon" ) ), s_icons->syncPixmap );
  } else {
    document->addResource( QTextDocument::ImageResource, QUrl( QLatin1String( "status_icon" ) ), s_icons->errorPixmap );
  }

  QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
  if ( cg == QPalette::Normal && !( option.state & QStyle::State_Active ) ) {
    cg = QPalette::Inactive;
  }

  QColor textColor;
  if ( option.state & QStyle::State_Selected ) {
    textColor = option.palette.color( cg, QPalette::HighlightedText );
  } else {
    textColor = option.palette.color( cg, QPalette::Text );
  }

  QString content = QString::fromLatin1(
     "<html style=\"color:%1\">"
     "<body>"
     "<table>"
     "<tr>"
     "<td rowspan=\"2\"><img src=\"agent_icon\">&nbsp;&nbsp;</td>"
     "<td><b>%2</b></td>"
     "</tr>" ).arg( textColor.name().toUpper() ).arg( name )
     + QString::fromLatin1(
     "<tr>"
     "<td><img src=\"status_icon\"/> %1 %2</td>"
     "</tr>" ).arg( statusMessage ).arg( status == 1 ? QString( QLatin1String( "(%1%)" ) ).arg( progress ) : QLatin1String( "" ) )
     + QLatin1String( "</table></body></html>" );

  document->setHtml( content );

  return document;
}

void AgentInstanceWidgetDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !index.isValid() ) {
    return;
  }

  QTextDocument *doc = document( option, index );
  if ( !doc ) {
    return;
  }

  painter->setRenderHint( QPainter::Antialiasing );

  QPen pen = painter->pen();

  QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
  if ( cg == QPalette::Normal && !( option.state & QStyle::State_Active ) ) {
    cg = QPalette::Inactive;
  }

  QStyleOptionViewItemV4 opt( option );
  opt.showDecorationSelected = true;
  QApplication::style()->drawPrimitive( QStyle::PE_PanelItemViewItem, &opt, painter );

  painter->save();
  painter->translate( option.rect.topLeft() );
  doc->drawContents( painter );
  delete doc;
  painter->restore();

  painter->setPen( pen );

  drawFocus( painter, option, option.rect );
}

QSize AgentInstanceWidgetDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !index.isValid() ) {
    return QSize( 0, 0 );
  }

  QTextDocument *doc = document( option, index );
  if ( !doc ) {
    return QSize( 0, 0 );
  }

  const QSize size = doc->documentLayout()->documentSize().toSize();
  delete doc;

  return size;
}

void AgentInstanceWidgetDelegate::drawFocus( QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect ) const
{
  if ( option.state & QStyle::State_HasFocus ) {
    QStyleOptionFocusRect o;
    o.QStyleOption::operator=( option );
    o.rect = rect;
    o.state |= QStyle::State_KeyboardFocusChange;
    QPalette::ColorGroup cg = ( option.state & QStyle::State_Enabled ) ? QPalette::Normal : QPalette::Disabled;
    o.backgroundColor = option.palette.color( cg, ( option.state & QStyle::State_Selected )
                                                  ? QPalette::Highlight : QPalette::Background );
    QApplication::style()->drawPrimitive( QStyle::PE_FrameFocusRect, &o, painter );
  }
}

}

#include "agentinstancewidget.moc"
