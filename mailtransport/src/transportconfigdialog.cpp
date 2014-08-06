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

#include <QDialogButtonBox>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>
#include <QPushButton>

#include <QDebug>
#include <KLocalizedString>

using namespace MailTransport;

class MailTransport::TransportConfigDialog::Private
{
public:
    Private(TransportConfigDialog *qq)
        : transport(0), configWidget(0), q(qq), okButton(0)
    {
    }

    Transport *transport;
    QWidget *configWidget;
    TransportConfigDialog *q;
    QPushButton *okButton;

    // slots
    void okClicked();
    void slotTextChanged(const QString &text);
    void slotEnabledOkButton(bool);
};

void TransportConfigDialog::Private::slotEnabledOkButton(bool b)
{
    okButton->setEnabled(b);
}

void TransportConfigDialog::Private::okClicked()
{
    if (TransportConfigWidget *w = dynamic_cast<TransportConfigWidget *>(configWidget)) {
        // It is not an Akonadi transport.
        w->apply();
        transport->save();
    }
}

void TransportConfigDialog::Private::slotTextChanged(const QString &text)
{
    okButton->setEnabled(!text.isEmpty());
}

TransportConfigDialog::TransportConfigDialog(Transport *transport, QWidget *parent)
    : QDialog(parent), d(new Private(this))
{
    Q_ASSERT(transport);
    d->transport = transport;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    bool pathIsEmpty = false;
    switch (transport->type()) {
    case Transport::EnumType::SMTP: {
        d->configWidget = new SMTPConfigWidget(transport, this);
        break;
    }
    case Transport::EnumType::Sendmail: {
        SendmailConfigWidget *sendMailWidget = new SendmailConfigWidget(transport, this);
        d->configWidget = sendMailWidget;
        connect(sendMailWidget, SIGNAL(enableButtonOk(bool)),
                this, SLOT(slotEnabledOkButton(bool)));
        pathIsEmpty = sendMailWidget->pathIsEmpty();
        break;
    }
    case Transport::EnumType::Akonadi: {
        qWarning() << "Tried to configure an Akonadi transport.";
        d->configWidget = new QLabel(i18n("This outgoing account cannot be configured."), this);
        break;
    }
    default: {
        Q_ASSERT(false);
        d->configWidget = 0;
        break;
    }
    }
    mainLayout->addWidget(d->configWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    d->okButton = buttonBox->button(QDialogButtonBox::Ok);
    d->okButton->setText(i18nc("create and configure a mail transport", "Create and Configure"));
    d->okButton->setEnabled(false);
    d->okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mainLayout->addWidget(buttonBox);

    connect(d->okButton, SIGNAL(clicked()), this, SLOT(okClicked()));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TransportConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TransportConfigDialog::reject);

    d->okButton->setEnabled(!pathIsEmpty);
}

TransportConfigDialog::~TransportConfigDialog()
{
    delete d;
}

#include "moc_transportconfigdialog.cpp"
