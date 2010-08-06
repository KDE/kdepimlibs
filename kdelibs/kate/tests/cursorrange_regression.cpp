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

#include "kateregression.h"

#include <QtCore/QStack>

#include <ktexteditor/editorchooser.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/view.h>
#include <ktexteditor/codecompletioninterface.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <klocale.h>
#include <ksycoca.h>

#include <qtest_kde.h>

#include "cursorexpectation.h"
#include "rangeexpectation.h"

QTEST_KDEMAIN( KateRegression, GUI )

using namespace KTextEditor;

KateRegression* KateRegression::s_self = 0L;

namespace QTest {
  template<>
  char* toString(const Cursor& cursor)
  {
    QByteArray ba = "Cursor(";
    ba += QByteArray::number(cursor.line()) + ", " + QByteArray::number(cursor.column());
    ba += ')';
    return qstrdup(ba.data());
  }

  template<>
  char* toString(const Range& range)
  {
    QByteArray ba = "Range[(";
    ba += QByteArray::number(range.start().line()) + ", " + QByteArray::number(range.start().column());
    ba += ") -> (";
    ba += QByteArray::number(range.end().line()) + ", " + QByteArray::number(range.end().column());
    ba += ")]";
    return qstrdup(ba.data());
  }

  template<>
  char* toString(const SmartRange& range)
  {
    QByteArray ba = "Range[(";
    ba += QByteArray::number(range.start().line()) + ", " + QByteArray::number(range.start().column());
    ba += ") -> (";
    ba += QByteArray::number(range.end().line()) + ", " + QByteArray::number(range.end().column());
    ba += ")]";
    return qstrdup(ba.data());
  }
}

void KateRegression::init()
{
  if ( !KSycoca::isAvailable() )
    QSKIP( "ksycoca not available", SkipAll );

  Editor* editor = EditorChooser::editor();
  QVERIFY(editor);

  m_doc = editor->createDocument(this);

  QVERIFY(m_doc);

  QVERIFY(smart());

/*  01234567890123456789
  0 Test Text
  1 More Test Text
  2 Even More Test Text */
  m_doc->insertText(Cursor(), "Test Text\nMore Test Text\nEven More Test Text");

  cursorStartOfLine = smart()->newSmartCursor(Cursor(1,0));

  cursorStartOfEdit = smart()->newSmartCursor(Cursor(1,5), SmartCursor::StayOnInsert);
  cursorEndOfEdit = smart()->newSmartCursor(Cursor(1,5), SmartCursor::MoveOnInsert);

  rangeEdit = smart()->newSmartRange(*cursorStartOfEdit, *cursorEndOfEdit, 0L, SmartRange::ExpandLeft | SmartRange::ExpandRight);

  rangePreEdit = smart()->newSmartRange(Cursor(1,4), *cursorStartOfEdit, 0L, SmartRange::ExpandLeft);
  rangePostEdit = smart()->newSmartRange(*cursorEndOfEdit, Cursor(2,0), 0L, SmartRange::ExpandRight);
  rangeNextLine = smart()->newSmartRange(Cursor(2,0), Cursor(2,1), 0L, SmartRange::ExpandRight);

  cursorPastEdit = smart()->newSmartCursor(Cursor(1,6));
  cursorEOL = smart()->newSmartCursor(m_doc->endOfLine(1), SmartCursor::StayOnInsert);
  cursorEOLMoves = smart()->newSmartCursor(m_doc->endOfLine(1), SmartCursor::MoveOnInsert);

  cursorNextLine = smart()->newSmartCursor(Cursor(2,0));
}


void KateRegression::testInsertText()
{
  m_doc->clear();

  // Multi-line insert
  Cursor* cursor1 = smart()->newSmartCursor(Cursor(), SmartCursor::StayOnInsert);
  Cursor* cursor2 = smart()->newSmartCursor(Cursor(), SmartCursor::MoveOnInsert);

  m_doc->insertText(Cursor(), "Test Text\nMore Test Text");
  QCOMPARE(m_doc->documentEnd(), Cursor(1,14));

  QString text = m_doc->text(Range(1,0,1,14));
  QCOMPARE(text, QString("More Test Text"));

  // Check cursors and ranges have moved properly
  QCOMPARE(*cursor1, Cursor(0,0));
  QCOMPARE(*cursor2, Cursor(1,14));

  Cursor cursor3 = m_doc->endOfLine(1);

  // Set up a few more lines
  m_doc->insertText(*cursor2, "\nEven More Test Text");
  QCOMPARE(m_doc->documentEnd(), Cursor(2,19));
  QCOMPARE(cursor3, m_doc->endOfLine(1));
}

