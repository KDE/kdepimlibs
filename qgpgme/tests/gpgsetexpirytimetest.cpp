/* tests/gpgsetexpirytimetest.cpp
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

//
// usage:
// gpgsetexpirytimetest <key> <YYYY-MM-DD>
//

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/gpgsetexpirytimeeditinteractor.h>
#include <gpgme++/context.h>
#include <gpgme++/error.h>
#include <gpgme++/data.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

#include <gpg-error.h>

#include <QtCore>

#include <memory>
#include <iostream>
#include <stdexcept>

using namespace GpgME;

int main( int argc, char * argv[] ) {

    QCoreApplication app( argc, argv );

    (void)QGpgME::EventLoopInteractor::instance();

    if ( argc != 3 )
        return 1;

    const Protocol proto = OpenPGP;
    const char * const keyid = argv[1];
    const char * const date = argv[2];

    try {

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
        std::auto_ptr<EditInteractor> ei( new GpgSetExpiryTimeEditInteractor( date ) );

        app.connect( QGpgME::EventLoopInteractor::instance(), SIGNAL(operationDoneEventSignal(GpgME::Context*,GpgME::Error)), SLOT(quit()) );

        if ( Error err = ctx->startEditing( key, ei, data ) )
            throw std::runtime_error( std::string( "startEditing: " ) + gpg_strerror( err ) );
        // ei released in passing to startEditing

        return app.exec();

    } catch ( const std::exception & e ) {
        std::cerr << "Caught error: " << e.what() << std::endl;
        return 1;
    }
}
