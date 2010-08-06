/*  This file is part of the KDE libraries and the Kate part.
 *
 *  Copyright (C) 2003,2004,2005 Hamish Rodda <rodda@kde.org>
 *  Copyright (C) 2010 Christoph Cullmann <cullmann@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "katesmartrange.h"

#include "katedocument.h"
#include "kateedit.h"
#include "kateview.h"
#include <ktexteditor/attribute.h>
#include "katesmartmanager.h"

KateSmartRange::KateSmartRange(const KTextEditor::Range& range, KateDocument* doc, KTextEditor::SmartRange* parent, KTextEditor::SmartRange::InsertBehaviors insertBehavior)
  : KTextEditor::SmartRange(new KateSmartCursor(range.start(), doc), new KateSmartCursor(range.end(), doc), parent, insertBehavior)
  , m_isInternal(false)
{
}

KateSmartRange::KateSmartRange(KateDocument* doc, KTextEditor::SmartRange* parent)
  : KTextEditor::SmartRange(new KateSmartCursor(doc), new KateSmartCursor(doc), parent)
  , m_isInternal(false)
{
}

KateSmartRange::KateSmartRange( KateSmartCursor * start, KateSmartCursor * end, KTextEditor::SmartRange * parent, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
  : KTextEditor::SmartRange(start, end, parent, insertBehavior)
  , m_isInternal(false)
{
}

KateSmartRange::~KateSmartRange()
{
  foreach (KTextEditor::SmartRangeNotifier* n, notifiers()) {
    emit static_cast<KateSmartRangeNotifier*>(n)->rangeDeleted(this);
    // FIXME delete the notifier
  }

  foreach (KTextEditor::SmartRangeWatcher* w, watchers())
    w->rangeDeleted(this);

  if (m_start)
    kateDocument()->smartManager()->rangeDeleted(this);
}

KateDocument * KateSmartRange::kateDocument( ) const
{
  return static_cast<KateDocument*>(document());
}

KateSmartRangeNotifier::KateSmartRangeNotifier(KateSmartRange* owner)
  : m_owner(owner)
{
}

void KateSmartRangeNotifier::connectNotify(const char* signal)
{
  if (receivers(signal) == 1)
    // which signal has been turned on?
    if (QMetaObject::normalizedSignature(SIGNAL(positionChanged(SmartRange*))) == signal)
      m_owner->checkFeedback();
}

void KateSmartRangeNotifier::disconnectNotify(const char* signal)
{
  if (receivers(signal) == 0)
    // which signal has been turned off?
    if (QMetaObject::normalizedSignature(SIGNAL(positionChanged(SmartRange*))) == signal)
      m_owner->checkFeedback();
}

KTextEditor::SmartRangeNotifier* KateSmartRange::createNotifier()
{
  return new KateSmartRangeNotifier(this);
}

void KateSmartRange::shifted( )
{
  if (kStart().lastPosition() != kStart() || kEnd().lastPosition() != kEnd()) {
    // position changed
    foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
      emit static_cast<KateSmartRangeNotifier*>(n)->rangePositionChanged(this);
    foreach (KTextEditor::SmartRangeWatcher* w, watchers())
      w->rangePositionChanged(this);
  }

  kStart().resetLastPosition();
  kEnd().resetLastPosition();
}

void KateSmartRange::translated(const KateEditInfo& edit)
{
  //Why is this assertion triggered sometimes? No clue..
//   Q_ASSERT(end() >= edit.start());
  if (end() < edit.start()) {
    kStart().resetLastPosition();
    kEnd().resetLastPosition();
    return;
  }

  // position changed
  if (kStart().lastPosition() != kStart() || kEnd().lastPosition() != kEnd()) {
    foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
      emit static_cast<KateSmartRangeNotifier*>(n)->rangePositionChanged(this);
    foreach (KTextEditor::SmartRangeWatcher* w, watchers())
      w->rangePositionChanged(this);
  }

  // contents changed
  foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
    emit static_cast<KateSmartRangeNotifier*>(n)->rangeContentsChanged(this);
  foreach (KTextEditor::SmartRangeWatcher* w, watchers())
    w->rangeContentsChanged(this);

  if (start() == end() && kStart().lastPosition() != kEnd().lastPosition()) {
    // range eliminated
    foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
      emit static_cast<KateSmartRangeNotifier*>(n)->rangeEliminated(this);
    foreach (KTextEditor::SmartRangeWatcher* w, watchers())
      w->rangeEliminated(this);
  }

  kStart().resetLastPosition();
  kEnd().resetLastPosition();
}

void KateSmartRange::feedbackRangeContentsChanged( KateSmartRange * mostSpecific )
{
  // most specific range feedback
  foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
    emit static_cast<KateSmartRangeNotifier*>(n)->rangeContentsChanged(this, mostSpecific);
  foreach (KTextEditor::SmartRangeWatcher* w, watchers())
    w->rangeContentsChanged(this, mostSpecific);
}

void KateSmartRange::feedbackMouseEnteredRange(KTextEditor::View* view)
{
      foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
        emit static_cast<KateSmartRangeNotifier*>(n)->mouseEnteredRange(this, view);
      foreach (KTextEditor::SmartRangeWatcher* w, watchers())
        w->mouseEnteredRange(this, view);
}

void KateSmartRange::feedbackMouseExitedRange(KTextEditor::View* view)
{
      foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
        emit static_cast<KateSmartRangeNotifier*>(n)->mouseExitedRange(this, view);
      foreach (KTextEditor::SmartRangeWatcher* w, watchers())
        w->mouseExitedRange(this, view);
}

void KateSmartRange::feedbackCaretEnteredRange(KTextEditor::View* view)
{
      foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
        emit static_cast<KateSmartRangeNotifier*>(n)->caretEnteredRange(this, view);
      foreach (KTextEditor::SmartRangeWatcher* w, watchers())
        w->caretEnteredRange(this, view);
}

void KateSmartRange::feedbackCaretExitedRange(KTextEditor::View* view)
{
      foreach (KTextEditor::SmartRangeNotifier* n, notifiers())
        emit static_cast<KateSmartRangeNotifier*>(n)->caretExitedRange(this, view);
      foreach (KTextEditor::SmartRangeWatcher* w, watchers())
        w->caretExitedRange(this, view);
}

void KateSmartRange::setParentRange( SmartRange * r )
{
  bool gotParent = false;
  bool lostParent = false;
  if (!parentRange() && r)
    gotParent = true;
  else if (parentRange() && !r)
    lostParent = true;

  SmartRange::setParentRange(r);

  if (gotParent)
    kateDocument()->smartManager()->rangeGotParent(this);
  else if (lostParent)
    kateDocument()->smartManager()->rangeLostParent(this);
}

void KateSmartRange::unbindAndDelete( )
{
  kateDocument()->smartManager()->rangeDeleted(this);
  kStart().unbindFromRange();
  kEnd().unbindFromRange();
  m_start = 0L;
  m_end = 0L;
  delete this;
}

void KateSmartRange::setInternal( )
{
  m_isInternal = true;
  kStart().setInternal();
  kEnd().setInternal();
}

void KateSmartRange::checkFeedback()
{
  kStart().checkFeedback();
  kEnd().checkFeedback();
}

// kate: space-indent on; indent-width 2; replace-tabs on;

#include "katesmartrange.moc"
