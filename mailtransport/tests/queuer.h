/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MESSAGEQUEUER_H
#define MESSAGEQUEUER_H

#include <QWidget>
#include <transportcombobox.h>

class KJob;
class QLineEdit;
class KTextEdit;

namespace MailTransport
{
class MessageQueueJob;
}

/**
  Mostly stolen from transportmgr.{h,cpp}
*/
class MessageQueuer : public QWidget
{
    Q_OBJECT

public:
    MessageQueuer();

private Q_SLOTS:
    void sendNowClicked();
    void sendQueuedClicked();
    void sendOnDateClicked();
    void jobResult(KJob *job);
    void jobPercent(KJob *job, unsigned long percent);
    void jobInfoMessage(KJob *job, const QString &info, const QString &info2);

private:
    MailTransport::TransportComboBox *mComboBox;
    QLineEdit *mSenderEdit, *mToEdit, *mCcEdit, *mBccEdit;
    KTextEdit *mMailEdit;

    MailTransport::MessageQueueJob *createQueueJob();
};

#endif
