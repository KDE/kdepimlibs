/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

    Based on MailTransport code by:
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2007 KovoKs <kovoks@kovoks.nl>

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

#include "smtpconfigwidget.h"
#include "transportconfigwidget_p.h"
#include "transport.h"
#include "transportmanager.h"
#include "servertest.h"
#include "mailtransport_defs.h"

#ifndef KDEPIM_MOBILE_UI
#include "ui_smtpsettings_desktop.h"
#else
#include "ui_smtpsettings_mobile.h"
#endif

#include <QAbstractButton>
#include <QButtonGroup>

#include <KProtocolInfo>
#include <KDebug>
#include <KMessageBox>

namespace {

// TODO: is this really necessary?
class BusyCursorHelper : public QObject
{
  public:
    inline BusyCursorHelper( QObject *parent ) : QObject( parent )
    {
#ifndef QT_NO_CURSOR
      qApp->setOverrideCursor( Qt::BusyCursor );
#endif
    }

    inline ~BusyCursorHelper()
    {
#ifndef QT_NO_CURSOR
      qApp->restoreOverrideCursor();
#endif
    }
};

}

using namespace MailTransport;

class MailTransport::SMTPConfigWidgetPrivate : public TransportConfigWidgetPrivate
{
  public:
    ::Ui::SMTPSettings ui;

    ServerTest *serverTest;
    QButtonGroup *encryptionGroup;

    // detected authentication capabilities
    QList<int> noEncCapa, sslCapa, tlsCapa;

    bool serverTestFailed;

    static void addAuthenticationItem( KComboBox *combo,
                                   int authenticationType )
    {
      combo->addItem( Transport::authenticationTypeString( authenticationType ),
                  QVariant( authenticationType ) );
    }

    void resetAuthCapabilities()
    {
      noEncCapa.clear();
      noEncCapa << Transport::EnumAuthenticationType::LOGIN
                << Transport::EnumAuthenticationType::PLAIN
                << Transport::EnumAuthenticationType::CRAM_MD5
                << Transport::EnumAuthenticationType::DIGEST_MD5
                << Transport::EnumAuthenticationType::NTLM
                << Transport::EnumAuthenticationType::GSSAPI;
      sslCapa = tlsCapa = noEncCapa;
      updateAuthCapbilities();

    }

    void updateAuthCapbilities()
    {
      if ( serverTestFailed ) {
        return;
      }

      QList<int> capa = noEncCapa;
      if ( ui.ssl->isChecked() ) {
        capa = sslCapa;
      } else if ( ui.tls->isChecked() ) {
        capa = tlsCapa;
      }

      ui.authCombo->clear();
      foreach ( int authType, capa ) {
        addAuthenticationItem( ui.authCombo, authType );
      }

      if ( transport->isValid() ) {
        const int idx = ui.authCombo->findData( transport->authenticationType() );

        if ( idx != -1 ) {
          ui.authCombo->setCurrentIndex( idx );
        }
      }

      if ( capa.count() == 0 ) {
        ui.noAuthPossible->setVisible( true );
        ui.kcfg_requiresAuthentication->setChecked( false );
        ui.kcfg_requiresAuthentication->setEnabled( false );
        ui.kcfg_requiresAuthentication->setVisible( false );
        ui.authCombo->setEnabled( false );
        ui.authLabel->setEnabled( false );
      } else {
        ui.noAuthPossible->setVisible( false );
        ui.kcfg_requiresAuthentication->setEnabled( true );
        ui.kcfg_requiresAuthentication->setVisible( true );
        ui.authCombo->setEnabled( true );
        ui.authLabel->setEnabled( true );
      }
    }
};

SMTPConfigWidget::SMTPConfigWidget( Transport *transport, QWidget *parent )
  : TransportConfigWidget( *new SMTPConfigWidgetPrivate, transport, parent )
{
  init();
}

SMTPConfigWidget::SMTPConfigWidget( SMTPConfigWidgetPrivate &dd,
                                    Transport *transport, QWidget *parent )
  : TransportConfigWidget( dd, transport, parent )
{
  init();
}

