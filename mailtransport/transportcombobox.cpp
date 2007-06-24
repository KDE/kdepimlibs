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

#include "transport.h"
#include "transportcombobox.h"
#include "transportmanager.h"

#include <kdebug.h>
#include <klocale.h>

#include <qlineedit.h>

using namespace MailTransport;

/**
 * Private class that helps to provide binary compatibility between releases.
 * @internal
 */
class TransportComboBoxPrivate
{
  public:
    QList<int> transports;
};

TransportComboBox::TransportComboBox(QWidget * parent) :
    KComboBox( parent ), d( new TransportComboBoxPrivate )
{
  fillComboBox();
  connect( TransportManager::self(), SIGNAL(transportsChanged()),
           SLOT(fillComboBox()) );
}

int TransportComboBox::currentTransportId() const
{
  if ( currentIndex() == 0 )
    return 0;
  if( currentIndex() > 0 && currentIndex() < d->transports.count() )
    return d->transports.at( currentIndex() );
  return -1;
}

void TransportComboBox::setCurrentTransport(int transportId)
{
  int i = d->transports.indexOf( transportId );
  if ( i >= 0 && i < count() )
    setCurrentIndex( i );
}

bool TransportComboBox::isAdHocTransport() const
{
  return currentTransportId() == -1;
}

TransportBase::EnumType::type TransportComboBox::transportType() const
{
  int transtype = TransportManager::self()->transportById( currentTransportId() )->type();
  return (TransportBase::EnumType::type)transtype;
}

void TransportComboBox::fillComboBox()
{
  int oldTransport = currentTransportId();
  QString oldText;
  if ( lineEdit() )
    oldText = lineEdit()->text();
  clear();
  d->transports.clear();

  if ( !TransportManager::self()->isEmpty() ) {
    QString defName = TransportManager::self()->defaultTransportName();
    if ( defName.isEmpty() )
      addItem( i18n( "Default" ) );
    else
      addItem( i18n( "Default (%1)", defName ) );
    addItems( TransportManager::self()->transportNames() );
    d->transports << 0;
    d->transports << TransportManager::self()->transportIds();
  }

  setCurrentTransport( oldTransport );
  if ( lineEdit() )
    lineEdit()->setText( oldText );
}

#include "transportcombobox.moc"
