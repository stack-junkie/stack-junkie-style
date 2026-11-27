#include <stackjunkie/components.hpp>
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QDate>
#include <stackjunkie/version.hpp>
#include <QButtonGroup>
#include <QClipboard>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QSplitterHandle>
#include <QStyle>
#include <QStyleHints>
#include <QTabBar>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QFontMetrics>
#include <QComboBox>
#include <QShortcut>
#include <QShowEvent>
#include <QtGlobal>

namespace StackJunkie {
ReadingView::ReadingView(const QString &html, QWidget *parent) : QTextBrowser(parent), sourceHtml_(html) {
    setProperty("sjFontRole", "editorial");
    setProperty("sjRole", "readingSurface");
    setAccessibleName(QStringLiteral("Reading surface"));
    setFrameShape(QFrame::NoFrame);
    applyTheme(Theme::load());
}
void ReadingView::setContentHtml(const QString &html) {
    sourceHtml_ = html;
    for (QWidget *scope = this; scope; scope = scope->parentWidget()) {
        if (scope->property("sjDirection").isValid()) {
            applyTheme(Theme::load(scope->property("sjDirection").toString(), scope->property("sjDark").toBool()));
            return;
        }
    }
    applyTheme(Theme::load());
}
void ReadingView::applyTheme(const Theme &theme) {
    const int scroll = verticalScrollBar()->value();
    const int selectionAnchor = textCursor().anchor();
    const int selectionPosition = textCursor().position();
    QFont body(theme.font(QStringLiteral("editorial")));
    body.setPixelSize(Theme::typeSize(QStringLiteral("reading"), 16));
    body.setWeight(static_cast<QFont::Weight>(Theme::typography(QStringLiteral("regularWeight"), 400)));
    document()->setDefaultFont(body);
    document()->setDefaultStyleSheet(QStringLiteral(
        "body,p,li { font-family: '%1'; font-size: %2px; font-weight: %3; }"
        "h1,h2,h3 { font-family: '%1'; font-weight: %4; margin-top: %8px; margin-bottom: %9px; }"
        "h1 { font-size: %5px; } h2 { font-size: %6px; } h3 { font-size: %7px; }")
        .arg(body.family()).arg(body.pixelSize()).arg(Theme::typography(QStringLiteral("regularWeight"), 400))
        .arg(Theme::typography(QStringLiteral("strongWeight"), 600))
        .arg(Theme::typeSize(QStringLiteral("title"), 28)).arg(Theme::typeSize(QStringLiteral("heading"), 22))
        .arg(Theme::typeSize(QStringLiteral("subheading"), 18)).arg(Theme::spacing(3)).arg(Theme::spacing(1)));
    QTextBrowser::setHtml(sourceHtml_);
    for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
        QTextCursor cursor(block);
        auto format = block.blockFormat();
        format.setLineHeight(Theme::typography(QStringLiteral("readingLineHeight"), 1.6) * 100, QTextBlockFormat::ProportionalHeight);
        cursor.setBlockFormat(format);
    }
    const int measure = qRound(QFontMetrics(body).horizontalAdvance(QLatin1Char('0')) * Theme::typography(QStringLiteral("readingMeasure"), 65));
    setMaximumWidth(measure + 2 * Theme::spacing(3) + verticalScrollBar()->sizeHint().width());
    QTextCursor restored(document());
    restored.setPosition(qMin(selectionAnchor, document()->characterCount() - 1));
    restored.setPosition(qMin(selectionPosition, document()->characterCount() - 1), QTextCursor::KeepAnchor);
    setTextCursor(restored);
    document()->documentLayout()->documentSize();
    verticalScrollBar()->setValue(scroll);
}
namespace {
class WrapLayout final : public QLayout {
public:
    explicit WrapLayout(QWidget *parent) : QLayout(parent) { setContentsMargins(0, 0, 0, 0); setSpacing(Theme::spacing(1)); }
    ~WrapLayout() override { while (auto *item = takeAt(0)) delete item; }
    void addItem(QLayoutItem *item) override { items_.append(item); }
    int count() const override { return items_.size(); }
    QLayoutItem *itemAt(int index) const override { return items_.value(index); }
    QLayoutItem *takeAt(int index) override { return index >= 0 && index < items_.size() ? items_.takeAt(index) : nullptr; }
    Qt::Orientations expandingDirections() const override { return Qt::Horizontal; }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override { return arrange(QRect(0, 0, width, 0), false); }
    QSize sizeHint() const override { return minimumSize(); }
    QSize minimumSize() const override {
        QSize result;
        for (auto *item : items_) if (!item->isEmpty()) result = result.expandedTo(item->minimumSize());
        return result;
    }
    void setGeometry(const QRect &bounds) override { QLayout::setGeometry(bounds); arrange(bounds, true); }
private:
    int arrange(const QRect &bounds, bool apply) const {
        int x = bounds.x(), y = bounds.y(), lineHeight = 0;
        for (auto *item : items_) {
            if (item->isEmpty()) continue;
            QSize size = item->sizeHint();
            size.setWidth(qMin(size.width(), bounds.width()));
            if (lineHeight && x + size.width() > bounds.right() + 1) { x = bounds.x(); y += lineHeight + spacing(); lineHeight = 0; }
            if (item->hasHeightForWidth()) size.setHeight(item->heightForWidth(size.width()));
            if (apply) item->setGeometry(QRect(QPoint(x, y), size));
            x += size.width() + spacing();
            lineHeight = qMax(lineHeight, size.height());
        }
        return y + lineHeight - bounds.y();
    }
    QList<QLayoutItem *> items_;
};
class AccessibleSplitHandle final : public QSplitterHandle {
public:
    explicit AccessibleSplitHandle(Qt::Orientation direction, SplitPane *parent) : QSplitterHandle(direction, parent) {
        setFocusPolicy(Qt::StrongFocus);
        setAccessibleName(QStringLiteral("Resize adjacent panes"));
        setAccessibleDescription(QStringLiteral("Use arrow keys to resize. Shift makes larger steps. Home and End move to the available limits."));
    }
protected:
    void keyPressEvent(QKeyEvent *event) override {
        const int key = event->key();
        if (key == Qt::Key_Home || key == Qt::Key_End || (orientation() == Qt::Horizontal && (key == Qt::Key_Left || key == Qt::Key_Right)) || (orientation() == Qt::Vertical && (key == Qt::Key_Up || key == Qt::Key_Down))) {
            static_cast<SplitPane *>(splitter())->resizeFromKeyboard(this, key, event->modifiers());
            event->accept();
        } else QSplitterHandle::keyPressEvent(event);
    }
};
}
SplitPane::SplitPane(QWidget *parent) : SplitPane(Qt::Horizontal, parent) {}
SplitPane::SplitPane(Qt::Orientation orientation, QWidget *parent) : QSplitter(orientation, parent) { setChildrenCollapsible(false); setHandleWidth(6); }
QSplitterHandle *SplitPane::createHandle() { return new AccessibleSplitHandle(orientation(), this); }
void SplitPane::setStackBelowWidth(int width) { stackBelowWidth_ = qMax(0, width); if (stackBelowWidth_) setOrientation(this->width() < stackBelowWidth_ ? Qt::Vertical : Qt::Horizontal); }
void SplitPane::resizeEvent(QResizeEvent *event) {
    if (stackBelowWidth_) {
        const auto direction = width() < stackBelowWidth_ ? Qt::Vertical : Qt::Horizontal;
        if (orientation() != direction) setOrientation(direction);
    }
    QSplitter::resizeEvent(event);
}
void SplitPane::resizeFromKeyboard(QSplitterHandle *target, int key, Qt::KeyboardModifiers modifiers) {
    int index = 1;
    while (index < count() && handle(index) != target) ++index;
    if (index >= count()) return;
    const int step = modifiers.testFlag(Qt::ShiftModifier) ? Theme::spacing(5) : Theme::spacing(1);
    int position = orientation() == Qt::Horizontal ? target->pos().x() : target->pos().y();
    if (key == Qt::Key_Home) position = 0;
    else if (key == Qt::Key_End) position = orientation() == Qt::Horizontal ? width() : height();
    else position += (key == Qt::Key_Left || key == Qt::Key_Up) ? -step : step;
    moveSplitter(closestLegalPosition(position, index), index);
}
Button::Button(const QString &text, ButtonVariant variant, QWidget *parent) : QPushButton(text, parent), normalLabel_(text) {
    const char *names[] = {"primary", "secondary", "quiet", "danger"};
    setProperty("sjVariant", names[static_cast<int>(variant)]);
    setAccessibleName(text);
    setMinimumHeight(0);
    setCursor(Qt::PointingHandCursor);
}
void Button::setBusy(bool busy, const QString &busyLabel) {
    if (busy && isEnabled()) normalLabel_ = text();
    setText(busy ? busyLabel : normalLabel_);
    setEnabled(!busy);
    setAccessibleDescription(busy ? QStringLiteral("Operation in progress") : QString());
}
IconButton::IconButton(const QString &iconName, const QString &accessibleName, QWidget *parent) : QToolButton(parent) {
    setProperty("sjIcon", iconName);
    setProperty("sjRole", "iconTarget");
    setIcon(StackJunkie::icon(iconName, palette().color(QPalette::ButtonText)));
    setAccessibleName(accessibleName);
    setToolTip(accessibleName);
    const int target = Theme::density(QStringLiteral("iconTarget"), 28);
    setFixedSize(target, target);
    setFocusPolicy(Qt::StrongFocus);
}
Field::Field(const QString &label, QWidget *input, const QString &help, QWidget *parent) : QWidget(parent), input_(input) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::spacing(0));
    auto *line = new QHBoxLayout;
    auto *caption = new QLabel(label, this);
    caption->setBuddy(input);
    caption->setWordWrap(true);
    line->addWidget(caption);
    if (!help.isEmpty()) line->addWidget(new HelpButton(QStringLiteral("About ") + label, help, this));
    line->addStretch();
    layout->addLayout(line);
    input->setAccessibleName(label);
    layout->addWidget(input);
    error_ = new QLabel(this);
    error_->setProperty("sjRole", "error");
    error_->setWordWrap(true);
    error_->hide();
    layout->addWidget(error_);
}
void Field::setError(const QString &message) {
    error_->setText(message);
    error_->setVisible(!message.isEmpty());
    input_->setProperty("sjInvalid", !message.isEmpty());
    input_->setAccessibleDescription(message);
    input_->style()->unpolish(input_);
    input_->style()->polish(input_);
}
SearchField::SearchField(const QString &label, QWidget *parent) : QLineEdit(parent) {
    setAccessibleName(label);
    setPlaceholderText(label);
    setClearButtonEnabled(true);
}
SegmentedControl::SegmentedControl(const QStringList &options, QWidget *parent) : QWidget(parent), group_(new QButtonGroup(this)) {
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    int id = 0;
    for (const auto &option : options) {
        auto *button = new QToolButton(this);
        button->setText(option);
        button->setAccessibleName(option);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::StrongFocus);
        group_->addButton(button, id++);
        layout->addWidget(button);
    }
    if (group_->button(0)) group_->button(0)->setChecked(true);
    connect(group_, &QButtonGroup::idClicked, this, &SegmentedControl::currentChanged);
}
int SegmentedControl::currentIndex() const { return group_->checkedId(); }
void SegmentedControl::setCurrentIndex(int index) {
    if (auto *button = group_->button(index)) {
        const bool changed = currentIndex() != index;
        button->setChecked(true);
        if (changed) emit currentChanged(index);
    }
}
HelpButton::HelpButton(const QString &name, const QString &text, QWidget *parent) : QToolButton(parent), popup_(new QFrame(this, Qt::Tool | Qt::FramelessWindowHint)) {
    setText(QStringLiteral("?"));
    setAccessibleName(name);
    setFocusPolicy(Qt::StrongFocus);
    setProperty("sjRole", "iconTarget");
    const int target = Theme::density(QStringLiteral("iconTarget"), 28);
    setFixedSize(target, target);
    setCheckable(true);
    popup_->setProperty("sjRole", "help");
    popup_->setAccessibleName(name);
    popup_->setAttribute(Qt::WA_ShowWithoutActivating);
    auto *layout = new QVBoxLayout(popup_);
    layout->setContentsMargins(14, 12, 14, 12);
    auto *body = paragraph(text, popup_);
    body->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByKeyboard);
    body->setMaximumWidth(320);
    layout->addWidget(body);
    popup_->setMaximumWidth(352);
    closeTimer_.setSingleShot(true);
    closeTimer_.setInterval(180);
    connect(&closeTimer_, &QTimer::timeout, this, [this] { if (!pinned_ && !underMouse() && !hasFocus() && !popup_->underMouse()) dismiss(); });
    connect(this, &QToolButton::clicked, this, [this] { if (pinned_) dismiss(); else { dismissed_ = false; showHelp(true); } });
    qApp->installEventFilter(this);
}
HelpButton::~HelpButton() { qApp->removeEventFilter(this); }
void HelpButton::showHelp(bool pin) {
    if (dismissed_ && !pin) return;
    pinned_ = pin || pinned_;
    setChecked(pinned_);
    setAccessibleDescription(pinned_ ? QStringLiteral("Help pinned. Press Escape to dismiss.") : QStringLiteral("Click to pin help."));
    closeTimer_.stop();
    // A tool window does not inherit its parent's stylesheet reliably on every platform.
    for (QWidget *scope = this; scope; scope = scope->parentWidget()) {
        if (scope->property("sjDirection").isValid()) {
            Theme::load(scope->property("sjDirection").toString(), scope->property("sjDark").toBool()).apply(popup_);
            break;
        }
    }
    popup_->adjustSize();
    const QRect available = screen() ? screen()->availableGeometry() : QRect(0, 0, 1920, 1080);
    QPoint point = mapToGlobal(QPoint(width() + 8, 0));
    point.setX(qBound(available.left(), point.x(), qMax(available.left(), available.right() - popup_->width())));
    point.setY(qBound(available.top(), point.y(), qMax(available.top(), available.bottom() - popup_->height())));
    popup_->move(point);
    popup_->show();
}
void HelpButton::dismiss() { popup_->hide(); pinned_ = false; setChecked(false); closeTimer_.stop(); }
bool HelpButton::eventFilter(QObject *object, QEvent *event) {
    if (object == this) {
        if (event->type() == QEvent::Enter || event->type() == QEvent::FocusIn) { dismissed_ = false; showHelp(false); }
        if (event->type() == QEvent::Leave || event->type() == QEvent::FocusOut) closeTimer_.start();
        if (event->type() == QEvent::Hide) dismiss();
    }
    if (object == popup_) {
        if (event->type() == QEvent::Enter) closeTimer_.stop();
        if (event->type() == QEvent::Leave) closeTimer_.start();
    }
    if (popup_->isVisible()) {
        if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape) { dismiss(); dismissed_ = true; return true; }
        if (event->type() == QEvent::MouseButtonPress) {
            auto *target = qobject_cast<QWidget *>(object);
            if (target && target != this && !isAncestorOf(target) && target != popup_ && !popup_->isAncestorOf(target)) dismiss();
        }
        if (event->type() == QEvent::ApplicationDeactivate) dismiss();
    }
    return QToolButton::eventFilter(object, event);
}
Disclosure::Disclosure(const QString &label, QWidget *content, bool expanded, QWidget *parent) : QFrame(parent), content_(content) {
    setProperty("sjRole", "disclosure");
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::spacing(1), Theme::spacing(1), Theme::spacing(1), Theme::spacing(1));
    toggle_ = new QToolButton(this);
    toggle_->setText(label);
    toggle_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggle_->setCheckable(true);
    toggle_->setAccessibleName(label);
    toggle_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(toggle_);
    layout->addWidget(content_);
    connect(toggle_, &QToolButton::toggled, this, [this](bool open) {
        content_->setVisible(open);
        toggle_->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
        toggle_->setAccessibleDescription(open ? QStringLiteral("Expanded") : QStringLiteral("Collapsed"));
    });
    setExpanded(expanded);
    content_->setVisible(expanded);
    toggle_->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
}
bool Disclosure::isExpanded() const { return toggle_->isChecked(); }
void Disclosure::setExpanded(bool expanded) { toggle_->setChecked(expanded); }
Notice::Notice(const QString &text, const QString &tone, QWidget *parent) : QFrame(parent) {
    setProperty("sjRole", "notice");
    setProperty("sjTone", tone);
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    text_ = paragraph(text, this);
    layout->addWidget(text_);
    setAccessibleName(tone + QStringLiteral(": ") + text);
}
void Notice::setText(const QString &text) { text_->setText(text); setAccessibleName(property("sjTone").toString() + QStringLiteral(": ") + text); }
Tag::Tag(const QString &text, const QString &tone, QWidget *parent) : QLabel(text, parent) {
    setProperty("sjRole", "tag"); setProperty("sjTone", tone);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
}
EmptyState::EmptyState(const QString &title, const QString &description, QWidget *action, QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::spacing(4), Theme::spacing(4), Theme::spacing(4), Theme::spacing(4));
    layout->addWidget(heading(title, this));
    if (!description.isEmpty()) layout->addWidget(paragraph(description, this));
    if (action) layout->addWidget(action, 0, Qt::AlignLeft);
}
CodeBlock::CodeBlock(const QString &text, QWidget *parent) : QFrame(parent) {
    setProperty("sjRole", "code");
    auto *layout = new QVBoxLayout(this);
    auto *copy = new Button(QStringLiteral("Copy"), ButtonVariant::Quiet, this);
    auto *editor = new QPlainTextEdit(text, this);
    editor->setProperty("sjRole", "code");
    editor->setReadOnly(true);
    editor->setAccessibleName(QStringLiteral("Code output"));
    editor->setMinimumHeight(120);
    layout->addWidget(copy, 0, Qt::AlignRight);
    layout->addWidget(editor);
    connect(copy, &QPushButton::clicked, this, [copy, text] {
        QGuiApplication::clipboard()->setText(text);
        copy->setText(QStringLiteral("Copied"));
        QTimer::singleShot(1200, copy, [copy] { copy->setText(QStringLiteral("Copy")); });
    });
}
Tabs::Tabs(QWidget *parent) : QTabWidget(parent) {
    setDocumentMode(true);
    tabBar()->setDrawBase(false);
    setUsesScrollButtons(true);
    setElideMode(Qt::ElideNone);
}
void Tabs::setDocumentTabs(bool closable) {
    setTabsClosable(closable); setMovable(closable);
    for (int i = 0; i < count(); ++i) tabInserted(i);
    if (closable) {
        // Qt::UniqueConnection needs a member function; a lambda asserts in debug builds.
        connect(this, &QTabWidget::tabCloseRequested, this, &QTabWidget::removeTab, Qt::UniqueConnection);
        auto *close = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
        connect(close, &QShortcut::activated, this, [this] { if (currentIndex() >= 0) emit tabCloseRequested(currentIndex()); });
    }
}
// Closable tabs use the library's quiet close icon instead of the platform's default button.
void Tabs::tabInserted(int index) {
    QTabWidget::tabInserted(index);
    if (!tabsClosable()) return;
    QWidget *page = widget(index);
    auto *close = new IconButton(QStringLiteral("close"), QStringLiteral("Close ") + tabText(index));
    close->setProperty("sjVariant", "quiet"); close->setProperty("sjRole", "tabClose"); close->setFixedSize(18, 18); close->setIconSize(QSize(12, 12));
    close->setFocusPolicy(Qt::NoFocus);
    connect(close, &QToolButton::clicked, this, [this, page] { const int at = indexOf(page); if (at >= 0) emit tabCloseRequested(at); });
    tabBar()->setTabButton(index, QTabBar::RightSide, close);
}
void Tabs::showEvent(QShowEvent *event) {
    QTabWidget::showEvent(event);
#ifndef NDEBUG
    for (QWidget *scope = parentWidget(); scope; scope = scope->parentWidget()) {
        if (scope->property("sjTabBoundary").toBool()) break;
        if (qobject_cast<QTabWidget *>(scope)) {
            qWarning("Stack Junkie UI: tab row \"%s\" sits inside another tab page. Stacked tab rows are not allowed; use the sidebar tree, a toolbar toggle or a TabBoundary frame.", qPrintable(accessibleName()));
            break;
        }
    }
#endif
}
SettingsPage::SettingsPage(const QString &title, QWidget *parent) : QScrollArea(parent) {
    setWidgetResizable(true); setFrameShape(QFrame::NoFrame);
    auto *body = new QWidget;
    body->setProperty("sjContentPadding", true);
    auto *layout = new QVBoxLayout(body);
    const int padding = Theme::density(QStringLiteral("contentPadding"), 20);
    layout->setContentsMargins(padding, padding, padding, padding);
    layout->setSpacing(20);
    auto *titleLabel = heading(title); titleLabel->setProperty("sjRole", "productTitle");
    layout->addWidget(titleLabel);
    content_ = new QVBoxLayout; content_->setSpacing(16);
    layout->addLayout(content_); layout->addStretch(); setWidget(body);
}
AboutPage::AboutPage(const QString &productName, const QString &description, const QString &productVersion, QWidget *productLogo, QWidget *parent) : SettingsPage(QStringLiteral("About"), parent) {
    auto *name = heading(productName); name->setProperty("sjRole", "productTitle");
    contentLayout()->addWidget(productLogo ? row({productLogo, name}) : name);
    contentLayout()->addWidget(paragraph(description));
    if (!productVersion.isEmpty()) contentLayout()->addWidget(paragraph(QStringLiteral("Version %1").arg(productVersion)));
    auto *separator = new QFrame; separator->setFrameShape(QFrame::HLine); separator->setProperty("sjRole", "separator"); contentLayout()->addWidget(separator);
    QFile source(QStringLiteral(":/stackjunkie/attribution.json"));
    if (!source.open(QIODevice::ReadOnly)) throw std::runtime_error("Stack Junkie attribution resource missing");
    const auto info = QJsonDocument::fromJson(source.readAll()).object();
    auto *creditLogo = new QLabel; creditLogo->setFixedSize(22, 22); creditLogo->setProperty("sjBrandSize", 22);
    auto *credit = paragraph(QStringLiteral("%1 ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â· %2<br><a href=\"%3\">%4</a><br>ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â© %5 %6")
        .arg(info.value("name").toString().toHtmlEscaped(), QStringLiteral(STACKJUNKIE_VERSION), info.value("website").toString().toHtmlEscaped(), info.value("projectLine").toString().toHtmlEscaped())
        .arg(QDate::currentDate().year()).arg(info.value("copyrightHolder").toString().toHtmlEscaped()));
    credit->setTextFormat(Qt::RichText); credit->setOpenExternalLinks(true); credit->setProperty("sjRole", "attribution");
    credit->setProperty("sjAttributionHtml", credit->text());
    contentLayout()->addWidget(row({creditLogo, credit}));
}
Workspace::Workspace(QWidget *parent) : QWidget(parent) {
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    toolbar_ = new QWidget(this);
    toolbar_->setProperty("sjRole", "shellHeader");
    toolbar_->setMinimumHeight(Theme::density(QStringLiteral("headerHeight"), 40));
    auto *toolbarLayout = new QHBoxLayout(toolbar_);
    toolbarLayout->setContentsMargins(Theme::spacing(1), Theme::spacing(0), Theme::spacing(1), Theme::spacing(0));
    collapse_ = new IconButton(QStringLiteral("menu"), QStringLiteral("Toggle contextual sidebar"), toolbar_);
    collapse_->setCheckable(true);
    collapse_->setChecked(true);
    heading_ = new QLabel(QStringLiteral("Workspace"), toolbar_);
    heading_->setProperty("sjRole", "shellTitle");
    toolbarLayout->addWidget(heading_);
    toolbarLayout->addStretch();
    outer->addWidget(toolbar_);
    auto *body = new QHBoxLayout;
    body->setSpacing(0);
    rail_ = new QListWidget(this);
    rail_->setProperty("sjRole", "rail");
    rail_->setAccessibleName(QStringLiteral("Sections"));
    rail_->setFixedWidth(Theme::density(QStringLiteral("railWidth"), 48));
    rail_->setIconSize(QSize(18, 18));
    rail_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    railContainer_ = new QWidget(this);
    auto *railLayout = new QVBoxLayout(railContainer_); railLayout->setContentsMargins(0, 0, 0, 0); railLayout->setSpacing(0);
    collapse_->setFixedSize(Theme::density(QStringLiteral("railWidth"), 48) - 8, Theme::density(QStringLiteral("contextHeight"), 36));
    collapse_->setProperty("sjVariant", "quiet");
    collapse_->setProperty("sjRole", "railToggle");
    collapse_->setToolTip(QStringLiteral("Hide navigation"));
    collapse_->setAccessibleName(QStringLiteral("Hide navigation"));
    railLayout->addWidget(collapse_, 0, Qt::AlignHCenter);
    railLayout->addWidget(rail_, 1);
    settingsButton_ = new IconButton(QStringLiteral("settings"), QStringLiteral("Settings"));
    settingsButton_->setCheckable(true); settingsButton_->hide();
    settingsButton_->setFixedSize(Theme::density(QStringLiteral("railWidth"), 48) - 8, Theme::density(QStringLiteral("contextHeight"), 36));
    settingsButton_->setProperty("sjVariant", "quiet");
    railLayout->addWidget(settingsButton_, 0, Qt::AlignHCenter);
    connect(settingsButton_, &QToolButton::clicked, this, [this] {
        if (settingsHandler_) { settingsButton_->setChecked(false); settingsHandler_(); return; }
        if (settingsSection_ >= 0) { setCurrentSection(settingsSection_); setSidebarVisible(true); }
    });
    body->addWidget(railContainer_);
    splitter_ = new SplitPane(this);
    splitter_->setHandleWidth(5);
    sidebar_ = new QWidget(splitter_);
    sidebar_->setMinimumWidth(0);
    sidebar_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    auto *sidebarLayout = new QVBoxLayout(sidebar_);
    sidebarLayout->setContentsMargins(0, 0, 0, 0); sidebarLayout->setSpacing(0); sidebarLayout->setSizeConstraint(QLayout::SetNoConstraint);
    contextHeading_ = new QLabel(sidebar_);
    contextHeading_->setProperty("sjRole", "contextTitle");
    contextHeading_->setMinimumHeight(Theme::density(QStringLiteral("contextHeight"), 36));
    contextHeading_->setWordWrap(true);
    sidebarLayout->addWidget(contextHeading_);
    navigation_ = new QListWidget(sidebar_);
    navigation_->setProperty("sjRole", "navigation");
    navigation_->setAccessibleName(QStringLiteral("Context pages"));
    sidebarLayout->addWidget(navigation_, 1);
    tree_ = new NavTree(QStringLiteral("Pages"), sidebar_);
    tree_->hide();
    sidebarLayout->addWidget(tree_, 1);
    // The work area: the frame's one tab row (open documents, or the pages when there is no sidebar) above the pages.
    work_ = new QWidget(splitter_);
    work_->setMinimumWidth(0);
    work_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    auto *workLayout = new QVBoxLayout(work_); workLayout->setContentsMargins(0, 0, 0, 0); workLayout->setSpacing(0);
    tabRow_ = new QFrame(work_); tabRow_->setProperty("sjRole", "pageTabRow"); tabRow_->hide();
    auto *tabRowLayout = new QHBoxLayout(tabRow_); tabRowLayout->setContentsMargins(0, 0, 0, 0); tabRowLayout->setSpacing(0);
    pageTabs_ = new QTabBar(tabRow_);
    pageTabs_->setAccessibleName(QStringLiteral("Pages"));
    pageTabs_->setDocumentMode(true); pageTabs_->setExpanding(false); pageTabs_->setUsesScrollButtons(true); pageTabs_->setDrawBase(false);
    tabRowLayout->addWidget(pageTabs_); tabRowLayout->addStretch();
    workLayout->addWidget(tabRow_);
    pages_ = new QStackedWidget(work_);
    pages_->setMinimumWidth(0);
    pages_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    workLayout->addWidget(pages_, 1);
    emptyPages_ = new QLabel(QStringLiteral("No open pages. Choose one in the sidebar."), work_);
    emptyPages_->setProperty("sjRole", "muted"); emptyPages_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    const int emptyPadding = Theme::density(QStringLiteral("contentPadding"), 20);
    emptyPages_->setContentsMargins(emptyPadding, emptyPadding, emptyPadding, emptyPadding);
    emptyPages_->setProperty("sjContentPadding", true); emptyPages_->hide();
    workLayout->addWidget(emptyPages_, 1);
    splitter_->addWidget(sidebar_);
    splitter_->addWidget(work_);
    splitter_->setCollapsible(0, true);
    splitter_->setStretchFactor(1, 1);
    splitter_->setSizes({220, 800});
    body->addWidget(splitter_, 1);
    outer->addLayout(body, 1);
    connect(collapse_, &QToolButton::clicked, this, [this] { setSidebarVisible(collapse_->isChecked()); });
    connect(navigation_, &QListWidget::currentRowChanged, this, [this](int row) { if (auto *item = navigation_->item(row)) setCurrentPage(item->data(Qt::UserRole).toInt()); });
    connect(rail_, &QListWidget::currentRowChanged, this, &Workspace::setCurrentSection);
    connect(rail_, &QListWidget::itemClicked, this, [this](QListWidgetItem *) { if (!sidebarVisible()) setSidebarVisible(true); });
    connect(tree_, &NavTree::pageChosen, this, [this](const QString &id) { setCurrentPage(id.toInt()); });
    connect(pageTabs_, &QTabBar::currentChanged, this, [this](int tab) { if (tab >= 0) setCurrentPage(pageTabs_->tabData(tab).toInt()); });
    connect(pageTabs_, &QTabBar::tabCloseRequested, this, [this](int tab) { closeDocument(pageTabs_->tabData(tab).toInt()); });
    auto *closeTab = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), pageTabs_, nullptr, nullptr, Qt::WidgetShortcut);
    connect(closeTab, &QShortcut::activated, this, [this] { if (pageTabs_->tabsClosable() && pageTabs_->currentIndex() >= 0) closeDocument(pageTabs_->tabData(pageTabs_->currentIndex()).toInt()); });
}
int Workspace::addSection(const QString &title, const QString &iconName) {
    const int section = sectionSelections_.size();
    sectionSelections_.append(-1);
    auto *item = new QListWidgetItem(icon(iconName, palette().color(QPalette::Text)), QString(), rail_);
    item->setData(Qt::AccessibleTextRole, title);
    item->setData(Qt::UserRole + 10, iconName);
    item->setToolTip(title);
    item->setSizeHint(QSize(Theme::density(QStringLiteral("railWidth"), 48) - Theme::spacing(1), Theme::density(QStringLiteral("contextHeight"), 36)));
    return section;
}
void Workspace::setSettingsSection(int section) {
    if (section < 0 || section >= sectionSelections_.size()) return;
    if (settingsSection_ >= 0) rail_->item(settingsSection_)->setHidden(false);
    settingsSection_ = section; rail_->item(section)->setHidden(true); settingsButton_->show();
}
void Workspace::setSettingsHandler(std::function<void()> handler) {
    settingsHandler_ = std::move(handler);
    settingsButton_->setCheckable(!settingsHandler_); settingsButton_->setVisible(settingsHandler_ || settingsSection_ >= 0);
}
void Workspace::setResizable(bool resizable) {
    splitter_->setHandleWidth(resizable ? 5 : 1);
    if (auto *handle = splitter_->handle(1)) { handle->setEnabled(resizable); handle->setCursor(resizable ? Qt::SplitHCursor : Qt::ArrowCursor); }
}
void Workspace::setCollapsible(bool collapsible) {
    collapsible_ = collapsible;
    collapse_->setVisible(collapsible && choices_.sidebar != QStringLiteral("off"));
    if (!collapsible) setSidebarVisible(true);
}
void Workspace::setSectionVisible(int section, bool visible) {
    if (section >= 0 && section < rail_->count()) rail_->item(section)->setHidden(!visible);
}
void Workspace::setRailVisible(bool visible) {
    railContainer_->setVisible(visible);
    if (visible) qobject_cast<QVBoxLayout *>(railContainer_->layout())->insertWidget(0, collapse_, 0, Qt::AlignHCenter);
    else qobject_cast<QHBoxLayout *>(toolbar_->layout())->insertWidget(0, collapse_);
    collapse_->setVisible(collapsible_ && choices_.sidebar != QStringLiteral("off"));
    auto *target = visible ? qobject_cast<QVBoxLayout *>(railContainer_->layout()) : qobject_cast<QVBoxLayout *>(sidebar_->layout());
    target->addWidget(settingsButton_); settingsButton_->setVisible(settingsHandler_ || settingsSection_ >= 0);
    if (!visible) setSidebarVisible(true);
}
int Workspace::addPage(const QString &title, QWidget *page, const QString &iconName, int section) {
    if (sectionSelections_.isEmpty()) addSection(QStringLiteral("Pages"), iconName);
    if (section < 0 || section >= sectionSelections_.size()) section = 0;
    const int index = pages_->addWidget(page);
    pageSections_.append(section); pageTitles_.append(title);
    QLabel *contextLabel = nullptr;
    if (auto *tabs = qobject_cast<QTabWidget *>(page)) {
        contextLabel = new QLabel(title, tabs);
        contextLabel->setProperty("sjRole", "contextTitle");
        contextLabel->setContentsMargins(12, 0, 12, 0);
        contextLabel->setToolTip(title);
        tabs->setCornerWidget(contextLabel, Qt::TopRightCorner);
        contextLabel->setVisible(!sidebarVisible());
    }
    pageLabels_.append(contextLabel);
    if (sectionSelections_[section] < 0) sectionSelections_[section] = index;
    if (currentSection_ == section || !choices_.rail) refreshNavigation();
    if (index == 0) setCurrentPage(0);
    return index;
}
void Workspace::setCurrentSection(int section) {
    if (section < 0 || section >= sectionSelections_.size()) return;
    if (currentSection_ != section) {
        currentSection_ = section;
        // Without the rail the sidebar and tab row list every page, so a new section changes nothing in them.
        if (choices_.rail) refreshNavigation();
    }
    if (rail_->currentRow() != section) { QSignalBlocker blocker(rail_); rail_->setCurrentRow(section); }
    settingsButton_->setChecked(section == settingsSection_);
    const int page = sectionSelections_[section];
    if (page >= 0) setCurrentPage(page);
}
void Workspace::setCurrentPage(int index) {
    if (index < 0 || index >= pageSections_.size()) return;
    const int section = pageSections_[index];
    sectionSelections_[section] = index;
    if (currentSection_ != section) { setCurrentSection(section); return; }
    const bool changed = pages_->currentIndex() != index;
    pages_->setCurrentIndex(index);
    if (choices_.documentTabs && choices_.sidebar != QStringLiteral("off")) openDocument(index);
    syncNavigation();
    if (changed) emit currentChanged(index);
}
int Workspace::currentPage() const { return pages_->currentIndex(); }
void Workspace::setSidebarVisible(bool visible) {
    const bool off = choices_.sidebar == QStringLiteral("off");
    if (off) visible = false;
    if (!visible && !sidebar_->isHidden()) savedWidth_ = qMax(0, splitter_->sizes().value(0, savedWidth_));
    if (!visible && sidebar_->isAncestorOf(QApplication::focusWidget())) collapse_->setFocus();
    sidebar_->setVisible(visible);
    if (railContainer_->isHidden()) {
        // With neither rail nor sidebar, Settings sits at the end of the header.
        auto *target = visible ? qobject_cast<QBoxLayout *>(sidebar_->layout()) : off ? qobject_cast<QBoxLayout *>(toolbar_->layout()) : qobject_cast<QBoxLayout *>(layout());
        target->addWidget(settingsButton_); settingsButton_->setVisible(settingsHandler_ || settingsSection_ >= 0);
    }
    collapse_->setChecked(visible);
    if (visible) splitter_->setSizes({savedWidth_, qMax(0, splitter_->width() - savedWidth_)});
    collapse_->setAccessibleDescription(visible ? QStringLiteral("Sidebar expanded") : QStringLiteral("Sidebar collapsed"));
    collapse_->setAccessibleName(visible ? QStringLiteral("Hide navigation") : QStringLiteral("Show navigation"));
    collapse_->setToolTip(collapse_->accessibleName());
    for (auto *label : pageLabels_) if (label) label->setVisible(!visible);
}
bool Workspace::sidebarVisible() const { return !sidebar_->isHidden(); }
void Workspace::setHeader(const QString &title) { heading_->setText(title); }
void Workspace::restoreSettings(const QString &key) {
    QSettings settings;
    savedWidth_ = qMax(0, settings.value(key + QStringLiteral("/sidebarWidth"), 220).toInt());
    const auto selections = settings.value(key + QStringLiteral("/sectionPages")).toList();
    for (int section = 0; section < qMin(selections.size(), sectionSelections_.size()); ++section) {
        const int page = selections[section].toInt();
        if (page >= 0 && page < pageSections_.size() && pageSections_[page] == section) sectionSelections_[section] = page;
    }
    setSidebarVisible(settings.value(key + QStringLiteral("/sidebarVisible"), true).toBool());
    setCurrentPage(settings.value(key + QStringLiteral("/page"), 0).toInt());
}
void Workspace::saveSettings(const QString &key) const {
    QSettings settings;
    settings.setValue(key + QStringLiteral("/sidebarWidth"), sidebarVisible() ? splitter_->sizes().value(0, savedWidth_) : savedWidth_);
    settings.setValue(key + QStringLiteral("/sidebarVisible"), sidebarVisible());
    settings.setValue(key + QStringLiteral("/page"), currentPage());
    QVariantList selections; for (int page : sectionSelections_) selections.append(page);
    settings.setValue(key + QStringLiteral("/sectionPages"), selections);
}
void Workspace::setChoices(const PresetChoices &choices) {
    const bool wasOff = choices_.sidebar == QStringLiteral("off");
    choices_ = choices;
    // Menu bar, Status bar, bottom panel and dashboard home are accepted but not drawn yet; they arrive with their components in #9.
    setRailVisible(choices.rail);
    if (choices.sidebar == QStringLiteral("off")) setSidebarVisible(false);
    else if (wasOff) setSidebarVisible(true);
    collapse_->setVisible(collapsible_ && choices.sidebar != QStringLiteral("off"));
    if (!choices.documentTabs) openPages_.clear();
    refreshNavigation();
    if (choices.documentTabs && choices.sidebar != QStringLiteral("off") && currentPage() >= 0) openDocument(currentPage());
    updateOpenState();
    if (choices.density != Theme::densityMode()) {
        Theme::setDensityMode(choices.density);
        applyDensity();
        // Re-apply the theme this frame already carries, so the stylesheet picks up the new control sizes.
        if (property("sjDirection").isValid()) Theme::load(property("sjDirection").toString(), property("sjDark").toBool()).apply(this);
    }
    emit choicesChanged(choices_);
}
// Beside the rail the sidebar holds the current section's pages; without it, every page grouped by section.
void Workspace::refreshNavigation() {
    const bool sidebarOn = choices_.sidebar != QStringLiteral("off");
    const bool tree = choices_.sidebar == QStringLiteral("tree");
    const bool allPages = !choices_.rail;
    const auto listed = [&](int page) { return allPages ? listedSection(pageSections_[page]) : pageSections_[page] == currentSection_; };
    {
        QSignalBlocker blocker(navigation_);
        navigation_->clear();
        if (sidebarOn && !tree) for (int i = 0; i < pageSections_.size(); ++i) if (listed(i)) {
            auto *item = new QListWidgetItem(pageTitles_[i], navigation_); item->setData(Qt::UserRole, i);
        }
    }
    {
        QSignalBlocker blocker(tree_);
        tree_->clear();
        if (sidebarOn && tree) for (int section = 0; section < sectionSelections_.size(); ++section) {
            if (allPages ? !listedSection(section) : section != currentSection_) continue;
            QList<QPair<QString, QString>> items;
            for (int i = 0; i < pageSections_.size(); ++i) if (pageSections_[i] == section) items.append({QString::number(i), pageTitles_[i]});
            if (!items.isEmpty()) tree_->addGroup(QString::number(section), rail_->item(section)->toolTip(), items);
        }
    }
    navigation_->setVisible(sidebarOn && !tree);
    tree_->setVisible(sidebarOn && tree);
    const QString title = allPages ? QStringLiteral("Pages") : currentSection_ >= 0 ? rail_->item(currentSection_)->toolTip() : QString();
    contextHeading_->setText(title);
    navigation_->setAccessibleName(title); tree_->setAccessibleName(title);
    refreshTabRow();
    syncNavigation();
}
// The frame's one tab row: open documents beside a sidebar, or the pages themselves when there is no sidebar.
void Workspace::refreshTabRow() {
    const bool sidebarOn = choices_.sidebar != QStringLiteral("off");
    const bool documents = sidebarOn && choices_.documentTabs;
    QList<int> wanted;
    if (documents) wanted = openPages_;
    else if (!sidebarOn) for (int i = 0; i < pageSections_.size(); ++i)
        if (choices_.rail ? pageSections_[i] == currentSection_ : listedSection(pageSections_[i])) wanted << i;
    QList<int> shown;
    for (int tab = 0; tab < pageTabs_->count(); ++tab) shown << pageTabs_->tabData(tab).toInt();
    if (shown != wanted || pageTabs_->tabsClosable() != documents) {
        QSignalBlocker blocker(pageTabs_);
        while (pageTabs_->count()) pageTabs_->removeTab(0);
        pageTabs_->setTabsClosable(documents);
        for (int page : wanted) {
            const int at = pageTabs_->addTab(pageTitles_[page]);
            pageTabs_->setTabData(at, page);
            if (!documents) continue;
            // Closing is immediate, with the library's quiet close icon instead of the platform's button.
            auto *close = new IconButton(QStringLiteral("close"), QStringLiteral("Close ") + pageTitles_[page]);
            close->setProperty("sjVariant", "quiet"); close->setProperty("sjRole", "tabClose"); close->setFixedSize(18, 18); close->setIconSize(QSize(12, 12));
            close->setFocusPolicy(Qt::NoFocus);
            connect(close, &QToolButton::clicked, this, [this, page] { closeDocument(page); });
            pageTabs_->setTabButton(at, QTabBar::RightSide, close);
        }
    }
    const bool row = documents || !sidebarOn;
    tabRow_->setVisible(row);
    // A page must not add a second tab row under the frame's: hide a tabbed page's own row while the frame shows one.
    for (int i = 0; i < pages_->count(); ++i) if (auto *tabs = qobject_cast<QTabWidget *>(pages_->widget(i))) tabs->tabBar()->setVisible(!row);
    updateOpenState();
}
void Workspace::syncNavigation() {
    const int page = pages_->currentIndex();
    const bool shown = !(choices_.documentTabs && choices_.sidebar != QStringLiteral("off") && openPages_.isEmpty());
    {
        QSignalBlocker blocker(navigation_);
        int match = -1;
        for (int row = 0; row < navigation_->count(); ++row) if (navigation_->item(row)->data(Qt::UserRole).toInt() == page) match = row;
        if (match >= 0 && shown) navigation_->setCurrentRow(match);
        else { navigation_->setCurrentRow(-1); navigation_->clearSelection(); }
    }
    if (shown) tree_->setCurrentId(QString::number(page));
    else { QSignalBlocker blocker(tree_); tree_->setCurrentItem(nullptr); tree_->clearSelection(); }
    QSignalBlocker blocker(pageTabs_);
    for (int tab = 0; tab < pageTabs_->count(); ++tab) if (pageTabs_->tabData(tab).toInt() == page) pageTabs_->setCurrentIndex(tab);
}
void Workspace::openDocument(int page) {
    if (page < 0 || openPages_.contains(page)) return;
    openPages_.append(page);
    refreshTabRow();
}
void Workspace::closeDocument(int page) {
    const int at = openPages_.indexOf(page);
    if (at < 0) return;
    openPages_.removeAt(at);
    refreshTabRow();
    if (page == currentPage() && !openPages_.isEmpty()) setCurrentPage(openPages_[qMin(at, openPages_.size() - 1)]);
    else syncNavigation();
}
void Workspace::updateOpenState() {
    const bool empty = choices_.documentTabs && choices_.sidebar != QStringLiteral("off") && openPages_.isEmpty();
    emptyPages_->setVisible(empty);
    pages_->setVisible(!empty);
}
// Fixed sizes come from the density tokens, so they are set again when the density changes.
void Workspace::applyDensity() {
    const int railWidth = Theme::density(QStringLiteral("railWidth"), 48);
    const int context = Theme::density(QStringLiteral("contextHeight"), 36);
    toolbar_->setMinimumHeight(Theme::density(QStringLiteral("headerHeight"), 40));
    rail_->setFixedWidth(railWidth);
    collapse_->setFixedSize(railWidth - 8, context);
    settingsButton_->setFixedSize(railWidth - 8, context);
    contextHeading_->setMinimumHeight(context);
    for (int i = 0; i < rail_->count(); ++i) rail_->item(i)->setSizeHint(QSize(railWidth - Theme::spacing(1), context));
    // Content areas mark themselves with sjContentPadding so their margins follow the density too.
    const int padding = Theme::density(QStringLiteral("contentPadding"), 20);
    for (auto *widget : findChildren<QWidget *>()) {
        if (!widget->property("sjContentPadding").toBool()) continue;
        if (widget->layout()) widget->layout()->setContentsMargins(padding, padding, padding, padding);
        else widget->setContentsMargins(padding, padding, padding, padding);
    }
}
void Workspace::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
}
AppShell::AppShell(const QString &name, const QString &description, const QString &version, QWidget *parent)
    : Workspace(parent), settings_(new SettingsWindow(this)), name_(name), description_(description), version_(version) {
    setHeader(name);
    setSettingsHandler([this] { finishSettings(); settings_->show(); settings_->raise(); });
    auto *open = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma), this);
    connect(open, &QShortcut::activated, this, [this] { finishSettings(); settings_->show(); settings_->raise(); });
}
void AppShell::addSettingsSection(const QString &label, QWidget *content, const QString &keywords) { settings_->addSection(label, content, keywords); }
void AppShell::setDark(bool dark) {
    dark_ = dark;
    if (mode_) { QSignalBlocker blocker(mode_); mode_->setCurrentIndex(dark ? 0 : 1); }
    const auto theme = Theme::load(QStringLiteral("clean"), dark);
    theme.apply(this);
}
void AppShell::setPreset(const QString &presetId, const QVariantMap &overrides) {
    preset_ = Preset::find(presetId).id;
    const auto choices = Preset::resolve(preset_, overrides);
    disconnect(followSystem_);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (choices.theme == QStringLiteral("system"))
        followSystem_ = connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this] { setDark(systemPrefersDark()); });