void KateRegression::testIntraLineInsert()
{

  new CursorExpectation(cursorStartOfLine);
  new CursorExpectation(cursorStartOfEdit, CursorExpectation::CharacterInsertedAfter);
  new CursorExpectation(cursorEndOfEdit, CursorExpectation::CharacterInsertedBefore | CursorExpectation::PositionChanged, Cursor(1,16));
  new CursorExpectation(cursorPastEdit, CursorExpectation::PositionChanged, Cursor(1,17));
  new CursorExpectation(cursorNextLine);

  new RangeExpectation(rangeEdit, RangeExpectation::PositionChanged | RangeExpectation::ContentsChanged, Range(*cursorStartOfEdit, Cursor(1,16)));
  new RangeExpectation(rangePreEdit, RangeExpectation::NoSignal, Range(Cursor(1,4), *cursorStartOfEdit));
  new RangeExpectation(rangePostEdit, RangeExpectation::PositionChanged, Range(1,16,2,0));
  new RangeExpectation(rangeNextLine, RangeExpectation::NoSignal);

  m_doc->insertText(*cursorStartOfEdit, "Additional ");

  QCOMPARE(*cursorEOL, m_doc->endOfLine(1));
  QCOMPARE(*cursorEOLMoves, m_doc->endOfLine(1));

  checkSignalExpectations();
}

void KateRegression::testIntraLineRemove()
{
  m_doc->insertText(*cursorStartOfEdit, "Additional ");

  Cursor* cursorInsideDelete = smart()->newSmartCursor(Cursor(1,7));

  new CursorExpectation(cursorStartOfEdit, CursorExpectation::CharacterDeletedAfter);
  new CursorExpectation(cursorInsideDelete, CursorExpectation::PositionChanged | CursorExpectation::PositionDeleted, *cursorStartOfEdit);
  new CursorExpectation(cursorEndOfEdit, CursorExpectation::CharacterDeletedBefore | CursorExpectation::PositionChanged, *cursorStartOfEdit);
  new CursorExpectation(cursorPastEdit, CursorExpectation::PositionChanged, Cursor(1,6));
  new CursorExpectation(cursorNextLine, CursorExpectation::NoSignal, Cursor(2,0));

  new RangeExpectation(rangeEdit, RangeExpectation::PositionChanged | RangeExpectation::ContentsChanged | RangeExpectation::Eliminated, Range(*cursorStartOfEdit, *cursorStartOfEdit));
  new RangeExpectation(rangePreEdit, RangeExpectation::NoSignal, Range(Cursor(1,4), *cursorStartOfEdit));
  new RangeExpectation(rangePostEdit, RangeExpectation::PositionChanged, Range(1,5,2,0));
  new RangeExpectation(rangeNextLine, RangeExpectation::NoSignal);

  m_doc->removeText(Range(*cursorStartOfEdit, 11));

  QCOMPARE(*cursorEOL, m_doc->endOfLine(1));
  QCOMPARE(*cursorEOLMoves, m_doc->endOfLine(1));

  checkSignalExpectations();
}

void KateRegression::testInsertAtEol()
{
  Cursor oldEOL = *cursorEOL;

  new CursorExpectation(cursorPastEdit);
  new CursorExpectation(cursorEOL, CursorExpectation::CharacterInsertedAfter);
  new CursorExpectation(cursorEOLMoves, CursorExpectation::CharacterInsertedBefore | CursorExpectation::PositionChanged, *cursorEOL + Cursor(0,10));
  new CursorExpectation(cursorNextLine);

  new RangeExpectation(rangeEdit, RangeExpectation::NoSignal);
  new RangeExpectation(rangePreEdit, RangeExpectation::NoSignal);
  new RangeExpectation(rangePostEdit, RangeExpectation::ContentsChanged, Range(1,5,2,0));
  new RangeExpectation(rangeNextLine, RangeExpectation::NoSignal);

  m_doc->insertText(m_doc->endOfLine(1), " Even More");

  checkSignalExpectations();
}

