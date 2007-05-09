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

#ifndef MAILTRANSPORT_SERVERTEST_H
#define MAILTRANSPORT_SERVERTEST_H

#include <mailtransport/mailtransport_export.h>

#include <QWidget>
#include <QHash>

class QProgressBar;
class QString;

namespace MailTransport {

class SocketSafe;

/**
 * @class ServerTest
 * This class can be used to test certain server to see if they support stuff.
 * @author Tom Albers <tomalbers@kde.nl>
 */
class MAILTRANSPORT_EXPORT ServerTest : public QWidget
{
    Q_OBJECT

    public:
        /**
         * Constructor
         * @param parent Parent Widget
         */
        ServerTest(QWidget* parent = 0);

        /**
         * Destructor
         */
        ~ServerTest();

        /**
         * Set the server to test.
         */
        void setServer( const QString& server ) { m_server = server; };

        /**
         * Set the progressbar to use. This class will call show() and hide()
         * and will count down.
         */
        void setProgressBar( QProgressBar* pb ) { m_testProgress = pb; };

        /**
         * Set the protocol to test.
         */
        void setProtocol( const QString& proto) {m_proto = proto; };

        /**
         * Start the test.
         */
        void start();

    signals:
        void finished( QHash<QString, bool> );

    private:
        void finalResult();

        QString                 m_server;
        QString                 m_proto;

        SocketSafe*             m_normal;
        SocketSafe*             m_ssl;

        QHash<QString, bool>    m_testResult;

        QTimer*                 m_normalTimer;
        QTimer*                 m_sslTimer;
        QTimer*                 m_progressTimer;

        QProgressBar*           m_testProgress;

        bool                    m_sslFinished;
        bool                    m_normalFinished;

    private slots:
        void slotNormalPossible();
        void slotNormalNotPossible();
        void slotSslPossible();
        void slotSslNotPossible();
        void slotRead(const QString& text);
        void slotUpdateProgress();
};

}

#endif