#endif
    setChoices(choices);
    setDark(prefersDark(choices.theme));
}
void AppShell::showEvent(QShowEvent *event) { finishSettings(); Workspace::showEvent(event); }
void AppShell::finishSettings() {
    if (settingsFinished_) return;
    settingsFinished_ = true;
    auto *appearance = new SettingsPage(QStringLiteral("Appearance"));
    auto *mode = new QComboBox; mode->setAccessibleName(QStringLiteral("Color theme"));
    mode->addItems({QStringLiteral("Dark"), QStringLiteral("Light")}); mode->setCurrentIndex(dark_ ? 0 : 1);
    mode_ = mode;
    // Picking a theme here ends following the device's color scheme.
    connect(mode, &QComboBox::currentIndexChanged, this, [this](int index) { disconnect(followSystem_); setDark(index == 0); });
    appearance->contentLayout()->addWidget(new Field(QStringLiteral("Theme"), mode, QStringLiteral("Choose the light or dark appearance.")));
    settings_->addSection(QStringLiteral("Appearance"), appearance, QStringLiteral("theme dark light mode color"));
    settings_->addSection(QStringLiteral("About"), new AboutPage(name_, description_, version_), QStringLiteral("version copyright"));
}
QLabel *heading(const QString &text, QWidget *parent) {
    auto *label = new QLabel(text, parent);
    label->setProperty("sjRole", "heading");
    label->setWordWrap(true);
    return label;
}
QLabel *paragraph(const QString &text, QWidget *parent) {
    auto *label = new QLabel(text, parent);
    label->setWordWrap(true);
    label->setTextFormat(Qt::PlainText);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return label;
}
QWidget *row(const QList<QWidget *> &widgets, QWidget *parent) {
    auto *result = new QWidget(parent);
    auto *layout = new QHBoxLayout(result);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::spacing(1));
    for (auto *widget : widgets) layout->addWidget(widget);
    layout->addStretch();
    return result;
}
QWidget *wrap(const QList<QWidget *> &widgets, QWidget *parent) {
    auto *result = new QWidget(parent);
    auto *layout = new WrapLayout(result);
    for (auto *widget : widgets) layout->addWidget(widget);
    return result;
}
}