static void checkHighestEnabledButton( QButtonGroup *group )
{
  Q_ASSERT( group );

  for ( int i = group->buttons().count() - 1; i >= 0; --i ) {
    QAbstractButton *b = group->buttons().at( i );
    if ( b && b->isEnabled() ) {
      b->animateClick();
      return;
    }
  }
}

void SMTPConfigWidget::init()
{
  Q_D( SMTPConfigWidget );
  d->serverTest = 0;

  connect( TransportManager::self(), SIGNAL(passwordsChanged()),
           SLOT(passwordsLoaded()) );

  d->serverTestFailed = false;

  d->ui.setupUi( this );
  d->manager->addWidget( this ); // otherwise it doesn't find out about these widgets
  d->manager->updateWidgets();

  d->encryptionGroup = new QButtonGroup( this );
  d->encryptionGroup->addButton( d->ui.none, Transport::EnumEncryption::None );
  d->encryptionGroup->addButton( d->ui.ssl, Transport::EnumEncryption::SSL );
  d->encryptionGroup->addButton( d->ui.tls, Transport::EnumEncryption::TLS );

  d->resetAuthCapabilities();

  if ( KProtocolInfo::capabilities( SMTP_PROTOCOL ).contains( QLatin1String( "SASL" ) ) == 0 ) {
    d->ui.authCombo->removeItem( d->ui.authCombo->findData(
                                 Transport::EnumAuthenticationType::NTLM ) );
    d->ui.authCombo->removeItem( d->ui.authCombo->findData(
                                 Transport::EnumAuthenticationType::GSSAPI ) );
  }

  connect( d->ui.checkCapabilities, SIGNAL(clicked()),
           SLOT(checkSmtpCapabilities()) );
  connect( d->ui.kcfg_host, SIGNAL(textChanged(QString)),
           SLOT(hostNameChanged(QString)) );
  connect( d->encryptionGroup, SIGNAL(buttonClicked(int)),
           SLOT(encryptionChanged(int)) );
  connect( d->ui.kcfg_requiresAuthentication, SIGNAL(toggled(bool)),
           SLOT(ensureValidAuthSelection()) );

  if ( !d->transport->isValid() ) {
    checkHighestEnabledButton( d->encryptionGroup );
  }

  // load the password
  d->transport->updatePasswordState();
  if ( d->transport->isComplete() ) {
    d->ui.password->setText( d->transport->password() );
  } else {
    if ( d->transport->requiresAuthentication() ) {
      TransportManager::self()->loadPasswordsAsync();
    }
  }

  hostNameChanged( d->transport->host() );

#ifdef KDEPIM_MOBILE_UI
  d->ui.smtpSettingsGroupBox->hide();
#endif
}

void SMTPConfigWidget::checkSmtpCapabilities()
{
  Q_D( SMTPConfigWidget );

  d->serverTest = new ServerTest( this );
  d->serverTest->setProtocol( SMTP_PROTOCOL );
  d->serverTest->setServer( d->ui.kcfg_host->text().trimmed() );
  if ( d->ui.kcfg_specifyHostname->isChecked() ) {
    d->serverTest->setFakeHostname( d->ui.kcfg_localHostname->text() );
  }
  QAbstractButton *encryptionChecked = d->encryptionGroup->checkedButton();
  if (encryptionChecked == d->ui.none) {
      d->serverTest->setPort( Transport::EnumEncryption::None, d->ui.kcfg_port->value());
  } else if (encryptionChecked == d->ui.ssl) {
      d->serverTest->setPort( Transport::EnumEncryption::SSL, d->ui.kcfg_port->value());
  }
  d->serverTest->setProgressBar( d->ui.checkCapabilitiesProgress );
  d->ui.checkCapabilitiesStack->setCurrentIndex( 1 );
  BusyCursorHelper *busyCursorHelper = new BusyCursorHelper( d->serverTest );

  connect( d->serverTest, SIGNAL(finished(QList<int>)),
           SLOT(slotFinished(QList<int>)));
  connect( d->serverTest, SIGNAL(finished(QList<int>)),
           busyCursorHelper, SLOT(deleteLater()) );
  d->ui.checkCapabilities->setEnabled( false );
  d->serverTest->start();
  d->serverTestFailed = false;
}

