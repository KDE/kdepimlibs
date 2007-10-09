/* tests/test_editinteractor.h
   Copyright (C) 2007 Klarälvdalens Datakonsult AB

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

// -*- c++ -*-

#ifndef __QGPGME_TESTS_TEST_EDITINTERACTOR_H__
#define __QGPGME_TESTS_TEST_EDITINTERACTOR_H__

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/editinteractor.h>
#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <gpg-error.h>

#include <QtCore>

#include <memory>
#include <stdexcept>

static int test_editinteractor( std::auto_ptr<GpgME::EditInteractor> ei, const char * keyid, GpgME::Protocol proto=GpgME::OpenPGP ) {

    using namespace GpgME;

    (void)QGpgME::EventLoopInteractor::instance();

    Key key;
    {
        const std::auto_ptr<Context> kl( Context::createForProtocol( proto ) );

        if ( !kl.get() )
            return 1;

        if ( Error err = kl->startKeyListing( keyid ) )
            throw std::runtime_error( std::string( "startKeyListing: " ) + gpg_strerror( err ) );

        Error err;
        key = kl->nextKey( err );
        if ( err )
            throw std::runtime_error( std::string( "nextKey: " ) + gpg_strerror( err ) );

        (void)kl->endKeyListing();
    }
        

    const std::auto_ptr<Context> ctx( Context::createForProtocol( proto ) );

    ctx->setManagedByEventLoopInteractor( true );

    Data data;
    ei->setDebugChannel( stderr );

    QObject::connect( QGpgME::EventLoopInteractor::instance(), SIGNAL(operationDoneEventSignal(GpgME::Context*,GpgME::Error)),
                      QCoreApplication::instance(), SLOT(quit()) );

    if ( Error err = ctx->startEditing( key, ei, data ) )
        throw std::runtime_error( std::string( "startEditing: " ) + gpg_strerror( err ) );
    // ei released in passing to startEditing

    return QCoreApplication::instance()->exec();
}



#endif // __QGPGME_TESTS_TEST_EDITINTERACTOR_H__
