/* This file is part of the KDE libraries
   Copyright (C) 2002 Joseph Wenninger <jowenn@jowenn.at> and Daniel Naber <daniel.naber@t-online.de>

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

//BEGIN includes
#include "kate_kdatatool.h"
#include "kate_kdatatool.moc"
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <ktexteditor/view.h>
#include <kdebug.h>
#include <kdatatool.h>
#include <ktexteditor/document.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kactionmenu.h>
#include <klocale.h>
//END includes

K_PLUGIN_FACTORY( KDataToolPluginFactory, registerPlugin<KTextEditor::KDataToolPlugin>(); )
K_EXPORT_PLUGIN( KDataToolPluginFactory( KAboutData("ktexteditor_kdatatool", "ktexteditor_plugins", ki18n("DataTool"), "0.1", ki18n("Data tool"), KAboutData::License_LGPL_V2) ) )

namespace KTextEditor {

KDataToolPlugin::KDataToolPlugin( QObject *parent, const QVariantList& )
	: KTextEditor::Plugin ( parent )
{
}


KDataToolPlugin::~KDataToolPlugin ()
{
}

void KDataToolPlugin::addView(KTextEditor::View *view)
{
	KDataToolPluginView *nview = new KDataToolPluginView (view);
	nview->setView (view);
	m_views.append (nview);
}

void KDataToolPlugin::removeView(KTextEditor::View *view)
{
	for (int z=0; z < m_views.count(); z++)
        {
		if (m_views.at(z)->parentClient() == view)
		{
			KDataToolPluginView *nview = m_views.at(z);
			m_views.removeAll (nview);
			delete nview;
		}
	}
}


KDataToolPluginView::KDataToolPluginView( KTextEditor::View *view )
	:QObject(view),KXMLGUIClient(view),m_menu(0),m_notAvailable(0)
{
	setComponentData( KDataToolPluginFactory::componentData() );
	setXMLFile("ktexteditor_kdatatoolui.rc");
	
	m_menu = new KActionMenu(i18n("Data Tools"), this);
        actionCollection()->addAction("popup_dataTool", m_menu);
	connect(m_menu->menu(), SIGNAL(aboutToShow()), this, SLOT(aboutToShow()));
	
	m_view = view;
}

KDataToolPluginView::~KDataToolPluginView()
{
        m_view->removeChildClient (this);
	delete m_menu;
}

void KDataToolPluginView::aboutToShow()
{
	kDebug( 13040 )<<"KTextEditor::KDataToolPluginView::aboutToShow";
	QString word;
	m_singleWord = false;
	m_wordUnderCursor.clear();

	// unplug old actions, if any:
	foreach (QAction *ac, m_actionList) {
		m_menu->removeAction(ac);
	}
	if (m_notAvailable) {
		m_menu->removeAction(m_notAvailable);
		delete m_notAvailable;
		m_notAvailable=0;
	}
	if ( m_view->selection() )
	{
		word = m_view->selectionText();
		if ( word.indexOf(' ') == -1 && word.indexOf('\t') == -1 && word.indexOf('\n') == -1 )
			m_singleWord = true;
		else
			m_singleWord = false;
	} else {
		// No selection -> use word under cursor
		KTextEditor::View *v = (KTextEditor::View*)m_view;
		int line, col;
		line = v->cursorPosition().line();
		col = v->cursorPosition().column();
		QString tmp_line = v->document()->line(line);
		m_wordUnderCursor = "";
		// find begin of word:
		m_singleWord_start = 0;
		for(int i = qMin(col,tmp_line.length()-1); i >= 0; i--) {
			QChar ch = tmp_line.at(i);
			if( ! (ch.isLetter() || ch == '-' || ch == '\'') )
			{
				m_singleWord_start = i+1;
				break;
			}
			m_wordUnderCursor = ch + m_wordUnderCursor;
		}
		// find end of word:
		m_singleWord_end = tmp_line.length();
		for(int i = col+1; i < tmp_line.length(); i++) {
			QChar ch = tmp_line.at(i);
			if( ! (ch.isLetter() || ch == '-' || ch == '\'') )
			{
				m_singleWord_end = i;
				break;
			}
			m_wordUnderCursor += ch;
		}
		if( ! m_wordUnderCursor.isEmpty() )
		{
			m_singleWord = true;
			m_singleWord_line = line;
		} else {
                        m_notAvailable = new KAction(i18n("(not available)"), this );
                        actionCollection()->addAction("dt_n_av", m_notAvailable);
                        connect( m_notAvailable, SIGNAL( triggered( bool ) ), this, SLOT(slotNotAvailable()) );
			m_menu->addAction(m_notAvailable);
			return;
		}
	}

	KComponentData inst=componentData();

	QList<KDataToolInfo> tools;
	tools += KDataToolInfo::query( "QString", "text/plain", inst );
	if( m_singleWord )
		tools += KDataToolInfo::query( "QString", "application/x-singleword", inst );

	m_actionList = KDataToolAction::dataToolActionList( tools, this,
		SLOT( slotToolActivated( const KDataToolInfo &, const QString & ) ),
                                                            actionCollection());

	foreach (QAction* ac, m_actionList)
		m_menu->addAction(ac);

	if( m_actionList.isEmpty() ) {
                m_notAvailable = new KAction(i18n("(not available)"), this);
                actionCollection()->addAction("dt_n_av", m_notAvailable);
                connect( m_notAvailable, SIGNAL( triggered( bool ) ), this, SLOT(slotNotAvailable()) );
		m_menu->addAction(m_notAvailable);
	}
}

void KDataToolPluginView::slotNotAvailable()
{
	KMessageBox::sorry(0, i18n("Data tools are only available when text is selected, "
		"or when the right mouse button is clicked over a word. If no data tools are offered "
		"even when text is selected, you need to install them. Some data tools are part "
		"of the KOffice package."));
}

void KDataToolPluginView::slotToolActivated( const KDataToolInfo &info, const QString &command )
{

	KDataTool* tool = info.createTool( );
	if ( !tool )
	{
		kWarning() << "Could not create Tool !";
		return;
	}

	QString text;
	if ( m_view->selection() )
		text = m_view->selectionText();
	else
		text = m_wordUnderCursor;

	QString mimetype = "text/plain";
	QString datatype = "QString";

	// If unsupported (and if we have a single word indeed), try application/x-singleword
	if ( !info.mimeTypes().contains( mimetype ) && m_singleWord )
		mimetype = "application/x-singleword";

	kDebug( 13040 ) << "Running tool with datatype=" << datatype << " mimetype=" << mimetype;

	QString origText = text;

	if ( tool->run( command, &text, datatype, mimetype) )
	{
		kDebug( 13040 ) << "Tool ran. Text is now " << text;
		if ( origText != text )
		{
			int line, col;
			line = m_view->cursorPosition().line();
      col = m_view->cursorPosition().column();
			if ( !m_view->selection() )
			{
				m_view->setSelection(KTextEditor::Range(m_singleWord_line, m_singleWord_start, m_singleWord_line, m_singleWord_end));
			}

			// replace selection with 'text'
			m_view->removeSelectionText();
			m_view->document()->insertText(m_view->cursorPosition(), text);
			 // fixme: place cursor at the end:
			 /* No idea yet (Joseph Wenninger)
			 for ( uint i = 0; i < text.length(); i++ ) {
				viewCursorInterface(m_view)->cursorRight();
			 } */
		}
	}

	delete tool;
}


}
