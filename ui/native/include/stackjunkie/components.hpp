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
class STACKJUNKIE_EXPORT Tabs : public QTabWidget {
public:
    explicit Tabs(QWidget *parent = nullptr);
};
class STACKJUNKIE_EXPORT GroupedTabs : public QWidget {
public:
    explicit GroupedTabs(QWidget *parent = nullptr);
    int addGroup(const QString &name);
    int addTab(int group, const QString &title, QWidget *page);
private:
    QTabWidget *groups_;
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

};
STACKJUNKIE_EXPORT QLabel *heading(const QString &text, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QLabel *paragraph(const QString &text, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QWidget *row(const QList<QWidget *> &widgets, QWidget *parent = nullptr);
STACKJUNKIE_EXPORT QWidget *wrap(const QList<QWidget *> &widgets, QWidget *parent = nullptr);
}
