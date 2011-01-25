/* tests/gpgagentmonitoreventcounterstest.cpp
   Copyright (C) 2010 Klarälvdalens Datakonsult AB

   This file is part of QGPGME's regression test suite.

   QGPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   QGPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with QGPGME; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA. */

// this is an stripped-down version of kleopatra/smartcard/readerstatus.cpp:

/* -*- mode: c++; c-basic-offset:4 -*-
    smartcard/readerstatus.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <gpgme++/context.h>
#include <gpgme++/assuanresult.h>
#include <gpgme++/defaultassuantransaction.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <gpg-error.h>

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QEventLoop>

#include <boost/shared_ptr.hpp>

#include <memory>
#include <algorithm>
#include <iterator>
#include <cstdlib>

using namespace GpgME;
using namespace boost;


namespace {
    template <typename T_Target, typename T_Source>
    std::auto_ptr<T_Target> dynamic_pointer_cast( std::auto_ptr<T_Source> & in ) {
        if ( T_Target * const target = dynamic_cast<T_Target*>( in.get() ) ) {
            in.release();
            return std::auto_ptr<T_Target>( target );
        } else {
            return std::auto_ptr<T_Target>();
        }
    }
}

template <typename CharT, typename Traits, typename Alloc>
QDebug operator<<( QDebug s, const std::basic_string<CharT,Traits,Alloc> & str ) {
    return s << str.c_str();
}

template <typename T, typename S>
QDebug operator<<( QDebug s, const std::pair<T,S> & p ) {
    return s << "std::pair(" << p.first << ',' << p.second << ')';
}

template <typename T, typename Alloc>
QDebug operator<<( QDebug s, const std::vector<T,Alloc> & v ) {
    for ( typename std::vector<T,Alloc>::const_iterator it = v.begin(), end = v.end() ; it != end ; ++it )
        s << *it << '\n';
    return s;
}

static std::auto_ptr<DefaultAssuanTransaction> gpgagent_transact( shared_ptr<Context> & gpgAgent, const char * command, Error & err ) {
    qDebug() << "gpgagent_transact(" << command << ")";
    const AssuanResult res = gpgAgent->assuanTransact( command );
    err = res.error();
    if ( !err.code() )
        err = res.assuanError();
    if ( err.code() ) {
        qDebug() << "gpgagent_transact(" << command << "):" << QString::fromLocal8Bit( err.asString() );
        if ( err.code() >= GPG_ERR_ASS_GENERAL && err.code() <= GPG_ERR_ASS_UNKNOWN_INQUIRE ) {
            qDebug() << "Assuan problem, killing context";
            gpgAgent.reset();
        }
        return std::auto_ptr<DefaultAssuanTransaction>();
    }
    std::auto_ptr<AssuanTransaction> t = gpgAgent->takeLastAssuanTransaction();
    return dynamic_pointer_cast<DefaultAssuanTransaction>( t );
}

// returns const std::string so template deduction in boost::split works, and we don't need a temporary
static const std::string scd_getattr_status( shared_ptr<Context> & gpgAgent, const char * what, Error & err ) {
    std::string cmd = "SCD GETATTR ";
    cmd += what;
    const std::auto_ptr<DefaultAssuanTransaction> t = gpgagent_transact( gpgAgent, cmd.c_str(), err );
    if ( t.get() ) {
        qDebug() << "scd_getattr_status(" << what << "): got" << t->statusLines();
        return t->firstStatusLine( what );
    } else {
        qDebug() << "scd_getattr_status(" << what << "): t == NULL";
        return std::string();
    }
}

static unsigned int parse_event_counter( const std::string & str ) {
    unsigned int result;
    if ( sscanf( str.c_str(), "%*u %*u %u ", &result ) == 1 )
        return result;
    return -1;
}

static unsigned int get_event_counter( shared_ptr<Context> & gpgAgent ) {
    Error err;
    const std::auto_ptr<DefaultAssuanTransaction> t = gpgagent_transact( gpgAgent, "GETEVENTCOUNTER", err );
    if ( err.code() )
        qDebug() << "get_event_counter(): got error" << err.asString();
    if ( t.get() ) {
        qDebug() << "get_event_counter(): got" << t->statusLines();
        return parse_event_counter( t->firstStatusLine( "EVENTCOUNTER" ) );
    } else {
        qDebug() << "scd_getattr_status(): t == NULL";
        return -1;
    }
}

// returns const std::string so template deduction in boost::split works, and we don't need a temporary
static const std::string gpgagent_data( shared_ptr<Context> & gpgAgent, const char * what, Error & err ) {
    const std::auto_ptr<DefaultAssuanTransaction> t = gpgagent_transact( gpgAgent, what, err );
    if ( t.get() )
        return t->data();
    else
        return std::string();
}

static Error get_card_status( shared_ptr<Context> & gpg_agent ) {
    if ( !gpg_agent )
        return Error( GPG_ERR_INV_VALUE );
    Error err;

    const std::string sernum = gpgagent_data( gpg_agent, "SCD SERIALNO", err );
    if ( err.code() ) {
        qDebug() << "SCD SERIALNO:" << err.asString();
        return err;
    } else {
        qDebug() << "SCD SERIALNO:" << sernum.c_str();
    }

    const std::string apptype = scd_getattr_status( gpg_agent, "APPTYPE", err );
    if ( err.code() ) {
        qDebug() << "SCD GETATTR APPTYPE:" << err.asString();
        return err;
    } else {
        qDebug() << "SCD GETATTR APPTYPE:" << apptype.c_str();
    }

    const std::string appversion = scd_getattr_status( gpg_agent, "NKS-VERSION", err );
    if ( err.code() ) {
        qDebug() << "SCD GETATTR NKS-VERSION:" << err.asString();
        return err;
    } else {
        qDebug() << "SCD GETATTR NKS-VERSION:" << appversion.c_str();
    }

    const std::string chvstatus = scd_getattr_status( gpg_agent, "CHV-STATUS", err );
    if ( err.code() ) {
        qDebug() << "SCD GETATTR CHV-STATUS:" << err.asString();
        return err;
    } else {
        qDebug() << "SCD GETATTR CHV-STATUS:" << chvstatus.c_str();
    }

    // check for keys to learn:
    const std::auto_ptr<DefaultAssuanTransaction> result = gpgagent_transact( gpg_agent, "SCD LEARN --keypairinfo", err );
    if ( err.code() ) {
        qDebug() << "SCD LEARN --keypairinfo:" << err.asString();
        return err;
    } else if ( !result.get() ) {
        qDebug() << "SCD LEARN --keypairinfo:" << "NULL result";
        return Error( GPG_ERR_ASS_GENERAL );
    } else {
        Q_FOREACH( const std::string & s, result->statusLine( "KEYPAIRINFO" ) )
            qDebug() << "SCD LEARN --keypairinfo:" << s.c_str();
    }

    return Error();
}


static bool check_event_counter_changed( shared_ptr<Context> & gpg_agent, unsigned int & counter ) {
    const unsigned int oldCounter = counter;
    counter = get_event_counter( gpg_agent );
    if ( oldCounter != counter ) {
        qDebug() << "ReaderStatusThread[2nd]: events:" << oldCounter << "->" << counter ;
        return true;
    } else {
        return false;
    }
}

static void run() {

    shared_ptr<Context> gpgAgent;
    unsigned int eventCounter = -1;

    for ( unsigned int i = 0 ; i < 100 ; ++i ) {

        if ( !gpgAgent ) {
            Error err;
            std::auto_ptr<Context> c = Context::createForEngine( AssuanEngine, &err );
            if ( err.code() == GPG_ERR_NOT_SUPPORTED )
                return;
            gpgAgent = c;
        }

        // sleep 2 seconds:
        QEventLoop loop;
        QTimer::singleShot( 2000, &loop, SLOT(quit()) );
        loop.exec();

        if ( !check_event_counter_changed( gpgAgent, eventCounter ) )
            continue; // early out

        Error err = get_card_status( gpgAgent );
        bool anyError = err;

        if ( anyError )
            gpgAgent.reset();

        // update event counter in case anything above changed
        // it:
        if ( gpgAgent )
            eventCounter = get_event_counter( gpgAgent );
        else
            eventCounter = -1;

        qDebug() << "eventCounter:" << eventCounter;

    }

}

static void msgHandler( QtMsgType, const char * msg ) {
    QFile file( "gpgagentmonitoreventcounterstest.log" );
    if ( !file.open( QFile::Append|QFile::WriteOnly|QFile::Text ) )
        return;
    QTextStream ts( &file );
    ts << msg << endl;
}

int main( int argc, char * argv[] ) {

    if ( GpgME::initializeLibrary(0) )
        return 1;

    QCoreApplication app( argc, argv );

    qInstallMsgHandler( &msgHandler );

    run();

    return 0;
}
