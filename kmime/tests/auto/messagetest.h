/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef MESSAGE_TEST_H
#define MESSAGE_TEST_H

#include <kmime/kmime_message.h>
#include <QtCore/QObject>
#include <boost/shared_ptr.hpp>

class MessageTest : public QObject
{
  Q_OBJECT
  private Q_SLOTS:
    void testMainBodyPart();
    void testBrunosMultiAssembleBug();
    void testWillsAndTillsCrash();
    void testDavidsParseCrash();
    void testHeaderFieldWithoutSpace();
    void testWronglyFoldedHeaders();
    void missingHeadersTest();
    void testBug219749();
    void testBidiSpoofing();
    void testUtf16();
    void testDecodedText();
    void testInlineImages();
    void testIssue3908();
    void testIssue3914();
    void testBug223509();
    void testEncapsulatedMessages();
    void testOutlookAttachmentNaming();
    void testEncryptedMails();
    void testCopyFlags();
    void testReturnSameMail();

  private:
    KMime::Message::Ptr readAndParseMail( const QString &mailFile ) const;
};


#endif
