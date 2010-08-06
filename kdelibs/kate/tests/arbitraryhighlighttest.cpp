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

#include "arbitraryhighlighttest.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/smartrangenotifier.h>
#include <ktexteditor/attribute.h>

#include <QtCore/QTimer>

using namespace KTextEditor;

ArbitraryHighlightTest::ArbitraryHighlightTest(Document* parent)
  : QObject(parent)
  , m_topRange(0L)
{
  QTimer::singleShot(0, this, SLOT(slotCreateTopRange()));
}

ArbitraryHighlightTest::~ArbitraryHighlightTest()
{
}

Document * ArbitraryHighlightTest::doc( ) const
{
  return qobject_cast<Document*>(const_cast<QObject*>(parent()));
}

KTextEditor::SmartInterface * ArbitraryHighlightTest::smart( ) const
{
  return dynamic_cast<SmartInterface*>(doc());
}

void ArbitraryHighlightTest::slotRangeChanged(SmartRange* range, SmartRange* mostSpecificChild)
{
  static Attribute::Ptr ranges[10] = {Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr(),Attribute::Ptr()};
  static const QChar openBrace = QChar('{');
  static const QChar closeBrace = QChar('}');

  if (!ranges[0]) {
    for (int i = 0; i < 10; ++i) {
      ranges[i] = new Attribute();
      ranges[i]->setBackground(QColor(0xFF - (i * 0x20), 0xFF, 0xFF));
    }
    //ranges[2]->setFontBold();
    //ranges[2]->setForeground(Qt::red);

    Attribute::Ptr dyn(new Attribute());
    dyn->setBackground(Qt::blue);
    dyn->setForeground(Qt::white);
    //dyn->setTextOutline(QPen(Qt::yellow));
    dyn->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
    ranges[1]->setDynamicAttribute(Attribute::ActivateMouseIn, dyn);

    Attribute::Ptr dyn2(new Attribute());
    dyn2->setBackground(Qt::green);
    dyn2->setForeground(Qt::white);
    ranges[1]->setDynamicAttribute(Attribute::ActivateCaretIn, dyn2);

    ranges[3]->setFontUnderline(true);
    ranges[3]->setSelectedForeground(Qt::magenta);
    ranges[4]->setFontStrikeOut(true);
    ranges[5]->setOutline(Qt::blue);
    ranges[5]->setForeground(Qt::white);
  }

  SmartRange* currentRange = mostSpecificChild;
  currentRange->deleteChildRanges();

  Cursor current = currentRange->start();
  QStringList text;

  Range textNeeded = *currentRange;
  if (range != currentRange) {
    if (textNeeded.start() >= textNeeded.end() - Cursor(0,2)) {
      outputRange(range, mostSpecificChild);
      return;
    }

    textNeeded.start() += Cursor(0,1);
    textNeeded.end() -= Cursor(0,1);

    current += Cursor(0,1);
  }

  text = currentRange->document()->textLines(textNeeded);

  foreach (const QString &string, text) {
    for (int i = 0; i < string.length(); ++i) {
      if (string.at(i) == openBrace) {
        currentRange = smart()->newSmartRange(current, currentRange->end(), currentRange);
        connect(currentRange->primaryNotifier(), SIGNAL(mouseEnteredRange(KTextEditor::SmartRange*, KTextEditor::View*)), SLOT(slotMouseEnteredRange(KTextEditor::SmartRange*, KTextEditor::View*)));
        connect(currentRange->primaryNotifier(), SIGNAL(mouseExitedRange(KTextEditor::SmartRange*, KTextEditor::View*)), SLOT(slotMouseExitedRange(KTextEditor::SmartRange*, KTextEditor::View*)));
        connect(currentRange->primaryNotifier(), SIGNAL(caretEnteredRange(KTextEditor::SmartRange*, KTextEditor::View*)), SLOT(slotCaretEnteredRange(KTextEditor::SmartRange*, KTextEditor::View*)));
        connect(currentRange->primaryNotifier(), SIGNAL(caretExitedRange(KTextEditor::SmartRange*, KTextEditor::View*)), SLOT(slotCaretExitedRange(KTextEditor::SmartRange*, KTextEditor::View*)));

        if (currentRange->depth() < 10)
          currentRange->setAttribute(ranges[currentRange->depth()]);

      } else if (string.at(i) == closeBrace && currentRange->parentRange()) {
        currentRange->end() = current + Cursor(0,1);
        currentRange = currentRange->parentRange();
      }
      current.setColumn(current.column() + 1);
    }
    current.setPosition(current.line() + 1, 0);
  }

  //outputRange(range, mostSpecificChild);
}

void ArbitraryHighlightTest::outputRange( KTextEditor::SmartRange * range, KTextEditor::SmartRange * mostSpecific )
{
  kDebug() << (mostSpecific == range ? "==> " : "       ") << QString(range->depth(), ' ') << *range;
  foreach (SmartRange* child, range->childRanges())
    outputRange(child, mostSpecific);
}

void ArbitraryHighlightTest::slotRangeDeleted( KTextEditor::SmartRange * )
{
  m_topRange = 0L;
  QTimer::singleShot(0, this, SLOT(slotCreateTopRange()));
}

void ArbitraryHighlightTest::slotCreateTopRange( )
{
  m_topRange = smart()->newSmartRange(static_cast<Document*>(parent())->documentRange());
  smart()->addHighlightToDocument(m_topRange, true);
  m_topRange->setInsertBehavior(SmartRange::ExpandRight);
  connect(m_topRange->primaryNotifier(), SIGNAL(rangeContentsChanged(KTextEditor::SmartRange*, KTextEditor::SmartRange*)), SLOT(slotRangeChanged(KTextEditor::SmartRange*, KTextEditor::SmartRange*)));
  connect(m_topRange->primaryNotifier(), SIGNAL(rangeDeleted(KTextEditor::SmartRange*)), SLOT(slotRangeDeleted(KTextEditor::SmartRange*)));

  slotRangeChanged(m_topRange, m_topRange);
}

void ArbitraryHighlightTest::slotMouseEnteredRange(KTextEditor::SmartRange* range, KTextEditor::View* view)
{
  Q_UNUSED(view)
  kDebug() << k_funcinfo << *range;
}

void ArbitraryHighlightTest::slotMouseExitedRange(KTextEditor::SmartRange* range, KTextEditor::View* view)
{
  Q_UNUSED(view)
  kDebug() << k_funcinfo << *range;
}

void ArbitraryHighlightTest::slotCaretEnteredRange(KTextEditor::SmartRange* range, KTextEditor::View* view)
{
  Q_UNUSED(view)
  kDebug() << k_funcinfo << *range;
}

void ArbitraryHighlightTest::slotCaretExitedRange(KTextEditor::SmartRange* range, KTextEditor::View* view)
{
  Q_UNUSED(view)
  kDebug() << k_funcinfo << *range;
}

#include "arbitraryhighlighttest.moc"
