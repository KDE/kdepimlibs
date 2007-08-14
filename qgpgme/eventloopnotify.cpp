/* qeventloopnotify.cpp
   Copyright (C) 2007 Klarälvdalens Datakonsult AB

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

#include <gpgme++/context.h>

#include <QSocketNotifier>
#include <QDebug>
#include "eventloopnotify.h"

using namespace GpgME;


QGpgME::EventLoopNotify::EventLoopNotify(int fd, EventLoopInteractor::Direction dir)
 : sn(new QSocketNotifier( fd, dir == EventLoopInteractor::Read ? QSocketNotifier::Read : QSocketNotifier::Write ))
{
  if ( dir == EventLoopInteractor::Read )
    connect( sn, SIGNAL(activated(int)), SIGNAL(activated(int)) );
  else
    connect( sn, SIGNAL(activated(int)), SIGNAL(activated(int)) );
}

QGpgME::EventLoopNotify::~EventLoopNotify()
{
  delete sn;
}

#include "eventloopnotify.moc"
