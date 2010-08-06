 /* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001-2010 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef kate_view_h
#define kate_view_h

#include <ktexteditor/view.h>
#include <ktexteditor/texthintinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/sessionconfiginterface.h>
#include <ktexteditor/templateinterface.h>
#include <ktexteditor/templateinterface2.h>
#include <ktexteditor/smartrangewatcher.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/annotationinterface.h>

#include <QtCore/QPointer>
#include <QModelIndex>
#include <QtGui/QMenu>
#include <QtCore/QLinkedList>
#include <QtCore/QHash>

#include <kdebug.h>

#include "kateviinputmodemanager.h"
#include "katetextrange.h"

namespace KTextEditor
{
  class AnnotationModel;
}

class KateDocument;
class KateBookmarks;
class KateCommandLineBar;
class KateViewConfig;
class KateRenderer;
class KateSpellCheckDialog;
class KateCompletionWidget;
class KateSmartRange;
class KateViewInternal;
class KateSearchBar;
class KateViModeBar;
class KateViewBar;
class KateGotoBar;
class KateDictionaryBar;
class KateSpellingMenu;

class KToggleAction;
class KAction;
class KRecentFilesAction;
class KSelectAction;

class QVBoxLayout;

//
// Kate KTextEditor::View class ;)
//
class KATEPART_TESTS_EXPORT KateView : public KTextEditor::View,
                 public KTextEditor::TextHintInterface,
                 public KTextEditor::SessionConfigInterface,
                 public KTextEditor::TemplateInterface2,
                 public KTextEditor::CodeCompletionInterface,
                 public KTextEditor::ConfigInterface,
                 private KTextEditor::SmartRangeWatcher,
                 public KTextEditor::AnnotationViewInterface,
                 public KTextEditor::CoordinatesToCursorInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::TextHintInterface)
    Q_INTERFACES(KTextEditor::SessionConfigInterface)
    Q_INTERFACES(KTextEditor::TemplateInterface)
    Q_INTERFACES(KTextEditor::TemplateInterface2)
    Q_INTERFACES(KTextEditor::ConfigInterface)
    Q_INTERFACES(KTextEditor::CodeCompletionInterface)
    Q_INTERFACES(KTextEditor::AnnotationViewInterface)
    Q_INTERFACES(KTextEditor::CoordinatesToCursorInterface)

    friend class KateViewInternal;
    friend class KateIconBorder;
    friend class KateViModeBase;

  public:
    KateView( KateDocument* doc, QWidget* parent );
    ~KateView ();

    KTextEditor::Document *document () const;

    QString viewMode () const;

  //
  // KTextEditor::ClipboardInterface
  //
  public Q_SLOTS:
    // TODO: Factor out of m_viewInternal
    void paste();
    void cut();
    void copy() const;

  private Q_SLOTS:
    /**
     * internal use, apply word wrap
     */
    void applyWordWrap ();

  //
  // KTextEditor::PopupMenuInterface
  //
  public:
    void setContextMenu( QMenu* menu );
    QMenu* contextMenu() const;
    QMenu* defaultContextMenu(QMenu* menu = 0L) const;

  private Q_SLOTS:
    void aboutToShowContextMenu();
    void aboutToHideContextMenu();

  private:
    QPointer<QMenu> m_contextMenu;

  //
  // KTextEditor::ViewCursorInterface
  //
  public:
    bool setCursorPosition (KTextEditor::Cursor position);

    KTextEditor::Cursor cursorPosition () const;

    KTextEditor::Cursor cursorPositionVirtual () const;

    QPoint cursorToCoordinate(const KTextEditor::Cursor& cursor) const;

    KTextEditor::Cursor coordinatesToCursor(const QPoint& coord) const;

    QPoint cursorPositionCoordinates() const;

    bool setCursorPositionVisual( const KTextEditor::Cursor& position );

    /**
     * Return the virtual cursor column, each tab is expanded into the
     * document's tabWidth characters. If word wrap is off, the cursor may be
     * behind the line's length.
     */
    int virtualCursorColumn() const;

    virtual bool mouseTrackingEnabled() const;
    virtual bool setMouseTrackingEnabled(bool enable);

  private:
    void notifyMousePositionChanged(const KTextEditor::Cursor& newPosition);

  // Internal
  public:
    bool setCursorPositionInternal( const KTextEditor::Cursor& position, uint tabwidth = 1, bool calledExternally = false );

  //
  // KTextEditor::ConfigInterface
  //
  public:
     QStringList configKeys() const;
     QVariant configValue(const QString &key);
     void setConfigValue(const QString &key, const QVariant &value);

  Q_SIGNALS:
    void configChanged();

  //
  // KTextEditor::CodeCompletionInterface2
  //
  public:
    virtual bool isCompletionActive() const;
    virtual void startCompletion(const KTextEditor::Range& word, KTextEditor::CodeCompletionModel* model);
    virtual void abortCompletion();
    virtual void forceCompletion();
    virtual void registerCompletionModel(KTextEditor::CodeCompletionModel* model);
    virtual void unregisterCompletionModel(KTextEditor::CodeCompletionModel* model);
    virtual bool isAutomaticInvocationEnabled() const;
    virtual void setAutomaticInvocationEnabled(bool enabled = true);

  Q_SIGNALS:
    void completionExecuted(KTextEditor::View* view, const KTextEditor::Cursor& position, KTextEditor::CodeCompletionModel* model, const QModelIndex&);
    void completionAborted(KTextEditor::View* view);

  public Q_SLOTS:
    void userInvokedCompletion();

  public:
    KateCompletionWidget* completionWidget() const;
    mutable KateCompletionWidget* m_completionWidget;
    void sendCompletionExecuted(const KTextEditor::Cursor& position, KTextEditor::CodeCompletionModel* model, const QModelIndex& index);
    void sendCompletionAborted();

  //
  // KTextEditor::TextHintInterface
  //
  public:
    void enableTextHints(int timeout);
    void disableTextHints();

  Q_SIGNALS:
    void needTextHint(const KTextEditor::Cursor& position, QString &text);

  public:
    bool dynWordWrap() const      { return m_hasWrap; }

  //
  // KTextEditor::SelectionInterface stuff
  //
  public Q_SLOTS:
    virtual bool setSelection ( const KTextEditor::Range &selection );

    // unhide method...
    bool setSelection (const KTextEditor::Cursor &c, int i, bool b)
    { return KTextEditor::View::setSelection (c, i, b); }

    virtual bool removeSelection () { return clearSelection(); }

    virtual bool removeSelectionText () { return removeSelectedText(); }

    virtual bool setBlockSelection (bool on) { return setBlockSelectionMode (on); }

    bool clearSelection ();
    bool clearSelection (bool redraw, bool finishedChangingSelection = true);

    bool removeSelectedText ();

    bool selectAll();

  public:
    virtual bool selection() const;
    virtual QString selectionText() const;
    virtual bool blockSelection() const { return blockSelectionMode(); }
    virtual const KTextEditor::Range &selectionRange() const;

    static void blockFix(KTextEditor::Range& range);

  private:
    mutable KTextEditor::Range m_holdSelectionRangeForAPI;

  //
  // Arbitrary Syntax HL + Action extensions
  //
  public:
    // Syntax highlighting extension
    void addExternalHighlight(KTextEditor::SmartRange* topRange, bool supportDynamic);
    const QList<KTextEditor::SmartRange*>& externalHighlights() const;
    void clearExternalHighlights();

    // Action association extension
    void deactivateEditActions();
    void activateEditActions();

  public Q_SLOTS:
    void removeExternalHighlight(KTextEditor::SmartRange* topRange);

  private:
    // Smart range watcher overrides
    virtual void rangeDeleted(KTextEditor::SmartRange* range);

    QList<KTextEditor::SmartRange*> m_externalHighlights;

  //
  // internal helper stuff, for katerenderer and so on
  //
  public:
    // should cursor be wrapped ? take config + blockselection state in account
    bool wrapCursor () const;

    // some internal functions to get selection state of a line/col
    bool cursorSelected(const KTextEditor::Cursor& cursor);
    bool lineSelected (int line);
    bool lineEndSelected (const KTextEditor::Cursor& lineEndPos);
    bool lineHasSelected (int line);
    bool lineIsSelection (int line);

    void ensureCursorColumnValid();

    void tagSelection (const KTextEditor::Range &oldSelection);

    void selectWord(   const KTextEditor::Cursor& cursor );
    void selectLine(   const KTextEditor::Cursor& cursor );

  //
  // KTextEditor::BlockSelectionInterface stuff
  //
  public Q_SLOTS:
    bool setBlockSelectionMode (bool on);
    bool toggleBlockSelectionMode ();

  public:
    bool blockSelectionMode() const;


  //BEGIN EDIT STUFF
  public:
    void editStart ();
    void editEnd (int editTagLineStart, int editTagLineEnd, bool tagFrom);

    void editSetCursor (const KTextEditor::Cursor &cursor);
  //END

  //BEGIN TAG & CLEAR
  public:
    bool tagLine (const KTextEditor::Cursor& virtualCursor);

    bool tagRange (const KTextEditor::Range& range, bool realLines = false);
    bool tagLines (int start, int end, bool realLines = false );
    bool tagLines (KTextEditor::Cursor start, KTextEditor::Cursor end, bool realCursors = false);
    bool tagLines (KTextEditor::Range range, bool realRange = false);

    void tagAll ();

    void relayoutRange(const KTextEditor::Range& range, bool realLines = false);

    void clear ();

    void repaintText (bool paintOnlyDirty = false);

    void updateView (bool changed = false);
  //END

  //
  // KTextEditor::AnnotationView
  //
  public:
    void setAnnotationModel( KTextEditor::AnnotationModel* model );
    KTextEditor::AnnotationModel* annotationModel() const;
    void setAnnotationBorderVisible( bool visible);
    bool isAnnotationBorderVisible() const;

  Q_SIGNALS:
    void annotationContextMenuAboutToShow( KTextEditor::View* view, QMenu* menu, int line );
    void annotationActivated( KTextEditor::View* view, int line );
    void annotationBorderVisibilityChanged( View* view, bool visible );

    void navigateLeft();
    void navigateRight();
    void navigateUp();
    void navigateDown();
    void navigateAccept();
    void navigateBack();

  private:
    KTextEditor::AnnotationModel* m_annotationModel;

  //
  // KTextEditor::View
  //
  public:
    void emitNavigateLeft() {
      emit navigateLeft();
    }
    void emitNavigateRight() {
      emit navigateRight();
    }
    void emitNavigateUp() {
      emit navigateUp();
    }
    void emitNavigateDown() {
      emit navigateDown();
    }
    void emitNavigateAccept() {
      emit navigateAccept();
    }
    void emitNavigateBack() {
      emit navigateBack();
    }
    /**
     Return values for "save" related commands.
    */
    bool isOverwriteMode() const;
    enum KTextEditor::View::EditMode viewEditMode() const {return isOverwriteMode() ? KTextEditor::View::EditOverwrite : KTextEditor::View::EditInsert;}
    QString currentTextLine();

  public Q_SLOTS:
    void indent();
    void unIndent();
    void cleanIndent();
    void align();
    void comment();
    void uncomment();
    void toggleComment();
    void killLine();

    /**
      Uppercases selected text, or an alphabetic character next to the cursor.
    */
    void uppercase();
    /**
      Lowercases selected text, or an alphabetic character next to the cursor.
    */
    void lowercase();
    /**
      Capitalizes the selection (makes each word start with an uppercase) or
      the word under the cursor.
    */
    void capitalize();
    /**
      Joins lines touched by the selection
    */
    void joinLines();

    // Note - the following functions simply forward to KateViewInternal
    void keyReturn();
    void smartNewline();
    void backspace();
    void deleteWordLeft();
    void keyDelete();
    void deleteWordRight();
    void transpose();
    void cursorLeft();
    void shiftCursorLeft();
    void cursorRight();
    void shiftCursorRight();
    void wordLeft();
    void shiftWordLeft();
    void wordRight();
    void shiftWordRight();
    void home();
    void shiftHome();
    void end();
    void shiftEnd();
    void up();
    void shiftUp();
    void down();
    void shiftDown();
    void scrollUp();
    void scrollDown();
    void topOfView();
    void shiftTopOfView();
    void bottomOfView();
    void shiftBottomOfView();
    void pageUp();
    void shiftPageUp();
    void pageDown();
    void shiftPageDown();
    void top();
    void shiftTop();
    void bottom();
    void shiftBottom();
    void toMatchingBracket();
    void shiftToMatchingBracket();

    void gotoLine();

  // config file / session management functions
  public:
    void readSessionConfig(const KConfigGroup&);
    void writeSessionConfig(KConfigGroup&);

  public Q_SLOTS:
    void setEol( int eol );
    void setAddBom( bool enabled);
    void find();
    void findSelectedForwards();
    void findSelectedBackwards();
    void replace();
    void findNext();
    void findPrevious();

    void setFoldingMarkersOn( bool enable ); // Not in KTextEditor::View, but should be
    void setIconBorder( bool enable );
    void setLineNumbersOn( bool enable );
    void setScrollBarMarks( bool enable );
    void toggleFoldingMarkers();
    void toggleIconBorder();
    void toggleLineNumbersOn();
    void toggleScrollBarMarks();
    void toggleDynWordWrap ();
    void toggleViInputMode ();
    void showViModeBar ();
    void hideViModeBar ();
    void setDynWrapIndicators(int mode);

  public:
    int getEol() const;

  public:
    KateRenderer *renderer ();

    bool iconBorder();
    bool lineNumbersOn();
    bool scrollBarMarks();
    int dynWrapIndicators();
    bool foldingMarkersOn();

  private Q_SLOTS:
    /**
     * used to update actions after selection changed
     */
    void slotSelectionChanged ();

  public:
    /**
     * accessor to katedocument pointer
     * @return pointer to document
     */
    KateDocument*  doc() { return m_doc; }
    const KateDocument*  doc() const { return m_doc; }

  public Q_SLOTS:
    void slotUpdateUndo();
    void toggleInsert();
    void reloadFile();
    void toggleWWMarker();
    void toggleWriteLock();
    void switchToCmdLine ();
    void slotReadWriteChanged ();

  Q_SIGNALS:
    void dropEventPass(QDropEvent*);

  public:
    void slotTextInserted ( KTextEditor::View *view, const KTextEditor::Cursor &position, const QString &text);

  protected:
    void contextMenuEvent( QContextMenuEvent* );

  private Q_SLOTS:
    void slotGotFocus();
    void slotLostFocus();
    void slotDropEventPass( QDropEvent* ev );
    void slotSaveCanceled( const QString& error );
    void slotExpandToplevel();
    void slotCollapseLocal();
    void slotExpandLocal();
    void slotConfigDialog ();

  private:
    void setupConnections();
    void setupActions();
    void setupEditActions();
    void setupCodeFolding();

    QList<QAction*>        m_editActions;
    KAction*               m_editUndo;
    KAction*               m_editRedo;
    KRecentFilesAction*    m_fileRecent;
    KToggleAction*         m_toggleFoldingMarkers;
    KToggleAction*         m_toggleIconBar;
    KToggleAction*         m_toggleLineNumbers;
    KToggleAction*         m_toggleScrollBarMarks;
    KToggleAction*         m_toggleDynWrap;
    KSelectAction*         m_setDynWrapIndicators;
    KToggleAction*         m_toggleWWMarker;
    KAction*               m_switchCmdLine;
    KToggleAction*         m_viInputModeAction;

    KSelectAction*         m_setEndOfLine;
    KToggleAction*         m_addBom;

    QAction *m_cut;
    QAction *m_copy;
    QAction *m_paste;
    QAction *m_selectAll;
    QAction *m_deSelect;

    KToggleAction *m_toggleBlockSelection;
    KToggleAction *m_toggleInsert;
    KToggleAction *m_toggleWriteLock;

    bool m_hasWrap;

    KateDocument     *const m_doc;
    KateViewConfig   *const m_config;
    KateRenderer     *const m_renderer;
    KateViewInternal *const m_viewInternal;
    KateSpellCheckDialog  *m_spell;
    KateBookmarks    *const m_bookmarks;

    QVBoxLayout *m_vBox;

  private Q_SLOTS:
    void slotHlChanged();

  /**
   * Configuration
   */
  public:
    inline KateViewConfig *config () { return m_config; }

    void updateConfig ();

    void updateDocumentConfig();

    void updateRendererConfig();

  private Q_SLOTS:
    void updateFoldingConfig ();

  private:
    bool m_startingUp;
    bool m_updatingDocumentConfig;

    // stores the current selection
    Kate::TextRange m_selection;

    // do we select normal or blockwise ?
    bool blockSelect;

  //
  // TemplateInterface + TemplateInterface2
  //
  public:
    virtual bool insertTemplateTextImplementation ( const KTextEditor::Cursor&, const QString &templateString, const QMap<QString,QString> &initialValues);
    virtual bool insertTemplateTextImplementation ( const KTextEditor::Cursor&, const QString &templateString, const QMap<QString,QString> &initialValues, KTextEditor::TemplateScript* templateScript);
  /**
   * Accessors to the bars...
   */
  public:
    KateViewBar *viewBar() const;
    KateCommandLineBar *cmdLineBar ();
    KateDictionaryBar *dictionaryBar();

  private:
    KateSearchBar *searchBar (bool initHintAsPower = false);
    bool hasSearchBar () const { return m_searchBar != 0; }
    KateViModeBar *viModeBar();
    KateGotoBar *gotoBar ();

  /**
   * viewbar + its widgets
   * they are created on demand...
   */
  private:
    // created in constructor of the view
    KateViewBar *m_bottomViewBar;
    KateViewBar *m_topViewBar;
    // created on demand..., only access them through the above accessors....
    KateCommandLineBar *m_cmdLine;
    KateSearchBar *m_searchBar;
    KateViModeBar *m_viModeBar;
    KateGotoBar *m_gotoBar;
    KateDictionaryBar *m_dictionaryBar;

  // vi Mode
  public:
    /**
     * @return boolean indicating whether vi mode is active or not
     */
    bool viInputMode() const;

    /**
     * @return the current vi mode
     */
    ViMode getCurrentViMode() const;

    /**
     * @return a pointer to the KateViInputModeManager belonging to the view
     */
    KateViInputModeManager* getViInputModeManager();

    /**
     * @return boolean indicating whether vi mode will override actions or not
     */
    bool viInputModeStealKeys() const;

    /**
     * Update vi mode statusbar according to the current mode
     */
    void updateViModeBarMode();

    /**
     * Update vi mode statusbar with the (partial) vi command being typed
     */
    void updateViModeBarCmd();

  public:
    KTextEditor::Range visibleRange();

  Q_SIGNALS:
    void displayRangeChanged(KateView *view);

  protected:
    KToggleAction*               m_toggleOnTheFlySpellCheck;
    KateSpellingMenu *m_spellingMenu;

  protected Q_SLOTS:
    void toggleOnTheFlySpellCheck(bool b);

  public Q_SLOTS:
    void changeDictionary();
    void reflectOnTheFlySpellCheckStatus(bool enabled);

  public:
    KateSpellingMenu* spellingMenu();
  private:
    bool m_userContextMenuSet;

