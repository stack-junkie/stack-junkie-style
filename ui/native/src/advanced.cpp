#include <stackjunkie/components.hpp>
#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QKeyEvent>
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
GroupedTabs::GroupedTabs(QWidget *parent) : QWidget(parent), groups_(new Tabs(this)) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    groups_->setAccessibleName(QStringLiteral("Document groups"));
    layout->addWidget(groups_);
}
int GroupedTabs::addGroup(const QString &name) {
    auto *tabs = new Tabs(groups_);
    tabs->setMovable(true);
    tabs->setAccessibleName(name + QStringLiteral(" documents"));
    return groups_->addTab(tabs, name);
}
int GroupedTabs::addTab(int group, const QString &title, QWidget *page) {
    auto *tabs = qobject_cast<QTabWidget *>(groups_->widget(group));
    return tabs ? tabs->addTab(page, title) : -1;
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
}
