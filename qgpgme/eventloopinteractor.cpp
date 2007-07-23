/* qeventloopinteractor.cpp
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

   This file is part of QGPGME.

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

#include <qgpgme/eventloopinteractor.h>
#include <gpgme++/context.h>

#include <QSocketNotifier>
#include <QCoreApplication>

using namespace GpgME;

QGpgME::EventLoopInteractor::EventLoopInteractor( QObject * parent )
 : QObject( parent ), GpgME::EventLoopInteractor()
{
  setObjectName( QLatin1String( "QGpgME::EventLoopInteractor::instance()" ) );
  if ( !parent )
    if ( QCoreApplication * const app = QCoreApplication::instance() ) {
      connect( app, SIGNAL(aboutToQuit()), SLOT(deleteLater()) );
      connect( app, SIGNAL(aboutToQuit()), SIGNAL(aboutToDestroy()) );
    }
  mSelf = this;
}

QGpgME::EventLoopInteractor::~EventLoopInteractor() {
  emit aboutToDestroy();
  mSelf = 0;
}

QGpgME::EventLoopInteractor * QGpgME::EventLoopInteractor::mSelf = 0;

QGpgME::EventLoopInteractor * QGpgME::EventLoopInteractor::instance() {
  if ( !mSelf )
#ifndef NDEBUG
    if ( !QCoreApplication::instance() )
      qWarning( "QGpgME::EventLoopInteractor: Need a Q(Core)Application object before calling instance()!" );
    else
#endif
     (void)new EventLoopInteractor;
  return mSelf;
}

void * QGpgME::EventLoopInteractor::registerWatcher( int fd, Direction dir, bool & ok ) {
  QSocketNotifier * const sn = new QSocketNotifier( fd,
      dir == Read ? QSocketNotifier::Read : QSocketNotifier::Write );
  if ( dir == Read )
    connect( sn, SIGNAL(activated(int)), SLOT(slotReadActivity(int)) );
  else
    connect( sn, SIGNAL(activated(int)), SLOT(slotWriteActivity(int)) );
  ok = true; // Can above operations fails?
  return sn;
}

void QGpgME::EventLoopInteractor::unregisterWatcher( void * tag ) {
  delete static_cast<QSocketNotifier*>( tag );
}

void QGpgME::EventLoopInteractor::slotWriteActivity( int socket ) {
  actOn( socket , Write );
}

void QGpgME::EventLoopInteractor::slotReadActivity( int socket ) {
  actOn( socket , Read );
}

void QGpgME::EventLoopInteractor::nextTrustItemEvent( GpgME::Context * context, const GpgME::TrustItem & item ) {
  emit nextTrustItemEventSignal( context, item );
}

void QGpgME::EventLoopInteractor::nextKeyEvent( GpgME::Context * context, const GpgME::Key & key ) {
  emit nextKeyEventSignal( context, key );
}

void QGpgME::EventLoopInteractor::operationDoneEvent( GpgME::Context * context, const GpgME::Error & e ) {
  emit operationDoneEventSignal( context, e );
}

#include "eventloopinteractor.moc"
