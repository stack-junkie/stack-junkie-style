#include <stackjunkie/components.hpp>
#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QListWidget>
#include <QStackedWidget>
#include <QTabBar>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <QSlider>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace StackJunkie {
static Theme currentTheme(const QWidget *widget) {
    for (auto *scope = widget; scope; scope = scope->parentWidget())
        if (scope->property("sjDirection").isValid()) return Theme::load(scope->property("sjDirection").toString(), scope->property("sjDark").toBool());
    return Theme::load();
}
GroupedTabs::GroupedTabs(QWidget *parent): Tabs(parent) {
    setAccessibleName(QStringLiteral("Document groups"));
    // A group chip is a tab in the same row. Choosing it collapses or expands the group and keeps the previous tab selected.
    connect(tabBar(), &QTabBar::tabBarClicked, this, [this](int index) {
        if (rebuilding_ || index < 0) return;
        const QVariant group = tabBar()->tabData(index);
        if (!group.isValid()) return;
        setGroupCollapsed(group.toInt(), !groups_[group.toInt()].collapsed);
    });
    connect(this, &QTabWidget::currentChanged, this, [this](int index) {
        if (rebuilding_ || index < 0 || !tabBar()->tabData(index).isValid()) return;
        for (int i = index + 1; i < count(); ++i) if (!tabBar()->tabData(i).isValid()) { setCurrentIndex(i); return; }
        for (int i = index - 1; i >= 0; --i) if (!tabBar()->tabData(i).isValid()) { setCurrentIndex(i); return; }
    });
}
int GroupedTabs::addGroup(const QString &name) { groups_.append({name, false, {}}); rebuild(); return groups_.size() - 1; }
int GroupedTabs::addTab(int group, const QString &title, QWidget *page) {
    if (group < 0 || group >= groups_.size()) return -1;
    groups_[group].tabs.append({title, page}); rebuild(); return indexOf(page);
}
void GroupedTabs::setGroupCollapsed(int group, bool collapsed) {
    if (group < 0 || group >= groups_.size()) return;
    groups_[group].collapsed = collapsed; rebuild();
}
void GroupedTabs::rebuild() {
    rebuilding_ = true;
    QWidget *current = currentWidget();
    while (count()) { QWidget *page = widget(0); removeTab(0); if (page && page->property("sjGroupChip").toBool()) page->deleteLater(); }
    const QColor accent = currentTheme(this).color(QStringLiteral("accent"));
    for (int g = 0; g < groups_.size(); ++g) {
        auto *chip = new QWidget; chip->setProperty("sjGroupChip", true);
        const int at = QTabWidget::addTab(chip, groups_[g].name);
        tabBar()->setTabData(at, g); tabBar()->setTabTextColor(at, accent);
        tabBar()->setTabIcon(at, icon(groups_[g].collapsed ? QStringLiteral("chevron-right") : QStringLiteral("chevron-down"), accent, 14));
        tabBar()->setTabToolTip(at, (groups_[g].collapsed ? QStringLiteral("Show ") : QStringLiteral("Collapse ")) + groups_[g].name);
        for (const auto &[title, page] : groups_[g].tabs) if (!groups_[g].collapsed || page == current) QTabWidget::addTab(page, title);
    }
    rebuilding_ = false;
    const int keep = current ? indexOf(current) : -1;
    if (keep >= 0) setCurrentIndex(keep);
    else for (int i = 0; i < count(); ++i) if (!tabBar()->tabData(i).isValid()) { setCurrentIndex(i); break; }
}
TabBoundary::TabBoundary(QWidget *content, QWidget *parent): QFrame(parent) {
    setProperty("sjRole", "panel"); setProperty("sjTabBoundary", true);
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(Theme::spacing(2), Theme::spacing(2), Theme::spacing(2), Theme::spacing(2));
    layout->addWidget(content);
}
NavTree::NavTree(const QString &label, QWidget *parent): QTreeWidget(parent) {
    setProperty("sjRole", "navTree"); setAccessibleName(label);
    setHeaderHidden(true); setRootIsDecorated(true); setIndentation(14); setUniformRowHeights(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem *item) {
        if (item && item->parent()) emit pageChosen(item->data(0, Qt::UserRole).toString());
    });
    connect(this, &QTreeWidget::itemClicked, this, [](QTreeWidgetItem *item) { if (!item->parent()) item->setExpanded(!item->isExpanded()); });
}
void NavTree::addGroup(const QString &id, const QString &label, const QList<QPair<QString, QString>> &items) {
    auto *group = new QTreeWidgetItem(this, {QStringLiteral("%1   %2").arg(label).arg(items.size())});
    group->setData(0, Qt::UserRole, id); group->setFlags(Qt::ItemIsEnabled);
    for (const auto &[itemId, itemLabel] : items) { auto *item = new QTreeWidgetItem(group, {itemLabel}); item->setData(0, Qt::UserRole, itemId); }
    group->setExpanded(true);
}
void NavTree::setCurrentId(const QString &id) {
    QSignalBlocker blocker(this);
    for (int g = 0; g < topLevelItemCount(); ++g) for (int i = 0; i < topLevelItem(g)->childCount(); ++i)
        if (topLevelItem(g)->child(i)->data(0, Qt::UserRole).toString() == id) { setCurrentItem(topLevelItem(g)->child(i)); topLevelItem(g)->setExpanded(true); return; }
}
SettingsWindow::SettingsWindow(QWidget *parent): QDialog(parent), search_(new SearchField(QStringLiteral("Search settings"))), list_(new QListWidget), pages_(new QStackedWidget) {
    setWindowTitle(QStringLiteral("Settings")); setModal(true); resize(760, 520);
    auto *layout = new QHBoxLayout(this); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(0);
    auto *nav = new QFrame; nav->setProperty("sjRole", "settingsNav"); nav->setFixedWidth(200);
    auto *navLayout = new QVBoxLayout(nav); navLayout->setContentsMargins(Theme::spacing(1), Theme::spacing(1), Theme::spacing(1), Theme::spacing(1));
    search_->setPlaceholderText(QStringLiteral("Search settings"));
    list_->setProperty("sjRole", "navigation"); list_->setAccessibleName(QStringLiteral("Settings sections"));
    navLayout->addWidget(search_); navLayout->addWidget(list_, 1);
    layout->addWidget(nav); layout->addWidget(pages_, 1);
    connect(list_, &QListWidget::currentRowChanged, this, [this](int row) { if (auto *item = list_->item(row)) pages_->setCurrentIndex(item->data(Qt::UserRole).toInt()); });
    connect(search_, &QLineEdit::textChanged, this, &SettingsWindow::filter);
}
void SettingsWindow::addSection(const QString &label, QWidget *content, const QString &keywords) {
    const int index = pages_->addWidget(content);
    auto *item = new QListWidgetItem(label, list_); item->setData(Qt::UserRole, index);
    keywords_.append(label + QStringLiteral(" ") + keywords);
    if (list_->count() == 1) list_->setCurrentRow(0);
}
void SettingsWindow::showSection(int index) { if (index >= 0 && index < list_->count()) list_->setCurrentRow(index); }
void SettingsWindow::filter(const QString &text) {
    const QString needle = text.trimmed();
    int first = -1;
    for (int row = 0; row < list_->count(); ++row) {
        const bool match = needle.isEmpty() || keywords_[row].contains(needle, Qt::CaseInsensitive);
        list_->item(row)->setHidden(!match);
        if (match && first < 0) first = row;
    }
    if (list_->currentItem() && list_->currentItem()->isHidden() && first >= 0) list_->setCurrentRow(first);
}
Drawer::Drawer(QWidget *host, const QString &title, QWidget *content, Edge edge) : QFrame(host), edge_(edge) {
    setProperty("sjRole", "panel");
    setAccessibleName(title);
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(Theme::spacing(3), Theme::spacing(3), Theme::spacing(3), Theme::spacing(3));
    auto *close = new IconButton(QStringLiteral("close"), QStringLiteral("Close ") + title, this);
    layout->addWidget(row({heading(title, this), close}, this));
    layout->addWidget(content, 1);
    connect(close, &QToolButton::clicked, this, &Drawer::dismiss);
    qApp->installEventFilter(this);
    hide();
}
Drawer::~Drawer() { qApp->removeEventFilter(this); }
void Drawer::place() {
    if (!parentWidget()) return;
    const QRect bounds = parentWidget()->rect();
    const int paneWidth = qMin(380, qMax(240, bounds.width() - 32));
    setGeometry(edge_ == Edge::Left ? bounds.left() : bounds.right() - paneWidth + 1, bounds.top(), paneWidth, bounds.height());
}
void Drawer::open() {
    previousFocus_ = QApplication::focusWidget();
    currentTheme(parentWidget()).apply(this);
    place(); show(); raise(); setFocus(Qt::OtherFocusReason);
}
void Drawer::dismiss() { hide(); if (previousFocus_) previousFocus_->setFocus(Qt::OtherFocusReason); }
void Drawer::setEdge(Edge edge) { edge_ = edge; if (isVisible()) place(); }
bool Drawer::eventFilter(QObject *object, QEvent *event) {
    if (object == parentWidget() && event->type() == QEvent::Resize && isVisible()) place();
    if (!isVisible()) return QFrame::eventFilter(object, event);
    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape) { dismiss(); return true; }
    if (event->type() == QEvent::MouseButtonPress) {
        auto *target = qobject_cast<QWidget *>(object);
        if (target && target != this && !isAncestorOf(target)) dismiss();
    }
    return QFrame::eventFilter(object, event);
}
Chart::Chart(QWidget *parent) : QWidget(parent) {
    setMinimumSize(320, 240);
    setAccessibleName(QStringLiteral("Chart"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}
void Chart::setData(const QList<QPair<QString, double>> &values, const QString &unit) {
    values_ = values; unit_ = unit;
    QStringList alternatives;
    for (const auto &value : values_) alternatives << value.first + QStringLiteral(": ") + QString::number(value.second) + QStringLiteral(" ") + unit;
    setAccessibleDescription(alternatives.join(QStringLiteral("; ")));
    setToolTip(accessibleDescription());
    update();
}
void Chart::setMode(Mode mode) { mode_ = mode; update(); }
void Chart::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto theme = currentTheme(this);
    painter.fillRect(rect(), theme.color(QStringLiteral("surface")));
    const QRectF plot(36, 30, qMax(100, width() - 52), qMax(90, height() - 92));
    painter.setPen(QPen(theme.color(QStringLiteral("border")), 1));
    painter.drawLine(plot.bottomLeft(), plot.bottomRight());
    painter.drawLine(plot.topLeft(), plot.bottomLeft());
    if (values_.isEmpty()) {
        painter.setPen(theme.color(QStringLiteral("muted")));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("No chart data"));
        return;
    }
    double maximum = 1;
    double minimum = 0;
    for (const auto &value : values_) { maximum = std::max(maximum, value.second); minimum = std::min(minimum, value.second); }
    const double span = maximum - minimum;
    const qreal zeroY = plot.bottom() + minimum / span * plot.height();
    const qreal step = plot.width() / values_.size();
    QPainterPath line;
    QList<QPointF> points;
    for (int i = 0; i < values_.size(); ++i) {
        const auto &value = values_[i];
        const qreal x = plot.left() + step * (i + 0.5);
        const qreal y = plot.bottom() - (value.second - minimum) / span * plot.height();
        points << QPointF(x, y);
        if (!i) line.moveTo(x, y); else line.lineTo(x, y);
        if (mode_ == Mode::Bars) painter.fillRect(QRectF(x - step * 0.26, qMin(y, zeroY), step * 0.52, qMax(1.0, qAbs(y - zeroY))), theme.color(QStringLiteral("accent")));
        painter.setPen(theme.color(QStringLiteral("text")));
        painter.drawText(QRectF(x - step / 2, plot.bottom() + 8, step, 42), Qt::AlignHCenter | Qt::TextWordWrap, value.first);
        painter.drawText(QRectF(x - step / 2, qMax(4.0, y - 25), step, 22), Qt::AlignCenter, QString::number(value.second) + QStringLiteral(" ") + unit_);
    }
    if (mode_ == Mode::Line) {
        painter.setPen(QPen(theme.color(QStringLiteral("accent")), 2)); painter.drawPath(line);
        painter.setBrush(theme.color(QStringLiteral("accent")));
        for (const auto &point : points) painter.drawEllipse(point, 4, 4);
    }
}
Canvas::Canvas(QWidget *parent) : QWidget(parent), scene_(new QGraphicsScene(this)), view_(new QGraphicsView(scene_, this)), selection_(new QComboBox(this)), x_(new QDoubleSpinBox(this)), y_(new QDoubleSpinBox(this)) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::spacing(2));
    selection_->setAccessibleName(QStringLiteral("Selected canvas object"));
    for (auto *value : {x_, y_}) { value->setRange(-10000, 10000); value->setDecimals(1); }
    layout->addWidget(row({new Field(QStringLiteral("Selection"), selection_), new Field(QStringLiteral("X"), x_), new Field(QStringLiteral("Y"), y_)}));
    scene_->setSceneRect(0, 0, 420, 210);
    view_->setAccessibleName(QStringLiteral("Editable canvas"));
    view_->setMinimumHeight(250);
    view_->setRenderHint(QPainter::Antialiasing);
    view_->viewport()->installEventFilter(this);
    layout->addWidget(view_);
    auto *zoom = new QSlider(Qt::Horizontal, this);
    zoom->setRange(25, 300); zoom->setValue(100); zoom->setAccessibleName(QStringLiteral("Canvas zoom percent"));
    auto *percent = new QLabel(QStringLiteral("100%"), this);
    layout->addWidget(row({new Field(QStringLiteral("Zoom"), zoom), percent}));
    connect(zoom, &QSlider::valueChanged, this, [this, percent](int value) { setZoomPercent(value); percent->setText(QString::number(value) + QStringLiteral("%")); });
    connect(selection_, &QComboBox::currentIndexChanged, this, [this](int) {
        const QString chosen = selection_->currentText();
        {
            const QSignalBlocker blocker(scene_);
            for (auto *item : scene_->items()) item->setSelected(item->data(1).toString() == chosen);
        }
        synchronizeSelection();
    });
    connect(x_, &QDoubleSpinBox::valueChanged, this, [this](double value) { for (auto *item : scene_->selectedItems()) item->setX(value); });
    connect(y_, &QDoubleSpinBox::valueChanged, this, [this](double value) { for (auto *item : scene_->selectedItems()) item->setY(value); });
    connect(scene_, &QGraphicsScene::selectionChanged, this, &Canvas::synchronizeSelection);
}
void Canvas::addRectangle(const QString &name, const QRectF &bounds, const QString &colorRole) {
    auto *item = scene_->addRect(QRectF(QPointF(), bounds.size()));
    item->setPos(bounds.topLeft()); item->setData(0, colorRole); item->setData(1, name);
    item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    selection_->addItem(name); applyTheme(currentTheme(this));
    if (selection_->count() == 1) item->setSelected(true);
}
void Canvas::addEllipse(const QString &name, const QRectF &bounds, const QString &colorRole) {
    auto *item = scene_->addEllipse(QRectF(QPointF(), bounds.size()));
    item->setPos(bounds.topLeft()); item->setData(0, colorRole); item->setData(1, name);
    item->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    selection_->addItem(name); applyTheme(currentTheme(this));
    if (selection_->count() == 1) item->setSelected(true);
}
void Canvas::setZoomPercent(int percent) { view_->resetTransform(); view_->scale(qBound(25, percent, 300) / 100.0, qBound(25, percent, 300) / 100.0); }
void Canvas::applyTheme(const Theme &theme) {
    scene_->setBackgroundBrush(theme.color(QStringLiteral("surface")));
    for (auto *item : scene_->items()) if (auto *shape = dynamic_cast<QAbstractGraphicsShapeItem *>(item)) {
        const auto color = theme.color(item->data(0).toString());
        QColor fill = color; fill.setAlpha(95);
        shape->setPen(QPen(color, 2)); shape->setBrush(fill);
    }
}
QString Canvas::selectedName() const { return scene_->selectedItems().isEmpty() ? QString() : scene_->selectedItems().first()->data(1).toString(); }
QPointF Canvas::selectedPosition() const { return scene_->selectedItems().isEmpty() ? QPointF() : scene_->selectedItems().first()->pos(); }
void Canvas::synchronizeSelection() {
    const auto name = selectedName();
    const QSignalBlocker first(selection_), second(x_), third(y_);
    if (!name.isEmpty()) { selection_->setCurrentText(name); x_->setValue(selectedPosition().x()); y_->setValue(selectedPosition().y()); }
    x_->setEnabled(!name.isEmpty()); y_->setEnabled(!name.isEmpty());
    view_->setAccessibleDescription(name.isEmpty() ? QStringLiteral("No selected object") : name + QStringLiteral(" at %1, %2").arg(selectedPosition().x()).arg(selectedPosition().y()));
}
bool Canvas::eventFilter(QObject *object, QEvent *event) {
    if (object == view_->viewport() && (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::KeyRelease)) QTimer::singleShot(0, this, &Canvas::synchronizeSelection);
    return QWidget::eventFilter(object, event);
}
Stub::Stub(Site site, const QString &text, QWidget *parent): QFrame(parent) {
    auto *label = new QLabel(text); label->setWordWrap(true); label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(Theme::spacing(3), Theme::spacing(2), Theme::spacing(3), Theme::spacing(2));
    layout->addWidget(label);
    mark(site);
}
Stub::Stub(Site site, QWidget *content, QWidget *parent): QFrame(parent) {
    // A wrapped widget paints its own background, so the stripes show as a band around it.
    auto *layout = new QVBoxLayout(this); const int band = Theme::spacing(2);
    layout->setContentsMargins(band, band, band, band);
    if (content) layout->addWidget(content);
    mark(site);
}
void Stub::mark(Site site) {
    setProperty("sjRole", "stub");
    location_ = QFileInfo(QString::fromUtf8(site.file)).fileName() + QStringLiteral(":") + QString::number(site.line);
    setToolTip(QStringLiteral("Stub at %1. Release builds refuse to finish while it remains.").arg(location_));
    setAccessibleDescription(QStringLiteral("Stub, development stand-in content, at ") + location_);
}
void Stub::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto theme = currentTheme(this);
    const QColor ground = theme.color(QStringLiteral("surface"));
    QColor stripe = theme.color(QStringLiteral("warning")); stripe.setAlphaF(theme.dark ? 0.24 : 0.16);
    painter.fillRect(rect(), ground);
    painter.save();
    painter.setClipRect(rect());
    painter.setPen(QPen(stripe, 6, Qt::SolidLine, Qt::FlatCap));
    for (int x = -height(); x < width() + height(); x += 16) painter.drawLine(QPointF(x, height()), QPointF(x + height(), 0));
    painter.restore();
    painter.setPen(QPen(theme.color(QStringLiteral("warning")), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5));
}
}