void SMTPConfigWidget::apply()
{
  Q_D( SMTPConfigWidget );
  Q_ASSERT( d->manager );
  d->manager->updateSettings();
  d->transport->setPassword( d->ui.password->text() );

  KConfigGroup group( d->transport->config(), d->transport->currentGroup() );
  const int index = d->ui.authCombo->currentIndex();
  if ( index >= 0 ) {
    group.writeEntry( "authtype", d->ui.authCombo->itemData( index ).toInt() );
  }

  TransportConfigWidget::apply();
}

void SMTPConfigWidget::passwordsLoaded()
{
  Q_D( SMTPConfigWidget );

  // Load the password from the original to our cloned copy
  d->transport->updatePasswordState();

  if ( d->ui.password->text().isEmpty() ) {
    d->ui.password->setText( d->transport->password() );
  }
}

// TODO rename
void SMTPConfigWidget::slotFinished( QList<int> results )
{
  Q_D( SMTPConfigWidget );

  d->ui.checkCapabilitiesStack->setCurrentIndex( 0 );

  d->ui.checkCapabilities->setEnabled( true );
  d->serverTest->deleteLater();

  // If the servertest did not find any useable authentication modes, assume the
  // connection failed and don't disable any of the radioboxes.
  if ( results.isEmpty() ) {
    KMessageBox::error(this, i18n("Failed to check capabilities. Please verify port and authentication mode."), i18n("Check Capabilities Failed"));
    d->serverTestFailed = true;
    return;
  }

  // encryption method
  d->ui.none->setEnabled( results.contains( Transport::EnumEncryption::None ) );
  d->ui.ssl->setEnabled( results.contains( Transport::EnumEncryption::SSL ) );
  d->ui.tls->setEnabled( results.contains( Transport::EnumEncryption::TLS ) );
  checkHighestEnabledButton( d->encryptionGroup );

  d->noEncCapa = d->serverTest->normalProtocols();
  if ( d->ui.tls->isEnabled() ) {
    d->tlsCapa = d->serverTest->tlsProtocols();
  } else {
    d->tlsCapa.clear();
  }
  d->sslCapa = d->serverTest->secureProtocols();
  d->updateAuthCapbilities();
}

void SMTPConfigWidget::hostNameChanged( const QString &text )
{
  // TODO: really? is this done at every change? wtf

  Q_D( SMTPConfigWidget );

  // sanitize hostname...
  int pos = d->ui.kcfg_host->cursorPosition();
  d->ui.kcfg_host->blockSignals( true );
  d->ui.kcfg_host->setText( text.trimmed() );
  d->ui.kcfg_host->blockSignals( false );
  d->ui.kcfg_host->setCursorPosition( pos );

  d->resetAuthCapabilities();
  for ( int i = 0; d->encryptionGroup && i < d->encryptionGroup->buttons().count(); i++ ) {
    d->encryptionGroup->buttons().at( i )->setEnabled( true );
  }
}

void SMTPConfigWidget::ensureValidAuthSelection()
{
  Q_D( SMTPConfigWidget );

  // adjust available authentication methods
  d->updateAuthCapbilities();
}

void SMTPConfigWidget::encryptionChanged( int enc )
{
  Q_D( SMTPConfigWidget );
  kDebug() << enc;

  // adjust port
  if ( enc == Transport::EnumEncryption::SSL ) {
    if ( d->ui.kcfg_port->value() == SMTP_PORT ) {
      d->ui.kcfg_port->setValue( SMTPS_PORT );
    }
  } else {
    if ( d->ui.kcfg_port->value() == SMTPS_PORT ) {
      d->ui.kcfg_port->setValue( SMTP_PORT );
    }
  }

  ensureValidAuthSelection();
}

