/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

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
#ifndef KLISTVIEWLINEEDIT_H
#define KLISTVIEWLINEEDIT_H


#include <klineedit.h>
#include <k3listview.h>

/**
 * the editor for a K3ListView.  please don't use this.
 * @internal
 **/
class K3ListViewLineEdit : public KLineEdit
{
Q_OBJECT
public:
	K3ListViewLineEdit(K3ListView *parent);
	~K3ListViewLineEdit();

	Q3ListViewItem *currentItem() const;

Q_SIGNALS:
	void done(Q3ListViewItem*, int);

public Q_SLOTS:
	void terminate();
	void load(Q3ListViewItem *i, int c);

protected:
	virtual void focusOutEvent(QFocusEvent *);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void paintEvent(QPaintEvent *e);
	virtual bool event (QEvent *pe);
	void selectNextCell (Q3ListViewItem *pi, int column, bool forward);
	void terminate(bool commit);
	Q3ListViewItem *item;
	int col;
	K3ListView* const p;

protected Q_SLOTS:
	void slotSelectionChanged();

};

#endif
