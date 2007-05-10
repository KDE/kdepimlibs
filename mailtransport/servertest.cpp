/* This file is part of the KDE project
   Copyright (C) 2007 KovoKs <info@kovoks.nl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

// Own
#include "servertest.h"
#include "socketsafe.h"

// Qt
#include <QProgressBar>
#include <QTimer>

// KDE
#include <klocale.h>
#include <kdebug.h>

using namespace MailTransport;

ServerTest::ServerTest(QWidget* parent )
           : QWidget(parent)
{
    m_normalTimer = new QTimer( this );
    m_normalTimer->setSingleShot( true );
    connect(m_normalTimer, SIGNAL(timeout()), SLOT(slotNormalNotPossible()));

    m_sslTimer = new QTimer( this );
    m_sslTimer->setSingleShot( true );
    connect(m_sslTimer, SIGNAL(timeout()), SLOT(slotSslNotPossible()));

    m_progressTimer = new QTimer( this );
    connect(m_progressTimer, SIGNAL(timeout()), SLOT(slotUpdateProgress()));
}

ServerTest::~ServerTest()
{
}

void ServerTest::start()
{
    kDebug(50002) << k_funcinfo << endl;

    m_testResult.clear();
    m_protocolResult.clear();

    m_testProgress->setMaximum(20);
    m_testProgress->setValue(0);
    m_testProgress->setTextVisible( true );
    m_testProgress->show();
    m_progressTimer->start(1000);

    m_normal = new SocketSafe(this);
    m_normal->setObjectName(QLatin1String("normal"));
    m_normal->setServer(m_server);
    m_normal->setProtocol(m_proto+QLatin1Char('s'));
    if (m_proto == QLatin1String("imap"))
        m_normal->setPort(143);
    else
        m_normal->setPort(25);
    connect(m_normal, SIGNAL(connected()), SLOT(slotNormalPossible()));
    connect(m_normal, SIGNAL(failed()), SLOT(slotNormalNotPossible()));
    connect(m_normal, SIGNAL(data(const QString&)),
            SLOT(slotReadNormal(const QString&)));
    m_normal->reconnect();
    m_normalTimer->start(10000);

    m_ssl = new SocketSafe(this);
    m_ssl->setObjectName(QLatin1String("secure"));
    m_ssl->setServer(m_server);
    m_ssl->setProtocol(m_proto+QLatin1Char('s'));
    if (m_proto == QLatin1String("imap"))
        m_ssl->setPort(993);
    else
        m_ssl->setPort(465);
    m_ssl->setSecure(true);
    connect(m_ssl, SIGNAL(connected()), SLOT(slotSslPossible()));
    connect(m_ssl, SIGNAL(failed()), SLOT(slotSslNotPossible()));
    connect(m_ssl, SIGNAL(data(const QString&)),
            SLOT(slotReadSecure(const QString&)));
    m_ssl->reconnect();
    m_sslTimer->start(10000);
}

void ServerTest::slotNormalPossible()
{
    kDebug(50002) << k_funcinfo << endl;

    m_normalTimer->stop();
    m_testResult[QLatin1String("none")] = true;
}

void ServerTest::slotReadNormal(const QString& text)
{
    static bool first = true;
    if (first)
    {
        if (m_proto == QLatin1String("imap"))
            m_normal->write(QLatin1String("1 CAPABILITY"));
        else if (m_proto == QLatin1String("smtp"))
            m_normal->write(QLatin1String("EHLO localhost"));
        first = false;
        return;
    }
    
    if (text.contains(QLatin1String("STARTTLS"), Qt::CaseInsensitive))
        m_testResult[QLatin1String("tls")] = true;
    else
        m_testResult[QLatin1String("tls")] = false;

    read(QLatin1String("normal"), text);
    
    m_normalFinished = true;
    first = true;
    finalResult();
}

void ServerTest::slotReadSecure(const QString& text)
{
    static bool first = true;
    if (first)
    {
        if (m_proto == QLatin1String("imap"))
            m_ssl->write(QLatin1String("1 CAPABILITY"));
        else if (m_proto == QLatin1String("smtp"))
            m_ssl->write(QLatin1String("EHLO localhost"));
        first = false;
        return;
    }
    
    read(QLatin1String("secure"), text);
    
    m_sslFinished = true;
    first = true;
    finalResult();
}

void ServerTest::read(const QString& type, const QString& text)
{
    kDebug(50002) << k_funcinfo << text << endl;

    if (!text.contains(QLatin1String("AUTH"), Qt::CaseInsensitive))
         return;

    QStringList protocols;
    protocols << QLatin1String("LOGIN") 
              << QLatin1String("PLAIN")
              << QLatin1String("CRAM-MD5")
              << QLatin1String("DIGEST-MD5")
              << QLatin1String("NTLM")
              << QLatin1String("GSSAPI");

    for (int i = 0 ; i < protocols.count(); ++i)
    {
        if (text.contains(protocols.at(i), Qt::CaseInsensitive))
        	m_protocolResult[type].append( protocols.at(i) );
    }
}

void ServerTest::slotNormalNotPossible()
{
    kDebug(50002) << k_funcinfo << endl;
    m_testResult[QLatin1String("tls")] = false;
    m_testResult[QLatin1String("none")] = false;
    m_normalFinished = true;
    finalResult();
}

void ServerTest::slotSslPossible()
{
    kDebug(50002) << k_funcinfo << endl;
    m_sslTimer->stop();
    m_testResult[QLatin1String("ssl")] = true;
    m_sslFinished = true;
    finalResult();
}

void ServerTest::slotSslNotPossible()
{
    kDebug(50002) << k_funcinfo << endl;
    m_testResult[QLatin1String("ssl")] = false;
    m_sslFinished = true;
    finalResult();
}

void ServerTest::finalResult()
{
    if (!m_sslFinished || !m_normalFinished)
        return;

    kDebug(50002) << k_funcinfo << m_testResult << endl;

    m_testProgress->hide();
    m_progressTimer->stop();

    emit finished( m_testResult );
}

void ServerTest::slotUpdateProgress()
{
    m_testProgress->setValue(m_testProgress->value()+1);
}

#include "servertest.moc"
