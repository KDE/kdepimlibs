/*
    Copyright (c) 2009 Igor Trindade Oliveira <igor_trindade@yahoo.com.br>
    based on kdepimlibs/akonadi/tests/benchmarker.cpp wrote by Robert Zwerus <arzie@dds.nl>

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

#include "maildir.h"

#include <collectiondeletejob.h>
#include <collectionfetchjob.h>
#include <itemdeletejob.h>
#include <itemfetchjob.h>
#include <itemfetchscope.h>
#include <itemmodifyjob.h>

#include <kmime/kmime_message.h>
#include "akonadi/kmime/messageparts.h"


#include <boost/shared_ptr.hpp>

#define WAIT_TIME 100

typedef boost::shared_ptr<KMime::Message> MessagePtr;

using namespace Akonadi;

MailDir::MailDir(const QString &dir) : MakeTest()
{
  createAgent(QLatin1String("akonadi_maildir_resource"));
  configureDBusIface(QLatin1String("Maildir"),dir);
}

MailDir::MailDir() : MakeTest(){}
