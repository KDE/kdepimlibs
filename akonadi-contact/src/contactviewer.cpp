/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "contactviewer.h"

#include "contactmetadata_p.h"
#include "contactmetadataattribute_p.h"
#include "customfieldmanager_p.h"
#include "standardcontactformatter.h"
#include "textbrowser_p.h"

#include "editor/im/improtocols.h"

#include <collection.h>
#include <collectionfetchjob.h>
#include <entitydisplayattribute.h>
#include <item.h>
#include <itemfetchscope.h>
#include <kcontacts/addressee.h>
#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <kstringhandler.h>

#include <QVBoxLayout>
#include <QIcon>
#ifdef HAVE_PRISON
#include <prison/QRCodeBarcode>
#include <prison/DataMatrixBarcode>
#include <kcontacts/vcardconverter.h>
#endif // HAVE_PRISON

using namespace Akonadi;

class ContactViewer::Private
{
  public:
    Private( ContactViewer *parent )
      : mParent( parent ), mParentCollectionFetchJob( 0 )
    {
      mStandardContactFormatter = new StandardContactFormatter;
      mContactFormatter = mStandardContactFormatter;
#ifdef HAVE_PRISON
      mQRCode = new prison::QRCodeBarcode();
      mDataMatrix = new prison::DataMatrixBarcode();
#endif // HAVE_PRISON
    }

    ~Private()
    {
      delete mStandardContactFormatter;
#ifdef HAVE_PRISON
      delete mQRCode;
      delete mDataMatrix;
#endif // HAVE_PRISON
    }

    void updateView( const QVariantList &localCustomFieldDescriptions = QVariantList(), const QString &addressBookName = QString() )
    {
      static QPixmap defaultPixmap = QIcon::fromTheme( QLatin1String( "user-identity" ) ).pixmap( QSize( 100, 100 ) );

      mParent->setWindowTitle( i18n( "Contact %1", mCurrentContact.assembledName() ) );

      if ( mCurrentContact.photo().isIntern() ) {
        mBrowser->document()->addResource( QTextDocument::ImageResource,
                                           QUrl( QLatin1String( "contact_photo" ) ),
                                           mCurrentContact.photo().data() );
      } else {
        mBrowser->document()->addResource( QTextDocument::ImageResource,
                                           QUrl( QLatin1String( "contact_photo" ) ),
                                           defaultPixmap );
      }

      if ( mCurrentContact.logo().isIntern() ) {
        mBrowser->document()->addResource( QTextDocument::ImageResource,
                                           QUrl( QLatin1String( "contact_logo" ) ),
                                           mCurrentContact.logo().data() );
      }

      mBrowser->document()->addResource( QTextDocument::ImageResource,
                                         QUrl( QLatin1String( "map_icon" ) ),
                                         QIcon::fromTheme( QLatin1String( "document-open-remote" ) ).pixmap( QSize( 16, 16 ) ) );

      mBrowser->document()->addResource( QTextDocument::ImageResource,
                                         QUrl( QLatin1String( "sms_icon" ) ),
                                         QIcon::fromTheme( IMProtocols::self()->icon( QStringLiteral( "messaging/sms" ) ) ).pixmap( QSize( 16, 16 ) ) );

#ifdef HAVE_PRISON
      KConfig config( QLatin1String( "akonadi_contactrc" ) );
      KConfigGroup group( &config, QLatin1String( "View" ) );
      if ( group.readEntry( "QRCodes", true ) ) {
        KContacts::VCardConverter converter;
        KContacts::Addressee addr( mCurrentContact );
        addr.setPhoto( KContacts::Picture() );
        addr.setLogo( KContacts::Picture() );
        const QString data = QString::fromUtf8( converter.createVCard( addr ) );
        mQRCode->setData( data );
        mDataMatrix->setData( data );
        mBrowser->document()->addResource( QTextDocument::ImageResource,
                                           QUrl( QLatin1String( "qrcode" ) ),
                                           mQRCode->toImage( QSizeF( 50, 50 ) ) );
        mBrowser->document()->addResource( QTextDocument::ImageResource,
                                           QUrl( QLatin1String( "datamatrix" ) ),
                                           mDataMatrix->toImage( QSizeF( 50, 50 ) ) );
      }
#endif // HAVE_PRISON

      // merge local and global custom field descriptions
      QList<QVariantMap> customFieldDescriptions;
      foreach ( const QVariant &entry, localCustomFieldDescriptions ) {
        customFieldDescriptions << entry.toMap();
      }

      const CustomField::List globalCustomFields = CustomFieldManager::globalCustomFieldDescriptions();
      foreach ( const CustomField &field, globalCustomFields ) {
        QVariantMap description;
        description.insert( QLatin1String( "key" ), field.key() );
        description.insert( QLatin1String( "title" ), field.title() );

        customFieldDescriptions << description;
      }

      KContacts::Addressee contact( mCurrentContact );
      if ( !addressBookName.isEmpty() ) {
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "AddressBook" ), addressBookName );
      }

      mContactFormatter->setContact( contact );
      mContactFormatter->setCustomFieldDescriptions( customFieldDescriptions );

      mBrowser->setHtml( mContactFormatter->toHtml() );
    }

    void slotMailClicked( const QString&, const QString &email )
    {
      QString name, address;

      // remove the 'mailto:' and split into name and email address
      KContacts::Addressee::parseEmailAddress( email.mid( 7 ), name, address );

      emit mParent->emailClicked( name, address );
    }

    void slotUrlClicked( const QUrl &url )
    {
      const QString urlScheme( url.scheme() );
      if ( urlScheme == QLatin1String( "http" ) ||
           urlScheme == QLatin1String( "https" ) ) {
        emit mParent->urlClicked( url );
      } else if ( urlScheme == QLatin1String( "phone" ) ) {
        const int pos = url.queryItemValue( QLatin1String( "index" ) ).toInt();

        const KContacts::PhoneNumber::List numbers = mCurrentContact.phoneNumbers();
        if ( pos < numbers.count() ) {
          emit mParent->phoneNumberClicked( numbers.at( pos ) );
        }
      } else if ( urlScheme == QLatin1String( "sms" ) ) {
        const int pos = url.queryItemValue( QLatin1String( "index" ) ).toInt();

        const KContacts::PhoneNumber::List numbers = mCurrentContact.phoneNumbers();
        if ( pos < numbers.count() ) {
          emit mParent->smsClicked( numbers.at( pos ) );
        }
      } else if ( urlScheme == QLatin1String( "address" ) ) {
        const int pos = url.queryItemValue( QLatin1String( "index" ) ).toInt();

        const KContacts::Address::List addresses = mCurrentContact.addresses();
        if ( pos < addresses.count() ) {
          emit mParent->addressClicked( addresses.at( pos ) );
        }
      } else if (urlScheme == QLatin1String( "mailto" ) ) {
        QString name, address;

        // remove the 'mailto:' and split into name and email address
        KContacts::Addressee::parseEmailAddress( url.path(), name, address );

        emit mParent->emailClicked( name, address );
      }
    }

    void slotParentCollectionFetched( KJob *job )
    {
      mParentCollectionFetchJob = 0;

      QString addressBookName;

      if ( !job->error() ) {
        CollectionFetchJob *fetchJob = qobject_cast<CollectionFetchJob*>( job );
        if ( !fetchJob->collections().isEmpty() ) {
          const Collection collection = fetchJob->collections().first();
          addressBookName = collection.displayName();
        }
      }

      // load the local meta data of the item
      ContactMetaData metaData;
      metaData.load( mCurrentItem );

      updateView( metaData.customFieldDescriptions(), addressBookName );
    }

    ContactViewer *mParent;
    TextBrowser *mBrowser;
    KContacts::Addressee mCurrentContact;
    Item mCurrentItem;
    AbstractContactFormatter *mContactFormatter;
    AbstractContactFormatter *mStandardContactFormatter;
    CollectionFetchJob *mParentCollectionFetchJob;
