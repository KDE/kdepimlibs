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

#include "collectionpropertiesdialog.h"

#include "cachepolicy.h"
#include "cachepolicypage.h"
#include "collection.h"
#include "collectiongeneralpropertiespage_p.h"
#include "collectionmodifyjob.h"

#include <kdebug.h>
#include <ktabwidget.h>

#include <QtGui/QBoxLayout>

using namespace Akonadi;

/**
 * @internal
 */
class CollectionPropertiesDialog::Private
{
  public:
    Private( CollectionPropertiesDialog *parent );

    static void registerBuiltinPages();

    void save()
    {
      for ( int i = 0; i < tabWidget->count(); ++i ) {
        CollectionPropertiesPage *page = static_cast<CollectionPropertiesPage*>( tabWidget->widget( i ) );
        page->save( collection );
      }

      CollectionModifyJob *job = new CollectionModifyJob( collection, q );
      connect( job, SIGNAL( result( KJob* ) ), q, SLOT( saveResult( KJob* ) ) );
    }

    void saveResult( KJob *job )
    {
      if ( job->error() ) {
        // TODO
        kWarning() << job->errorString();
      }
      q->deleteLater();
    }

    Collection collection;
    KTabWidget* tabWidget;
    CollectionPropertiesDialog *q;
};

typedef QList<CollectionPropertiesPageFactory*> CollectionPropertiesPageFactoryList;

K_GLOBAL_STATIC( CollectionPropertiesPageFactoryList, s_pages )

static bool s_defaultPage = true;

CollectionPropertiesDialog::Private::Private( CollectionPropertiesDialog *parent ) : q( parent )
{
  if ( s_pages->isEmpty() && s_defaultPage)
    registerBuiltinPages();
}

void CollectionPropertiesDialog::Private::registerBuiltinPages()
{
  s_pages->append( new CollectionGeneralPropertiesPageFactory() );
  s_pages->append( new CachePolicyPageFactory() );
}


CollectionPropertiesDialog::CollectionPropertiesDialog(const Collection & collection, QWidget * parent) :
    KDialog( parent ),
    d( new Private( this ) )
{
  d->collection = collection;

  QBoxLayout *layout = new QHBoxLayout( mainWidget() );
  layout->setMargin( 0 );
  d->tabWidget = new KTabWidget( mainWidget() );
  layout->addWidget( d->tabWidget );

  foreach ( CollectionPropertiesPageFactory *factory, *s_pages ) {
    CollectionPropertiesPage *page = factory->createWidget( d->tabWidget );
    if ( page->canHandle( d->collection ) ) {
      d->tabWidget->addTab( page, page->pageTitle() );
      page->load( d->collection );
    } else {
      delete page;
    }
  }

  connect( this, SIGNAL( okClicked() ), SLOT( save() ) );
  connect( this, SIGNAL( cancelClicked() ), SLOT( deleteLater() ) );
}

CollectionPropertiesDialog::~CollectionPropertiesDialog()
{
  delete d;
}

void CollectionPropertiesDialog::registerPage(CollectionPropertiesPageFactory * factory)
{
  if ( s_pages->isEmpty() && s_defaultPage)
    Private::registerBuiltinPages();
  s_pages->append( factory );
}

void CollectionPropertiesDialog::useDefaultPage( bool defaultPage )
{
  s_defaultPage = defaultPage;
}

#include "collectionpropertiesdialog.moc"