void KateRegression::testWrapLine()
{
  m_doc->insertText(m_doc->endOfLine(1), " Even More");

  *cursorEOL = *cursorEOLMoves;

  new CursorExpectation(cursorPastEdit);
  new CursorExpectation(cursorEOL, CursorExpectation::CharacterInsertedAfter, m_doc->endOfLine(1));
  new CursorExpectation(cursorEOLMoves, CursorExpectation::CharacterInsertedBefore | CursorExpectation::PositionChanged, Cursor(2, 0));
  new CursorExpectation(cursorNextLine, CursorExpectation::PositionChanged, Cursor(3, 0));

  new RangeExpectation(rangeEdit, RangeExpectation::NoSignal);
  new RangeExpectation(rangePreEdit, RangeExpectation::NoSignal);
  new RangeExpectation(rangePostEdit, RangeExpectation::PositionChanged | RangeExpectation::ContentsChanged, Range(1,5,3,0));
  new RangeExpectation(rangeNextLine, RangeExpectation::PositionChanged);

  m_doc->insertText(m_doc->endOfLine(1), "\n");

  checkSignalExpectations();
}

void KateRegression::testRemoveLineWrapping()
{
  m_doc->insertText(m_doc->endOfLine(1), " Even More\n");

  *cursorEOL = m_doc->endOfLine(1);

  new CursorExpectation(cursorPastEdit);
  new CursorExpectation(cursorEOL, CursorExpectation::CharacterDeletedAfter, m_doc->endOfLine(1));
  new CursorExpectation(cursorEOLMoves, CursorExpectation::CharacterDeletedBefore | CursorExpectation::PositionChanged, m_doc->endOfLine(1));
  new CursorExpectation(cursorNextLine, CursorExpectation::PositionChanged, Cursor(2, 0));

  new RangeExpectation(rangeEdit, RangeExpectation::NoSignal);
  new RangeExpectation(rangePreEdit, RangeExpectation::NoSignal);
  new RangeExpectation(rangePostEdit, RangeExpectation::PositionChanged | RangeExpectation::ContentsChanged, Range(1,5,2,0));
  new RangeExpectation(rangeNextLine, RangeExpectation::PositionChanged);

  m_doc->removeText(Range(m_doc->endOfLine(1), Cursor(2, 0)));

  checkSignalExpectations();
}

void KateRegression::testEndOfRangeRemove()
{
  m_doc->insertText(*cursorStartOfEdit, "Additional ");
  QCOMPARE(*rangeEdit, Range(*cursorStartOfEdit, 11));

  Cursor *cursorBeforeDelete = smart()->newSmartCursor(Cursor(1,15));

  new CursorExpectation(cursorBeforeDelete, CursorExpectation::CharacterDeletedAfter);
  new CursorExpectation(cursorEndOfEdit, CursorExpectation::CharacterDeletedBefore | CursorExpectation::PositionChanged, *cursorBeforeDelete);
  new CursorExpectation(cursorPastEdit, CursorExpectation::PositionChanged, *cursorEndOfEdit);
  new CursorExpectation(cursorNextLine, CursorExpectation::NoSignal, Cursor(2,0));

  new RangeExpectation(rangeEdit, RangeExpectation::PositionChanged | RangeExpectation::ContentsChanged, Range(*cursorStartOfEdit, *cursorBeforeDelete));
  new RangeExpectation(rangePreEdit, RangeExpectation::NoSignal, Range(Cursor(1,4), *cursorStartOfEdit));
  new RangeExpectation(rangePostEdit, RangeExpectation::PositionChanged, Range(*cursorBeforeDelete, Cursor(2,0)));
  new RangeExpectation(rangeNextLine, RangeExpectation::NoSignal);

  m_doc->removeText(Range(*cursorBeforeDelete, 1));

  QCOMPARE(*cursorEOL, m_doc->endOfLine(1));
  QCOMPARE(*cursorEOLMoves, m_doc->endOfLine(1));

  checkSignalExpectations();
}

void KateRegression::testDelete()
{
  new RangeExpectation(rangeEdit, RangeExpectation::Deleted);
  new RangeExpectation(rangePreEdit);
  new CursorExpectation(cursorPastEdit, CursorExpectation::Deleted);
  new CursorExpectation(cursorEOL);

  delete rangeEdit;
  delete cursorPastEdit;

  checkSignalExpectations();
}

void KateRegression::testSmartCursor( )
{
  // Test advancing over the current document
  int line = 0;
  int lineLength = m_doc->lineLength(line);
  Cursor* advanceCursor = smart()->newSmartCursor();
  Cursor c;
  do {
    QCOMPARE(*advanceCursor, c);

    if (c.column() == lineLength) {
      c.setPosition(c.line() + 1, 0);
      lineLength = m_doc->lineLength(++line);
    } else {
      c.setColumn(c.column() + 1);
    }

  } while (static_cast<SmartCursor*>(advanceCursor)->advance(1));

  QCOMPARE(*advanceCursor, m_doc->documentRange().end());
}