#ifdef HAVE_PRISON
    prison::AbstractBarcode* mQRCode;
    prison::AbstractBarcode* mDataMatrix;
#endif // HAVE_PRISON
};

ContactViewer::ContactViewer( QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 0 );

  d->mBrowser = new TextBrowser;

  connect( d->mBrowser, SIGNAL(anchorClicked(QUrl)),
          this, SLOT(slotUrlClicked(QUrl)) );

  layout->addWidget( d->mBrowser );

  // always fetch full payload for contacts
  fetchScope().fetchFullPayload();
  fetchScope().fetchAttribute<ContactMetaDataAttribute>();
  fetchScope().setAncestorRetrieval( ItemFetchScope::Parent );
}

ContactViewer::~ContactViewer()
{
  delete d;
}

Akonadi::Item ContactViewer::contact() const
{
  return ItemMonitor::item();
}

KContacts::Addressee ContactViewer::rawContact() const
{
  return d->mCurrentContact;
}

void ContactViewer::setContactFormatter( AbstractContactFormatter *formatter )
{
  if ( formatter == 0 ) {
    d->mContactFormatter = d->mStandardContactFormatter;
  } else {
    d->mContactFormatter = formatter;
  }
}

void ContactViewer::setContact( const Akonadi::Item &contact )
{
  ItemMonitor::setItem( contact );
}

void ContactViewer::setRawContact( const KContacts::Addressee &contact )
{
  d->mCurrentContact = contact;

  d->updateView();
}

void ContactViewer::itemChanged( const Item &contactItem )
{
  if ( !contactItem.hasPayload<KContacts::Addressee>() ) {
    return;
  }

  d->mCurrentItem = contactItem;
  d->mCurrentContact = contactItem.payload<KContacts::Addressee>();

  // stop any running fetch job
  if ( d->mParentCollectionFetchJob ) {
    disconnect( d->mParentCollectionFetchJob, SIGNAL(result(KJob*)), this, SLOT(slotParentCollectionFetched(KJob*)) );
    delete d->mParentCollectionFetchJob;
    d->mParentCollectionFetchJob = 0;
  }

  d->mParentCollectionFetchJob = new CollectionFetchJob( contactItem.parentCollection(), CollectionFetchJob::Base, this );
  connect( d->mParentCollectionFetchJob, SIGNAL(result(KJob*)), SLOT(slotParentCollectionFetched(KJob*)) );
}

void ContactViewer::itemRemoved()
{
  d->mBrowser->clear();
}

#include "moc_contactviewer.cpp"
