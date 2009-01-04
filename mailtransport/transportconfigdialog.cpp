/*
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

#include "transportconfigdialog.h"
#include "transport.h"
#include "transportmanager.h"
#include "servertest.h"
#include "mailtransport_defs.h"

#include "ui_smtpsettings.h"
#include "ui_sendmailsettings.h"

#include <kconfigdialogmanager.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>

#include <QButtonGroup>

namespace {

class BusyCursorHelper : public QObject
{
public:
  inline BusyCursorHelper( QObject *parent )
         : QObject( parent )
  {
    qApp->setOverrideCursor( Qt::BusyCursor );
  }

  inline ~BusyCursorHelper()
  {
    qApp->restoreOverrideCursor();
  }
};

}

using namespace MailTransport;

class MailTransport::TransportConfigDialog::Private
{
  public:
    Transport *transport;

    Ui::SMTPSettings smtp;
    Ui::SendmailSettings sendmail;

    KConfigDialogManager *manager;
    KLineEdit *passwordEdit;
    ServerTest *serverTest;
    QButtonGroup *encryptionGroup;
    QButtonGroup *authGroup;

    // detected authentication capabilities
    QList<int> noEncCapa, sslCapa, tlsCapa;

    bool serverTestFailed;

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
      if ( authGroup ) {
        updateAuthCapbilities();
      }
    }

    void updateAuthCapbilities()
    {
      Q_ASSERT( transport->type() == Transport::EnumType::SMTP );

      if ( serverTestFailed ) {
        return;
      }

      QList<int> capa = noEncCapa;
      if ( smtp.ssl->isChecked() ) {
        capa = sslCapa;
      } else if ( smtp.tls->isChecked() ) {
        capa = tlsCapa;
      }

      for ( int i = 0; i < authGroup->buttons().count(); ++i ) {
        authGroup->buttons().at( i )->setEnabled( capa.contains( i ) );
      }

      if ( capa.count() == 0 ) {
        smtp.noAuthPossible->setVisible( true );
        smtp.kcfg_requiresAuthentication->setChecked( false );
        smtp.kcfg_requiresAuthentication->setEnabled( false );
      } else {
        smtp.noAuthPossible->setVisible( false );
        smtp.kcfg_requiresAuthentication->setEnabled( true );
      }
    }
};

TransportConfigDialog::TransportConfigDialog( Transport *transport, QWidget *parent )
  : KDialog( parent ), d( new Private )
{
  Q_ASSERT( transport );

  d->transport = transport;
  d->passwordEdit = 0;
  d->serverTest = 0;
  d->encryptionGroup = 0;
  d->authGroup = 0;
  d->resetAuthCapabilities();

  setButtons( Ok|Cancel|User3 );
  showButton( User3, false );
  setButtonText( User3, i18n( "Use Sendmail" ) );
  connect( this, SIGNAL( user3Clicked() ), SLOT( slotUser3() ) );
  connect( this, SIGNAL(okClicked()), SLOT(save()) );
  connect( TransportManager::self(), SIGNAL(passwordsChanged()),
           SLOT(passwordsLoaded()) );

  switch ( transport->type() ) {
    case Transport::EnumType::SMTP:
    {
      showButton( User3, true );

      d->smtp.setupUi( mainWidget() );
      d->passwordEdit = d->smtp.password;

      d->encryptionGroup = new QButtonGroup( this );
      d->encryptionGroup->addButton( d->smtp.none );
      d->encryptionGroup->addButton( d->smtp.ssl );
      d->encryptionGroup->addButton( d->smtp.tls );

      d->authGroup = new QButtonGroup( this );
      d->authGroup->addButton( d->smtp.login );
      d->authGroup->addButton( d->smtp.plain );
      d->authGroup->addButton( d->smtp.crammd5 );
      d->authGroup->addButton( d->smtp.digestmd5 );
      d->authGroup->addButton( d->smtp.gssapi );
      d->authGroup->addButton( d->smtp.ntlm );

      if ( KProtocolInfo::capabilities( SMTP_PROTOCOL ).contains( QLatin1String( "SASL" ) ) == 0 ) {
        d->smtp.ntlm->hide();
        d->smtp.gssapi->hide();
      }

      connect( d->smtp.checkCapabilities, SIGNAL(clicked()),
               SLOT(checkSmtpCapabilities()) );
      connect( d->smtp.kcfg_host, SIGNAL(textChanged(QString)),
               SLOT(hostNameChanged(QString)) );
      connect( d->smtp.kcfg_encryption, SIGNAL(clicked(int)),
               SLOT(encryptionChanged(int)) );
      connect( d->smtp.kcfg_requiresAuthentication, SIGNAL( toggled(bool) ),
               SLOT( ensureValidAuthSelection() ) );
      break;
    }
    case Transport::EnumType::Sendmail:
    {
      d->sendmail.setupUi( mainWidget() );

      connect( d->sendmail.chooseButton, SIGNAL(clicked()),
               SLOT(chooseSendmail()) );
      connect( d->sendmail.kcfg_host, SIGNAL(textChanged(QString)),
               SLOT(hostNameChanged(QString)) );
    }
  }

  // load the password if necessary
  if ( d->passwordEdit ) {
    if ( d->transport->isComplete() ) {
      d->passwordEdit->setText( d->transport->password() );
    } else {
      if ( d->transport->requiresAuthentication() ) {
        TransportManager::self()->loadPasswordsAsync();
      }
    }
  }

  d->manager = new KConfigDialogManager( this, transport );
  d->manager->updateWidgets();
  hostNameChanged( d->transport->host() );
}

TransportConfigDialog::~ TransportConfigDialog()
{
  delete d;
}

void TransportConfigDialog::checkSmtpCapabilities()
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::SMTP );

  d->serverTest = new ServerTest( this );
  d->serverTest->setProtocol( SMTP_PROTOCOL );
  d->serverTest->setServer( d->smtp.kcfg_host->text().trimmed() );
  if ( d->smtp.kcfg_specifyHostname->isChecked() ) {
    d->serverTest->setFakeHostname( d->smtp.kcfg_localHostname->text() );
  }
  d->serverTest->setProgressBar( d->smtp.checkCapabilitiesProgress );
  BusyCursorHelper *busyCursorHelper = new BusyCursorHelper( d->serverTest );

  connect( d->serverTest, SIGNAL(finished( QList< int > )),
           SLOT(slotFinished( QList< int > )));
  connect( d->serverTest, SIGNAL(finished( QList< int > )),
           busyCursorHelper, SLOT(deleteLater()) );
  d->smtp.checkCapabilities->setEnabled( false );
  d->serverTest->start();
  d->serverTestFailed = false;
}

void TransportConfigDialog::save()
{
  d->manager->updateSettings();
  if ( d->passwordEdit ) {
    d->transport->setPassword( d->passwordEdit->text() );
  }

  // enforce unique name
  QStringList existingNames;
  foreach ( Transport *t, TransportManager::self()->transports() ) {
    if ( t->id() != d->transport->id() ) {
      existingNames << t->name();
    }
  }
  int suffix = 1;
  QString origName = d->transport->name();
  while ( existingNames.contains( d->transport->name() ) ) {
    d->transport->setName( i18nc( "%1: name; %2: number appended to it to make "
                                  "it unique among a list of names", "%1 %2", origName, suffix ) );
    ++suffix;
  }

  d->transport->writeConfig();
}

void TransportConfigDialog::slotUser3()
{
  reject();
  emit sendmailClicked();
}

void TransportConfigDialog::chooseSendmail()
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::Sendmail );

  KFileDialog dialog( KUrl( "/" ), QString(), this );
  dialog.setCaption( i18n( "Choose sendmail Location" ) );

  if ( dialog.exec() == QDialog::Accepted ) {
    KUrl url = dialog.selectedUrl();
    if ( url.isEmpty() == true ) {
      return;
    }
    if ( !url.isLocalFile() ) {
      KMessageBox::sorry( this, i18n( "Only local files allowed." ) );
      return;
    }
    d->sendmail.kcfg_host->setText( url.path() );
  }
}

void TransportConfigDialog::passwordsLoaded()
{
  Q_ASSERT( d->passwordEdit );

  // Load the password from the original to our cloned copy
  d->transport->updatePasswordState();

  if ( d->passwordEdit->text().isEmpty() ) {
    d->passwordEdit->setText( d->transport->password() );
  }
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

void TransportConfigDialog::slotFinished( QList<int> results )
{
  d->smtp.checkCapabilities->setEnabled( true );
  d->serverTest->deleteLater();

  // If the servertest did not find any useable authentication modes, assume the
  // connection failed and don't disable any of the radioboxes.
  if ( results.isEmpty() ) {
    d->serverTestFailed = true;
    return;
  }

  // encryption method
  d->smtp.none->setEnabled( results.contains( Transport::EnumEncryption::None ) );
  d->smtp.ssl->setEnabled( results.contains( Transport::EnumEncryption::SSL ) );
  d->smtp.tls->setEnabled( results.contains( Transport::EnumEncryption::TLS ) );
  checkHighestEnabledButton( d->encryptionGroup );

  d->noEncCapa = d->serverTest->normalProtocols();
  if ( d->smtp.tls->isEnabled() ) {
    d->tlsCapa = d->serverTest->tlsProtocols();
  } else {
    d->tlsCapa.clear();
  }
  d->sslCapa = d->serverTest->secureProtocols();
  d->updateAuthCapbilities();
  checkHighestEnabledButton( d->authGroup );
}

void TransportConfigDialog::hostNameChanged( const QString &text )
{
  // sanitize hostname...
  if ( d->transport->type() == Transport::EnumType::Sendmail ) {
    d->sendmail.kcfg_host->blockSignals( true );
    d->sendmail.kcfg_host->setText( text.trimmed() );
    d->sendmail.kcfg_host->blockSignals( false );
  } else if ( d->transport->type() == Transport::EnumType::SMTP ) {
    d->smtp.kcfg_host->blockSignals( true );
    d->smtp.kcfg_host->setText( text.trimmed() );
    d->smtp.kcfg_host->blockSignals( false );
  }

  d->resetAuthCapabilities();
  enableButton( Ok, !text.isEmpty() );
  for ( int i = 0; d->encryptionGroup && i < d->encryptionGroup->buttons().count(); i++ ) {
    d->encryptionGroup->buttons().at( i )->setEnabled( true );
  }
}

void TransportConfigDialog::ensureValidAuthSelection()
{
  // adjust available authentication methods
  d->updateAuthCapbilities();
  foreach ( QAbstractButton *b, d->authGroup->buttons() ) {
    if ( b->isChecked() && !b->isEnabled() ) {
      checkHighestEnabledButton( d->authGroup );
      break;
    }
  }
}

void TransportConfigDialog::encryptionChanged( int enc )
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::SMTP );
  kDebug() << enc;

  // adjust port
  if ( enc == Transport::EnumEncryption::SSL ) {
    if ( d->smtp.kcfg_port->value() == SMTP_PORT ) {
      d->smtp.kcfg_port->setValue( SMTPS_PORT );
    }
  } else {
    if ( d->smtp.kcfg_port->value() == SMTPS_PORT ) {
      d->smtp.kcfg_port->setValue( SMTP_PORT );
    }
  }

  ensureValidAuthSelection();
}

#include "transportconfigdialog.moc"
