#pragma once
#include <stackjunkie/theme.hpp>
#include <stackjunkie/presets.hpp>
#include <QFrame>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolButton>
#include <QTimer>
#include <QTextBrowser>
#include <QDialog>
#include <QTreeWidget>
#include <QKeySequence>
#include <QHash>
#include <functional>
class QVBoxLayout;
class QButtonGroup;
class QListWidget;
class QSplitterHandle;
class QTabBar;
class QMenuBar;
class QShortcut;
class QMenu;
class QAction;
class QHBoxLayout;
#include <QTabWidget>
class QGraphicsScene;
class QGraphicsView;
class QComboBox;
class QDoubleSpinBox;

namespace StackJunkie {
class STACKJUNKIE_EXPORT ReadingView : public QTextBrowser {
public:
    explicit ReadingView(const QString &html = {}, QWidget *parent = nullptr);
    void setContentHtml(const QString &html);
    void applyTheme(const Theme &theme);
private:
    QString sourceHtml_;
};
class STACKJUNKIE_EXPORT SplitPane : public QSplitter {
public:
    explicit SplitPane(QWidget *parent = nullptr);
    explicit SplitPane(Qt::Orientation orientation, QWidget *parent = nullptr);
    void resizeFromKeyboard(QSplitterHandle *handle, int key, Qt::KeyboardModifiers modifiers);
    void setStackBelowWidth(int width);
protected:
    QSplitterHandle *createHandle() override;
    void resizeEvent(QResizeEvent *event) override;
private:
    int stackBelowWidth_ = 0;
};
enum class ButtonVariant { Primary, Secondary, Quiet, Danger };
class STACKJUNKIE_EXPORT Button : public QPushButton {
public:
    explicit Button(const QString &text, ButtonVariant variant = ButtonVariant::Secondary, QWidget *parent = nullptr);
    void setBusy(bool busy, const QString &busyLabel = QStringLiteral("Working..."));
private:
    QString normalLabel_;
};
class STACKJUNKIE_EXPORT IconButton : public QToolButton {
public:
    explicit IconButton(const QString &iconName, const QString &accessibleName, QWidget *parent = nullptr);
};
class STACKJUNKIE_EXPORT Field : public QWidget {
public:
    explicit Field(const QString &label, QWidget *input, const QString &help = {}, QWidget *parent = nullptr);
    void setError(const QString &message);
    QWidget *input() const { return input_; }
private:
    QWidget *input_;
    QLabel *error_;
};
class STACKJUNKIE_EXPORT SearchField : public QLineEdit {
public:
    explicit SearchField(const QString &label = QStringLiteral("Search"), QWidget *parent = nullptr);
};
class STACKJUNKIE_EXPORT SegmentedControl : public QWidget {
    Q_OBJECT
public:
    explicit SegmentedControl(const QStringList &options, QWidget *parent = nullptr);
    int currentIndex() const;
    void setCurrentIndex(int index);
signals:
    void currentChanged(int index);
private:
    QButtonGroup *group_;
};
class STACKJUNKIE_EXPORT HelpButton : public QToolButton {
public:
    explicit HelpButton(const QString &name, const QString &text, QWidget *parent = nullptr);
    ~HelpButton() override;
    bool isPinned() const { return pinned_; }
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
private:
    void showHelp(bool pin);
    void dismiss();
    bool pinned_ = false;
    bool dismissed_ = false;
    QFrame *popup_;
    QTimer closeTimer_;
};
class STACKJUNKIE_EXPORT Disclosure : public QFrame {
public:
    explicit Disclosure(const QString &label, QWidget *content, bool expanded = false, QWidget *parent = nullptr);
    bool isExpanded() const;
    void setExpanded(bool expanded);
private:
    QToolButton *toggle_;
    QWidget *content_;
};
class STACKJUNKIE_EXPORT Notice : public QFrame {
public:
    explicit Notice(const QString &text, const QString &tone = QStringLiteral("info"), QWidget *parent = nullptr);
    void setText(const QString &text);
private:
    QLabel *text_;
};
class STACKJUNKIE_EXPORT Tag : public QLabel {
public:
    explicit Tag(const QString &text, const QString &tone = QStringLiteral("neutral"), QWidget *parent = nullptr);
};
class STACKJUNKIE_EXPORT EmptyState : public QWidget {
public:
    explicit EmptyState(const QString &title, const QString &description, QWidget *action = nullptr, QWidget *parent = nullptr);
};
class STACKJUNKIE_EXPORT CodeBlock : public QFrame {
public:
    explicit CodeBlock(const QString &text, QWidget *parent = nullptr);
};
// One row of tabs. A tab row must never sit inside another tab page: debug builds warn when it does.
// Use the sidebar tree, a toolbar toggle, or a TabBoundary frame instead.
class STACKJUNKIE_EXPORT Tabs : public QTabWidget {
public:
    explicit Tabs(QWidget *parent = nullptr);
    /** Open-document tabs: closing is immediate, with no confirmation. Control W closes the current tab. */
    void setDocumentTabs(bool closable);
protected:
    void showEvent(QShowEvent *event) override;
    void tabInserted(int index) override;
};
// Marks a deliberate visual break, such as a framed specimen or an embedded app window, so tab rows inside it are allowed.
class STACKJUNKIE_EXPORT TabBoundary : public QFrame {
public:
    explicit TabBoundary(QWidget *content, QWidget *parent = nullptr);
};
// Tab groups in a single row, like browser tab groups: a group chip sits in the row and collapses its tabs.
class STACKJUNKIE_EXPORT GroupedTabs : public Tabs {
public:
    explicit GroupedTabs(QWidget *parent = nullptr);
    int addGroup(const QString &name);
    int addTab(int group, const QString &title, QWidget *page);
    void setGroupCollapsed(int group, bool collapsed);
private:
    struct Group { QString name; bool collapsed = false; QList<QPair<QString, QWidget *>> tabs; };
    void rebuild();
    QList<Group> groups_;
    bool rebuilding_ = false;
};
// Collapsible groups of pages, VS Code style. This is where depth goes in apps with many pages.
class STACKJUNKIE_EXPORT NavTree : public QTreeWidget {
    Q_OBJECT
public:
    explicit NavTree(const QString &label, QWidget *parent = nullptr);
    void addGroup(const QString &id, const QString &label, const QList<QPair<QString, QString>> &items);
    void setCurrentId(const QString &id);
signals:
    void pageChosen(const QString &id);
};
// Settings open as a window over the app, never as a page. Section list and search on the left;
// changes apply immediately, so the only action is closing the window.
class STACKJUNKIE_EXPORT SettingsWindow : public QDialog {
public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    void addSection(const QString &label, QWidget *content, const QString &keywords = {});
    void showSection(int index);
private:
    void filter(const QString &text);
    QLineEdit *search_;
    QListWidget *list_;
    QStackedWidget *pages_;
    QStringList keywords_;
};
class STACKJUNKIE_EXPORT Drawer : public QFrame {
public:
    enum class Edge { Left, Right };
    explicit Drawer(QWidget *host, const QString &title, QWidget *content, Edge edge = Edge::Right);
    ~Drawer() override;
    void open();
    void dismiss();
    void setEdge(Edge edge);
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
private:
    void place();
    Edge edge_;
    QPointer<QWidget> previousFocus_;
};
class STACKJUNKIE_EXPORT Chart : public QWidget {
public:
    enum class Mode { Bars, Line };
    explicit Chart(QWidget *parent = nullptr);
    void setData(const QList<QPair<QString, double>> &values, const QString &unit);
    void setMode(Mode mode);
    Mode mode() const { return mode_; }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QList<QPair<QString, double>> values_;
    QString unit_;
    Mode mode_ = Mode::Bars;
};
class STACKJUNKIE_EXPORT Canvas : public QWidget {
public:
    explicit Canvas(QWidget *parent = nullptr);
    void addRectangle(const QString &name, const QRectF &bounds, const QString &colorRole = QStringLiteral("accent"));
    void addEllipse(const QString &name, const QRectF &bounds, const QString &colorRole = QStringLiteral("success"));
    void setZoomPercent(int percent);
    void applyTheme(const Theme &theme);
    QString selectedName() const;
    QPointF selectedPosition() const;
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
private:
    void synchronizeSelection();
    QGraphicsScene *scene_;
    QGraphicsView *view_;
    QComboBox *selection_;
    QDoubleSpinBox *x_;
    QDoubleSpinBox *y_;
};
class STACKJUNKIE_EXPORT SettingsPage : public QScrollArea {
public:
    explicit SettingsPage(const QString &title, QWidget *parent = nullptr);
    QVBoxLayout *contentLayout() const { return content_; }
private:
    QVBoxLayout *content_;
};
class STACKJUNKIE_EXPORT AboutPage : public SettingsPage {
public:
    AboutPage(const QString &productName, const QString &description, const QString &productVersion = {}, QWidget *productLogo = nullptr, QWidget *parent = nullptr);
};
// The thin bottom row of a desktop app: the current item's state on the left, small facts on the right. Never ads or tips.
class STACKJUNKIE_EXPORT StatusBar : public QFrame {
public:
    explicit StatusBar(QWidget *parent = nullptr);
    void setState(const QString &text);
    QString state() const;
    /** Replaces the small facts on the right, such as a version or a count. */ void setFacts(const QStringList &facts);
    QStringList facts() const { return facts_; }
private:
    QLabel *state_;
    QHBoxLayout *factsLayout_;
    QStringList facts_;
};
// A panel under the work area with its own tab row, such as Output and Problems. In a Workspace it sits in a vertical
// splitter, so people drag or arrow-key its top edge to resize it.
class STACKJUNKIE_EXPORT BottomPanel : public QFrame {
public:
    explicit BottomPanel(QWidget *parent = nullptr);
    /** Adds a view with the usual content padding. */ int addTab(QWidget *content, const QString &label);
    int count() const;
    int currentIndex() const;
    void setCurrentIndex(int index);
    /** Shows a close control in the tab row that calls this. Workspace uses it to hide the panel. */ void setCloseHandler(std::function<void()> handler);
private:
    QTabBar *tabBar_;
    QStackedWidget *views_;
    IconButton *close_;
    std::function<void()> closeHandler_;
    std::function<void()> tabsChanged_;
    friend class Workspace;
};
class STACKJUNKIE_EXPORT Workspace : public QWidget {
    Q_OBJECT
public:
    explicit Workspace(QWidget *parent = nullptr);
    int addSection(const QString &title, const QString &iconName = QStringLiteral("grid"));
    int addPage(const QString &title, QWidget *page, const QString &iconName = QStringLiteral("grid"), int section = -1);
    void setCurrentSection(int section);
    void setSettingsSection(int section);
    /** The rail's gear calls this instead of switching sections. Use it to open a SettingsWindow. */
    void setSettingsHandler(std::function<void()> handler);
    /** Let people drag the sidebar to any width. */ void setResizable(bool resizable);
    /** Let people hide the sidebar with the menu control. */ void setCollapsible(bool collapsible);
    void setSectionVisible(int section, bool visible);
    void setRailVisible(bool visible);
    void setCurrentPage(int index);
    int currentPage() const;
    void setSidebarVisible(bool visible);
    bool sidebarVisible() const;
    QWidget *toolbar() const { return toolbar_; }
    void setHeader(const QString &title);
    void restoreSettings(const QString &key);
    void saveSettings(const QString &key) const;
    /** Apply App frame choices, usually Preset::resolve(id, overrides). The frame draws the rail, the sidebar as a tree
        (NavTree), a list or nothing, closable document tabs and the density. Without the rail the sidebar lists every
        page; without a sidebar the pages become the one tab row. Theme and command palette belong to the app: AppShell
        applies the theme, and an app that owns a palette reads choices().commandPalette. The Menu bar, Status bar and
        bottom panel are drawn when their choices are on. The frame does not draw a Dashboard; an app that has one reads choices().dashboardHome. */
    void setChoices(const PresetChoices &choices);
    PresetChoices choices() const { return choices_; }
    /** The Menu bar: File, Edit, View, the app's menus, then Help, shown when choices().menuBar is on. File holds
        Settings (which calls the settings handler) and Help holds About (the about handler). View holds the sidebar,
        bottom panel and Status bar toggles. Edit holds only app items and is hidden without them. */
    QMenuBar *menuBar() const { return menuBar_; }
    QMenu *fileMenu() const { return fileMenu_; }
    QMenu *editMenu() const { return editMenu_; }
    QMenu *viewMenu() const { return viewMenu_; }
    QMenu *helpMenu() const { return helpMenu_; }
    /** Adds one of the app's own menus, between View and Help. */ QMenu *addMenu(const QString &title);
    /** Adds an app item. In File, Edit, View or Help it sits above the frame's own items. The shortcut is shown in the
        menu and works while the Menu bar is shown. */
    QAction *addMenuItem(QMenu *menu, const QString &text, std::function<void()> run, const QKeySequence &shortcut = {});
    /** Help > About calls this. AppShell opens its Settings window at About. */ void setAboutHandler(std::function<void()> handler);
    /** Shown when choices().statusBar is on. The app sets its state and facts. */ StatusBar *statusBar() const { return statusBar_; }
    /** Shown when choices().bottomPanel is on and the app has added a tab. */ BottomPanel *bottomPanel() const { return bottomPanel_; }
    /** Hide or show the Status bar or bottom panel for this session, as the View menu does. The choices decide whether they exist. */
    void setStatusBarShown(bool shown);
    void setBottomPanelShown(bool shown);
signals:
    void currentChanged(int index);
    void choicesChanged(const StackJunkie::PresetChoices &choices);protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void refreshNavigation();
    void refreshTabRow();
    void syncNavigation();
    void openDocument(int page);
    void closeDocument(int page);
    void updateOpenState();
    void applyDensity();
    void updateBars();
    bool listedSection(int section) const { return section != settingsSection_; }
    QMenuBar *menuBar_ = nullptr;
    QMenu *fileMenu_ = nullptr, *editMenu_ = nullptr, *viewMenu_ = nullptr, *helpMenu_ = nullptr;
    /** The first of the frame's own items in each frame menu; app items go above it. */ QHash<QMenu *, QAction *> frameItems_;
    QAction *settingsAction_ = nullptr, *aboutAction_ = nullptr, *sidebarAction_ = nullptr, *panelAction_ = nullptr, *statusAction_ = nullptr;
    StatusBar *statusBar_ = nullptr;
    BottomPanel *bottomPanel_ = nullptr;
    QSplitter *workSplit_ = nullptr;
    QShortcut *menuKey_ = nullptr;
    bool statusShown_ = true, panelShown_ = true;
    std::function<void()> aboutHandler_;
    QListWidget *navigation_;
    NavTree *tree_;
    QWidget *work_;
    QFrame *tabRow_;
    QTabBar *pageTabs_;
    QLabel *emptyPages_;
    PresetChoices choices_;
    QList<int> openPages_;
    bool collapsible_ = true;    QWidget *sidebar_;
    QLabel *contextHeading_;
    QList<int> pageSections_;
    QList<QString> pageTitles_;
    QList<QLabel *> pageLabels_;
    QList<int> sectionSelections_;
    int currentSection_ = -1;
    QListWidget *rail_;
    QWidget *railContainer_;
    IconButton *settingsButton_;
    int settingsSection_ = -1;
    QStackedWidget *pages_;
    QSplitter *splitter_;
    QWidget *toolbar_;
    QLabel *heading_;
    IconButton *collapse_;
    int savedWidth_ = 220;
    std::function<void()> settingsHandler_;

};
// The standard frame every Stack Junkie app uses: rail, sidebar, Settings window with Appearance and About.
// Apps supply sections, pages, and their own settings sections.
class STACKJUNKIE_EXPORT AppShell : public Workspace {
public:
    AppShell(const QString &name, const QString &description, const QString &version, QWidget *parent = nullptr);
    SettingsWindow *settingsWindow() const { return settings_; }
    /** Product settings appear before the built-in Appearance and About sections. Call before show(). */
    void addSettingsSection(const QString &label, QWidget *content, const QString &keywords = {});
    void setDark(bool dark);
    bool isDark() const { return dark_; }
    /** Start from a preset and replace single choices, for example {{"sidebar", "list"}}. Unknown ids use Web app.
        Applies the layout and density (see Workspace::setChoices) and the default theme, where "system" follows the
        device's color scheme until someone picks a theme in Settings. */
    void setPreset(const QString &presetId, const QVariantMap &overrides = {});
    QString preset() const { return preset_; }
protected:
    void showEvent(QShowEvent *event) override;
private:
    void finishSettings();
    SettingsWindow *settings_;
    QString name_, description_, version_;
    bool dark_ = true;
    bool settingsFinished_ = false;
    QString preset_;
    QComboBox *mode_ = nullptr;
    QAction *darkAction_ = nullptr;
    int productSections_ = 0;
    QMetaObject::Connection followSystem_;
};
// Development stand-in content (sample text, fake data, unfinished labels). Create it only with
// STACKJUNKIE_STUB. Development builds paint it striped; release builds fail at each remaining Stub.
// Hint text inside an empty input is not a Stub.
class STACKJUNKIE_EXPORT Stub : public QFrame {
public:
    struct Site { const char *file; int line; };
    Stub(Site site, const QString &text, QWidget *parent = nullptr);
    Stub(Site site, QWidget *content, QWidget *parent = nullptr);
    QString location() const { return location_; }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void mark(Site site);
    QString location_;
};
STACKJUNKIE_EXPORT QLabel *heading(const QString &text, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QLabel *paragraph(const QString &text, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QWidget *row(const QList<QWidget *> &widgets, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QWidget *wrap(const QList<QWidget *> &widgets, QWidget *parent = nullptr);
}

// STACKJUNKIE_STUB("Sample text") or STACKJUNKIE_STUB(widget, parent) returns a new StackJunkie::Stub.
// Release builds (NDEBUG, or STACKJUNKIE_RELEASE, which the CMake package sets for Release,
// RelWithDebInfo and MinSizeRel) refuse to compile while any remain, and the error names the file and line.
#if defined(STACKJUNKIE_RELEASE) || defined(NDEBUG)
#define STACKJUNKIE_STUBS_ALLOWED 0
#else
#define STACKJUNKIE_STUBS_ALLOWED 1
#endif
#define STACKJUNKIE_STUB_STRING_(value) #value
#define STACKJUNKIE_STUB_LINE_(value) STACKJUNKIE_STUB_STRING_(value)
#if STACKJUNKIE_STUBS_ALLOWED
#define STACKJUNKIE_STUB(...) (new ::StackJunkie::Stub(::StackJunkie::Stub::Site{__FILE__, __LINE__}, __VA_ARGS__))
#else
#define STACKJUNKIE_STUB(...) ([]() -> ::StackJunkie::Stub * { \
    static_assert(false, "Stub remains at " __FILE__ "(" STACKJUNKIE_STUB_LINE_(__LINE__) "): replace it with real content before a release build."); \
    return nullptr; }())
#endif
