/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2007 KovoKs <kovoks@kovoks.nl>
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    Based on KMail code by:
    Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>

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

#include "transportconfigdialog.h"
#include "transport.h"
#include "transportconfigwidget.h"
#include "transportmanager.h"
#include "transporttype.h"
#include "sendmailconfigwidget.h"
#include "smtpconfigwidget.h"

#include <QLabel>
#include <QString>

#include <KDebug>
#include <KLocalizedString>

using namespace MailTransport;

class MailTransport::TransportConfigDialog::Private
{
  public:
    Transport *transport;
    QWidget *configWidget;

    // slots
    void okClicked();
};

void TransportConfigDialog::Private::okClicked()
{
  if( TransportConfigWidget *w = dynamic_cast<TransportConfigWidget*>( configWidget ) ) {
    // It is not an Akonadi transport.
    w->apply();
    transport->writeConfig();
  }
}



TransportConfigDialog::TransportConfigDialog( Transport *transport, QWidget *parent )
  : KDialog( parent )
  , d( new Private )
{
  Q_ASSERT( transport );
  d->transport = transport;

  switch( transport->type() ) {
    case Transport::EnumType::SMTP:
      {
        d->configWidget = new SMTPConfigWidget( transport, this );
        break;
      }
    case Transport::EnumType::Sendmail:
      {
        d->configWidget = new SendmailConfigWidget( transport, this );
        break;
      }
    case Transport::EnumType::Akonadi:
      {
        kWarning() << "Tried to configure an Akonadi transport.";
        d->configWidget = new QLabel( i18n( "This transport cannot be configured." ), this );
        break;
      }
    default:
      {
        Q_ASSERT( false );
        d->configWidget = 0;
        break;
      }
  }
  setMainWidget( d->configWidget );

  setButtons( Ok|Cancel );
  connect( this, SIGNAL(okClicked()), this, SLOT(okClicked()) );
}

TransportConfigDialog::~TransportConfigDialog()
{
  delete d;
}

#include "transportconfigdialog.moc"
