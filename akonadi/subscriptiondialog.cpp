/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "subscriptiondialog_p.h"

#include "ui_subscriptiondialog.h"
#include "subscriptionmodel_p.h"
#include "subscriptionjob_p.h"
#include "subscriptionchangeproxymodel_p.h"
#include "flatcollectionproxymodel_p.h"
#include "control.h"

#include <krecursivefilterproxymodel.h>
#include <recursivecollectionfilterproxymodel.h>
#include <kdebug.h>

using namespace Akonadi;

/**
 * @internal
 */
class SubscriptionDialog::Private
{
  public:
    Private( SubscriptionDialog *parent ) : q( parent ) {}

    void setupChangeView( QTreeView *view, bool subscribe )
    {
      FlatCollectionProxyModel *flatProxy = new FlatCollectionProxyModel( q );
      flatProxy->setSourceModel( model );
      SubscriptionChangeProxyModel *subProxy = new SubscriptionChangeProxyModel( subscribe, q );
      subProxy->setSourceModel( flatProxy );
      view->setModel( subProxy );
    }

    void done()
    {
      SubscriptionJob *job = new SubscriptionJob( q );
      job->subscribe( model->subscribed() );
      job->unsubscribe( model->unsubscribed() );
      connect( job, SIGNAL( result( KJob* ) ), q, SLOT( subscriptionResult( KJob* ) ) );
    }

    void subscriptionResult( KJob *job )
    {
      if ( job->error() ) {
        // TODO
        kWarning() << job->errorString();
      }
      q->deleteLater();
    }

    void subscribeClicked()
    {
      foreach ( const QModelIndex &index, ui.collectionView->selectionModel()->selectedIndexes() )
        model->setData( index, Qt::Checked, Qt::CheckStateRole );
    }

    void unsubscribeClicked()
    {
      foreach ( const QModelIndex &index, ui.collectionView->selectionModel()->selectedIndexes() )
        model->setData( index, Qt::Unchecked, Qt::CheckStateRole );
    }

    void modelLoaded()
    {
      ui.collectionView->setEnabled( true );
      ui.subscribeButton->setEnabled( true );
      ui.unsubscribeButton->setEnabled( true );
      ui.subscribeView->setEnabled( true );
      ui.unsubscribeView->setEnabled( true );
      ui.collectionView->expandAll();
      q->enableButtonOk( true );
    }

    SubscriptionDialog* q;
    Ui::SubscriptionDialog ui;
    SubscriptionModel* model;
    KRecursiveFilterProxyModel *filterTreeViewModel;
    RecursiveCollectionFilterProxyModel *filterRecursiveCollectionFilter;

};

SubscriptionDialog::SubscriptionDialog(QWidget * parent) :
    KDialog( parent ),
    d( new Private( this ) )
{
  init();
}

SubscriptionDialog::SubscriptionDialog(const QString& mimetype, QWidget * parent) :
    KDialog( parent ),
    d( new Private( this ) )
{
  init( mimetype );
}

void SubscriptionDialog::init( const QString& mimetype )
{
  enableButtonOk( false );
  d->ui.setupUi( mainWidget() );
  KIcon icon;
  if ( QApplication::isLeftToRight() )
    icon = KIcon( QLatin1String( "go-next" ) );
  else
    icon = KIcon( QLatin1String( "go-previous" ) );
  d->ui.subscribeButton->setIcon( icon );
  d->ui.unsubscribeButton->setIcon( icon );

  d->model = new SubscriptionModel( this );

  d->filterTreeViewModel = new KRecursiveFilterProxyModel( this );
  d->filterTreeViewModel->setDynamicSortFilter( true );
  d->filterTreeViewModel->setSourceModel( d->model );
  d->filterTreeViewModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

  d->filterRecursiveCollectionFilter = new Akonadi::RecursiveCollectionFilterProxyModel( this );
  if ( !mimetype.isEmpty() )
    d->filterRecursiveCollectionFilter->addContentMimeTypeInclusionFilter( mimetype );

  d->filterRecursiveCollectionFilter->setSourceModel( d->filterTreeViewModel );


  d->ui.collectionView->setModel( d->filterRecursiveCollectionFilter );



  d->setupChangeView( d->ui.subscribeView, true );
  d->setupChangeView( d->ui.unsubscribeView, false );

  connect( d->model, SIGNAL( loaded() ), SLOT( modelLoaded() ) );
  connect( d->ui.subscribeButton, SIGNAL( clicked() ), SLOT( subscribeClicked() ) );
  connect( d->ui.unsubscribeButton, SIGNAL( clicked() ), SLOT( unsubscribeClicked() ) );
  connect( this, SIGNAL( okClicked() ), SLOT( done() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( deleteLater() ) );
  connect( d->ui.klineedit, SIGNAL( textChanged( const QString& ) ),
           d->filterTreeViewModel, SLOT( setFilterFixedString( const QString& ) ) );
  Control::widgetNeedsAkonadi( mainWidget() );
}

SubscriptionDialog::~ SubscriptionDialog()
{
  delete d;
}

#include "subscriptiondialog_p.moc"