void KateRegression::testCornerCaseInsertion()
{
  Range* rangeEdit = smart()->newSmartRange(Range(0,0,0,0), 0L);

  m_doc->insertText(Cursor(0,0), "\n");
  QCOMPARE(*rangeEdit, Range(1,0,1,0));
}

void KateRegression::checkSignalExpectations( )
{
  foreach (CursorExpectation* e, m_cursorExpectations)
    e->checkExpectationsFulfilled();
  qDeleteAll(m_cursorExpectations);
  m_cursorExpectations.clear();

  foreach (RangeExpectation* e, m_rangeExpectations)
    e->checkExpectationsFulfilled();
  qDeleteAll(m_rangeExpectations);
  m_rangeExpectations.clear();
}

void KateRegression::addRangeExpectation( RangeExpectation * expectation )
{
  m_rangeExpectations.append(expectation);
}

void KateRegression::addCursorExpectation( CursorExpectation * expectation )
{
  m_cursorExpectations.append(expectation);
}

KateRegression::KateRegression( )
{
  s_self = this;
}

KateRegression * KateRegression::self( )
{
  return s_self;
}

void KateRegression::testRange( )
{
  Range r;
  checkRange(r);
}

void KateRegression::testSmartRange( )
{
  SmartRange* range = smart()->newSmartRange();
  checkRange(*range);
  delete range;
}

void KateRegression::checkRange( Range & valid )
{
  QVERIFY(valid.isValid() && valid.start() <= valid.end());

  Cursor before(0,1), start(0,2), end(1,4), after(1,10);

  Range result(start, end);
  QVERIFY(valid.isValid() && valid.start() <= valid.end());

  valid.setRange(start, end);
  QVERIFY(valid.isValid() && valid.start() <= valid.end());
  QCOMPARE(valid, result);

  valid.setRange(end, start);
  QVERIFY(valid.isValid() && valid.start() <= valid.end());
  QCOMPARE(valid, result);

  valid.start() = after;
  QVERIFY(valid.isValid() && valid.start() <= valid.end());
  QCOMPARE(valid, Range(after, after));

  valid = result;
  QCOMPARE(valid, result);

  valid.end() = before;
  QVERIFY(valid.isValid() && valid.start() <= valid.end());
  QCOMPARE(valid, Range(before, before));
}

