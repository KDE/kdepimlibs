/* This file is part of the KDE libraries
   Copyright (C) 2005 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KATEREGRESSION_H
#define KATEREGRESSION_H

#include <QtCore/QObject>
#include <QtCore/QMap>

#include <ktexteditor/range.h>

class CursorExpectation;
class RangeExpectation;

namespace KTextEditor {
  class Document;
  class SmartInterface;
}

#ifdef Q_OS_WIN

#include <QtTest/QtTest>
namespace QTest {
  template<>
  char* toString(const KTextEditor::Cursor& cursor);

  template<>
  char* toString(const KTextEditor::Range& r);
}  

#endif 

class KateRegression : public QObject
{
  Q_OBJECT

  public:
    KateRegression();

    static KateRegression* self();

    void addCursorExpectation(CursorExpectation* expectation);
    void addRangeExpectation(RangeExpectation* expectation);

    KTextEditor::SmartInterface* smart() const;

    

  private Q_SLOTS:
    void init();
    void testInsertText();
    void testIntraLineInsert();
    void testIntraLineRemove();
    void testInsertAtEol();
    void testWrapLine();
    void testRemoveLineWrapping();
    void testDelete();
    void testEndOfRangeRemove();

    void testRange();
    void testSmartCursor();
    void testSmartRange();
    void testRangeTree();
    void testCornerCaseInsertion();

  private:
    void checkRange(KTextEditor::Range& valid);
    void checkSmartManager();
    void checkSignalExpectations();

    static KateRegression* s_self;
    KTextEditor::Document* m_doc;
    QList<CursorExpectation*> m_cursorExpectations;
    QList<RangeExpectation*> m_rangeExpectations;

    KTextEditor::Cursor* cursorStartOfLine;
    KTextEditor::Cursor* cursorStartOfEdit;
    KTextEditor::Cursor* cursorEndOfEdit;
    KTextEditor::Range* rangeEdit;
    KTextEditor::Range* rangePreEdit;
    KTextEditor::Range* rangePostEdit;
    KTextEditor::Range* rangeNextLine;
    KTextEditor::Cursor* cursorPastEdit;
    KTextEditor::Cursor* cursorEOL;
    KTextEditor::Cursor* cursorEOLMoves;
    KTextEditor::Cursor* cursorNextLine;
};

#endif