public:
    /**
     * Attribute of a range changed or range with attribute changed in given line range.
     * @param startLine start line of change
     * @param endLine end line of change
     * @param rangeWithAttribute attribute changed or is active, this will perhaps lead to repaints
     */
    void notifyAboutRangeChange (int startLine, int endLine, bool rangeWithAttribute);

  private Q_SLOTS:
    /**
     * Delayed update for view after text ranges changed
     */
    void slotDelayedUpdateOfView ();

  Q_SIGNALS:
    /**
     * Delayed update for view after text ranges changed
     */
    void delayedUpdateOfView ();

  public:
      /**
       * set of ranges which had the mouse inside last time, used for rendering
       * @return set of ranges which had the mouse inside last time checked
       */
      const QSet<Kate::TextRange *> *rangesMouseIn () const { return &m_rangesMouseIn; }

      /**
       * set of ranges which had the caret inside last time, used for rendering
       * @return set of ranges which had the caret inside last time checked
       */
      const QSet<Kate::TextRange *> *rangesCaretIn () const { return &m_rangesCaretIn; }

      /**
       * check if ranges changed for mouse in and caret in
       * @param activationType type of activation to check
       */
      void updateRangesIn (KTextEditor::Attribute::ActivationType activationType);

  //
  // helpers for delayed view update after ranges changes
  //
  private:
      /**
       * update already inited?
       */
      int m_delayedUpdateTriggered;

      /**
       * minimal line to update
       */
      int m_lineToUpdateMin;

      /**
       * maximal line to update
       */
      int m_lineToUpdateMax;

      /**
       * set of ranges which had the mouse inside last time
       */
      QSet<Kate::TextRange *> m_rangesMouseIn;

      /**
       * set of ranges which had the caret inside last time
       */
      QSet<Kate::TextRange *> m_rangesCaretIn;
};

/**
 * metatype register
 */
Q_DECLARE_METATYPE(KTextEditor::Cursor)
Q_DECLARE_METATYPE(KTextEditor::Range)

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
