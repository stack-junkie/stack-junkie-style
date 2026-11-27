#pragma once
#include <stackjunkie/theme.hpp>
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
#include <functional>
class QVBoxLayout;
class QButtonGroup;
class QListWidget;
class QSplitterHandle;
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
signals:
    void currentChanged(int index);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    QListWidget *navigation_;
    QWidget *sidebar_;
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
protected:
    void showEvent(QShowEvent *event) override;
private:
    void finishSettings();
    SettingsWindow *settings_;
    QString name_, description_, version_;
    bool dark_ = true;
    bool settingsFinished_ = false;
};
STACKJUNKIE_EXPORT QLabel *heading(const QString &text, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QLabel *paragraph(const QString &text, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QWidget *row(const QList<QWidget *> &widgets, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QWidget *wrap(const QList<QWidget *> &widgets, QWidget *parent = nullptr);
}
