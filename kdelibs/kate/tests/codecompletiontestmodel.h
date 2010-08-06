/* This file is part of the KDE project
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

#ifndef CODECOMPLETIONTEST_H
#define CODECOMPLETIONTEST_H

#include <ktexteditor/codecompletionmodel.h>

namespace KTextEditor {
  class View;
  class CodeCompletionInterface;
}

class CodeCompletionTestModel : public KTextEditor::CodeCompletionModel
{
  Q_OBJECT

  public:
    CodeCompletionTestModel(KTextEditor::View* parent = 0L, const QString &startText = QString());

    KTextEditor::View* view() const;
    KTextEditor::CodeCompletionInterface* cc() const;

    virtual void completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  private:
    QString m_startText;
    bool m_autoStartText;
};

#endif