void KateRegression::testRangeTree( )
{
  SmartRange* top = smart()->newSmartRange(m_doc->documentRange());

  Range second(1, 2, 1, 10);
  Range* secondLevel = smart()->newSmartRange(second, top);
  QCOMPARE(*secondLevel, second);

  // Check creation restriction
  Range third(1, 1, 1, 11);
  Range* thirdLevel = smart()->newSmartRange(third, static_cast<SmartRange*>(secondLevel));
  QCOMPARE(*thirdLevel, third);

  // Ensure second level has expanded
  QCOMPARE(*secondLevel, third);

  Range fourth(1, 4, 1, 6);
  Range* fourthLevel = smart()->newSmartRange(fourth, static_cast<SmartRange*>(thirdLevel));
  QCOMPARE(*fourthLevel, fourth);

  Range fourth2(1, 7, 1, 8);
  Range* fourth2Level = smart()->newSmartRange(fourth2, static_cast<SmartRange*>(thirdLevel));
  QCOMPARE(*fourthLevel, fourth);
  QCOMPARE(*fourth2Level, fourth2);

  // Check moving start before parent
  thirdLevel->start().setColumn(1);
  QCOMPARE(thirdLevel->start(), Cursor(1,1));
  QCOMPARE(*thirdLevel, *secondLevel);

  // Check moving end after parent
  thirdLevel->end().setColumn(11);
  QCOMPARE(thirdLevel->end(), Cursor(1,11));
  QCOMPARE(*thirdLevel, *secondLevel);

  // Check moving parent after child start
  secondLevel->start() = second.start();
  QCOMPARE(secondLevel->start(), second.start());
  QCOMPARE(*thirdLevel, *secondLevel);

  // Check moving parent before child end
  secondLevel->end() = second.end();
  QCOMPARE(secondLevel->end(), second.end());
  QCOMPARE(*thirdLevel, *secondLevel);

  // Check moving parent before child start
  Range largerSecond(1, 0, 1, 20);
  secondLevel->start() = largerSecond.start();
  QCOMPARE(secondLevel->start(), largerSecond.start());
  QCOMPARE(*thirdLevel, second);

  // Check moving parent after child end
  secondLevel->end() = largerSecond.end();
  QCOMPARE(*secondLevel, largerSecond);
  QCOMPARE(*thirdLevel, second);

  top->deleteChildRanges();

  QVERIFY(!top->childRanges().count());

  // Test out-of-order creation
  Range range1(Cursor(1,2),2);
  Range range2(Cursor(1,5),1);
  Range range3(Cursor(1,6),3);
  SmartRange* child1 = smart()->newSmartRange(range1, static_cast<SmartRange*>(top));
  Range* childR1 = child1;

  SmartRange* child3 = smart()->newSmartRange(range3, static_cast<SmartRange*>(top));
  Range* childR3 = child3;

  SmartRange* child2 = smart()->newSmartRange(range2, static_cast<SmartRange*>(top));
  Range* childR2 = child2;

  QList<SmartRange*> childList;
  childList << child1 << child2 << child3;

  QCOMPARE(childList, top->childRanges());
  QCOMPARE(*childR1, range1);
  QCOMPARE(*childR2, range2);
  QCOMPARE(*childR3, range3);

  // Test moving child ranges
  range2 = Range(Cursor(1,5),3);
  *child2 = range2;

  range3.start() = range2.end();

  QCOMPARE(childList, top->childRanges());
  QCOMPARE(*childR1, range1);
  QCOMPARE(*childR2, range2);
  QCOMPARE(*childR3, range3);

  range2 = Range(Cursor(1,3),5);
  *child2 = range2;

  range1.end() = range2.start();

  QCOMPARE(childList, top->childRanges());
  QCOMPARE(*childR1, range1);
  QCOMPARE(*childR2, range2);
  QCOMPARE(*childR3, range3);

  // Test childBefore / childAfter
  QCOMPARE(top->childBefore(child1), (SmartRange*)0L);
  QCOMPARE(top->childBefore(child2), child1);
  QCOMPARE(top->childBefore(child3), child2);
  QCOMPARE(top->childAfter(child1), child2);
  QCOMPARE(top->childAfter(child2), child3);
  QCOMPARE(top->childAfter(child3), (SmartRange*)0L);

  // Test firstRangeContaining
  Range range11(range1.start(), 1);
  SmartRange* child11 = smart()->newSmartRange(range11, child1);

  Range range111(range11.end(), 0);
  SmartRange* child111 = smart()->newSmartRange(range111, child11);

  QCOMPARE(top->firstRangeContaining(range11.start()), top);

  QStack<SmartRange*> enterStack, exitStack;
  QStack<SmartRange*> expectedEnterStack, expectedExitStack;
  expectedEnterStack << child1 << child11;

  // Test deepestRangeContaining - straight descent
  QCOMPARE(top->deepestRangeContaining(range11.start(), &enterStack, &exitStack), child11);
  QCOMPARE(enterStack, expectedEnterStack);
  QCOMPARE(exitStack, expectedExitStack);

  enterStack.clear();
  expectedExitStack << child2;

  // Test deepestRangeContaining - exit + descent backwards
  QCOMPARE(child2->deepestRangeContaining(range11.start(), &enterStack, &exitStack), child11);
  QCOMPARE(enterStack, expectedEnterStack);
  QCOMPARE(exitStack, expectedExitStack);

  enterStack.clear();
  exitStack.clear();
  expectedExitStack.clear();
  expectedEnterStack.clear();
  expectedExitStack << child11 << child1;
  expectedEnterStack << child2;

  // Test deepestRangeContaining - exit + descent forwards
  QCOMPARE(*child11->deepestRangeContaining(range2.start(), &enterStack, &exitStack), *child2);
  QCOMPARE(enterStack, expectedEnterStack);
  QCOMPARE(exitStack, expectedExitStack);

  enterStack.clear();
  exitStack.clear();
  expectedExitStack.clear();
  expectedEnterStack.clear();
  expectedExitStack << child111 << child11 << child1;

  // Test deepestRangeContaining - exit + descent not past a certain point
  child3->start() = Cursor(1,10);

  QCOMPARE(child111->deepestRangeContaining(Cursor(1,9), &enterStack, &exitStack), top);
  QCOMPARE(enterStack, expectedEnterStack);
  QCOMPARE(exitStack, expectedExitStack);

  top->deleteChildRanges();
  delete top;
}

SmartInterface * KateRegression::smart( ) const
{
  return dynamic_cast<SmartInterface*>(const_cast<Document*>(m_doc));
}



#include "kateregression.moc"
