#include <stackjunkie/theme.hpp>
#include <stackjunkie/components.hpp>
#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QHelpEvent>
#include <QMenuBar>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QTabBar>
#include <QToolTip>
#include <QFile>
#include <QFontDatabase>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QListWidget>
#include <QPainter>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <algorithm>
#include <QPalette>
#include <QStyle>
#include <QWidget>
#include <stdexcept>

static void registerStackJunkieResources() { Q_INIT_RESOURCE(stackjunkie_resources); }

namespace StackJunkie {
static QHash<QString, QString> registeredFamilies;
static int currentZoom = 100;
static constexpr int zoomSteps[] = {50, 67, 75, 80, 90, 100, 110, 125, 150, 175, 200};

// Shared pointer and tooltip behavior for every widget in the app.
// The pointing hand shows over enabled buttons, tabs, menu triggers and the rows of navigation lists and trees;
// disabled controls keep the arrow. Tooltips sit against their element: beside Rail items and anything marked
// sjTipSide "right", below buttons, instead of below the pointer.
class Interaction : public QObject {
public:
    using QObject::QObject;
protected:
    bool eventFilter(QObject *object, QEvent *event) override {
        auto *widget = qobject_cast<QWidget *>(object);
        if (!widget) return false;
        switch (event->type()) {
        case QEvent::Polish: case QEvent::EnabledChange: updateCursor(widget); break;
        case QEvent::MouseMove: case QEvent::HoverMove: if (auto *view = pointerView(widget)) updateRowCursor(view, static_cast<QMouseEvent *>(event)->position().toPoint()); break;
        case QEvent::ToolTip: return placeTip(widget, static_cast<QHelpEvent *>(event));
        default: break;
        }
        return false;
    }
private:
    static bool clickable(QWidget *widget) {
        return qobject_cast<QAbstractButton *>(widget) || qobject_cast<QTabBar *>(widget) || qobject_cast<QMenuBar *>(widget);
    }
    // A navigation list or tree whose rows are links: the rail, the sidebar, the Settings sections.
    static QAbstractItemView *pointerView(QWidget *viewport) {
        auto *view = qobject_cast<QAbstractItemView *>(viewport->parentWidget());
        return view && view->viewport() == viewport && !view->property("sjRole").toString().isEmpty()
            && QStringList{QStringLiteral("rail"), QStringLiteral("navigation"), QStringLiteral("navTree")}.contains(view->property("sjRole").toString()) ? view : nullptr;
    }
    static void updateCursor(QWidget *widget) {
        if (auto *view = pointerView(widget)) { widget->setMouseTracking(true); widget->setAttribute(Qt::WA_Hover); Q_UNUSED(view); return; }
        if (!clickable(widget)) return;
        if (widget->isEnabled()) widget->setCursor(Qt::PointingHandCursor); else widget->unsetCursor();
    }
    static void updateRowCursor(QAbstractItemView *view, const QPoint &at) {
        const auto index = view->indexAt(at);
        if (index.isValid() && (index.flags() & Qt::ItemIsEnabled) && view->isEnabled()) view->viewport()->setCursor(Qt::PointingHandCursor);
        else view->viewport()->unsetCursor();
    }
    static bool placeTip(QWidget *widget, QHelpEvent *help) {
        // Qt places a tooltip 2 pixels right of and 16 below the point it is given; these subtract that offset.
        const QPoint offset(2, 16);
        if (auto *view = pointerView(widget)) {
            const auto index = view->indexAt(help->pos());
            const QString text = index.data(Qt::ToolTipRole).toString();
            if (!index.isValid() || text.isEmpty()) { QToolTip::hideText(); return true; }
            const QRect rect = view->visualRect(index);
            if (view->property("sjTipSide").toString() != QStringLiteral("right")) return false;
            const int tipHeight = QFontMetrics(QToolTip::font()).height() + 10;
            QToolTip::showText(widget->mapToGlobal(QPoint(rect.right() + 6, rect.center().y() - tipHeight / 2)) - offset, text, widget, rect);
            return true;
        }
        if (!qobject_cast<QAbstractButton *>(widget) || widget->toolTip().isEmpty()) return false;
        const bool right = widget->property("sjTipSide").toString() == QStringLiteral("right");
        const int tipHeight = QFontMetrics(QToolTip::font()).height() + 10;
        const QPoint at = right ? QPoint(widget->width() + 6, widget->height() / 2 - tipHeight / 2) : QPoint(0, widget->height() + 4);
        QToolTip::showText(widget->mapToGlobal(at) - offset, widget->toolTip(), widget, widget->rect());
        return true;
    }
};

void initializeResources() {
    static bool initialized = false;
    if (!initialized) {
        registerStackJunkieResources();
        for (const auto &font : {"Manrope", "FiraCode"}) {
            const QString fileName = QString::fromLatin1(font);
            const int id = QFontDatabase::addApplicationFont(QStringLiteral(":/stackjunkie/fonts/%1.ttf").arg(fileName));
            const auto families = QFontDatabase::applicationFontFamilies(id);
            if (families.isEmpty()) throw std::runtime_error("Cannot register an embedded Stack Junkie font");
            const QString tokenName = fileName == QStringLiteral("FiraCode") ? QStringLiteral("Fira Code") : fileName;
            // Basic/offscreen and platform font databases can expose different family
            // names for variable fonts. Use the actual registered family on each host.
            registeredFamilies.insert(tokenName, families.contains(tokenName) ? tokenName : families.first());
        }
        initialized = true;
    }
    // Once there is an application: the style's UI font for every window, including menus, dialogs and tooltips,
    // which do not inherit a font from the window that opens them, and the shared pointer and tooltip behavior.
    static bool appReady = false;
    if (!appReady && qobject_cast<QApplication *>(QCoreApplication::instance())) {
        appReady = true;
        const Theme theme = Theme::load();
        QFont font(theme.font(QStringLiteral("ui")));
        font.setPixelSize(theme.metric(QStringLiteral("fontSize"), 14));
        QApplication::setFont(font);
        QToolTip::setFont(font);
        qApp->installEventFilter(new Interaction(qApp));
    }
}
void Theme::setZoom(int percent) { currentZoom = qBound(zoomSteps[0], percent, zoomSteps[std::size(zoomSteps) - 1]); }
int Theme::globalZoom() { return currentZoom; }
int Theme::zoomStep(int current, int direction) {
    if (direction > 0) { for (int step : zoomSteps) if (step > current) return step; return zoomSteps[std::size(zoomSteps) - 1]; }
    for (int i = int(std::size(zoomSteps)) - 1; i >= 0; --i) if (zoomSteps[i] < current) return zoomSteps[i];
    return zoomSteps[0];
}
Theme Theme::zoomed(int percent) const { Theme copy = *this; copy.zoom = qBound(zoomSteps[0], percent, zoomSteps[std::size(zoomSteps) - 1]); return copy; }
int Theme::scaled(int pixels) const { return pixels == 0 ? 0 : qMax(1, (pixels * zoom + 50) / 100); }
static int globalScaled(int pixels) { return pixels == 0 ? 0 : qMax(1, (pixels * currentZoom + 50) / 100); }
static QJsonObject document(const QString &file) {
    initializeResources();
    QFile input(QStringLiteral(":/stackjunkie/") + file);
    if (!input.open(QIODevice::ReadOnly)) throw std::runtime_error("Missing embedded Stack Junkie resource");
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(input.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) throw std::runtime_error("Invalid embedded Stack Junkie JSON");
    return doc.object();
}
Theme Theme::load(const QString &direction, bool dark) {
    Theme result;
    const auto choices = document(QStringLiteral("tokens.json")).value(QStringLiteral("directions")).toObject();
    if (!choices.contains(direction)) throw std::runtime_error("Unknown Stack Junkie design direction");
    result.direction = direction;
    result.dark = dark;
    result.definition = choices.value(direction).toObject();
    result.colors = result.definition.value(dark ? QStringLiteral("dark") : QStringLiteral("light")).toObject();
    result.zoom = currentZoom;
    return result;
}
QStringList Theme::directions() { return document(QStringLiteral("tokens.json")).value(QStringLiteral("directions")).toObject().keys(); }
QJsonObject Theme::catalog() { return document(QStringLiteral("catalog.json")); }
static int rawSpacing(int index) {
    const auto values = document(QStringLiteral("tokens.json")).value(QStringLiteral("spacing")).toArray();
    return index >= 0 && index < values.size() ? values.at(index).toInt(8) : 8;
}
static int rawTypeSize(const QString &role, int fallback) { return document(QStringLiteral("tokens.json")).value(QStringLiteral("typeScale")).toObject().value(role).toInt(fallback); }
int Theme::spacing(int index) { return globalScaled(rawSpacing(index)); }
int Theme::typeSize(const QString &role, int fallback) { return globalScaled(rawTypeSize(role, fallback)); }
static QString currentDensity = QStringLiteral("compact");
void Theme::setDensityMode(const QString &mode) { currentDensity = mode == QStringLiteral("comfortable") ? mode : QStringLiteral("compact"); }
QString Theme::densityMode() { return currentDensity; }
// The density tokens are compact. Their comfortable block lists only the values that change.
static QJsonValue comfortableValue(const QJsonObject &density, const QString &role) {
    if (currentDensity != QStringLiteral("comfortable")) return {};
    return density.value(QStringLiteral("comfortable")).toObject().value(role);
}
static int rawDensity(const QString &role, int fallback) {
    const auto values = document(QStringLiteral("tokens.json")).value(QStringLiteral("density")).toObject();
    const auto comfortable = comfortableValue(values, role);
    return comfortable.isDouble() ? comfortable.toInt(fallback) : values.value(role).toInt(fallback);
}
int Theme::density(const QString &role, int fallback) { return globalScaled(rawDensity(role, fallback)); }
qreal Theme::typography(const QString &role, qreal fallback) { return document(QStringLiteral("tokens.json")).value(QStringLiteral("typography")).toObject().value(role).toDouble(fallback); }
int Theme::motionDuration(const QString &role) { return document(QStringLiteral("tokens.json")).value(QStringLiteral("motion")).toObject().value(role).toInt(0); }
QColor Theme::color(const QString &name) const { return QColor(colors.value(name).toString()); }
int Theme::metric(const QString &name, int fallback) const {
    if (name == QStringLiteral("controlHeight")) {
        const auto comfortable = comfortableValue(document(QStringLiteral("tokens.json")).value(QStringLiteral("density")).toObject(), name);
        if (comfortable.isDouble()) return comfortable.toInt(fallback);
    }
    return definition.value(QStringLiteral("metrics")).toObject().value(name).toInt(fallback);
}
QString Theme::font(const QString &role) const {
    initializeResources();
    const QString token = definition.value(QStringLiteral("fonts")).toObject().value(role).toString(QStringLiteral("Manrope"));
    if (token == QStringLiteral("system-ui")) {
        const auto available = QFontDatabase::families();
        for (const auto &preferred : {QStringLiteral("Segoe UI Variable Text"), QStringLiteral("Segoe UI")})
            if (available.contains(preferred, Qt::CaseInsensitive)) return preferred;
        const auto general = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
        if (available.contains(general, Qt::CaseInsensitive) && !QFontDatabase::isFixedPitch(general)) return general;
        return registeredFamilies.value(QStringLiteral("Manrope"), QStringLiteral("sans-serif"));
    }
    return registeredFamilies.value(token, token);
}
QString Theme::label() const { return definition.value(QStringLiteral("label")).toString(direction); }
QString Theme::styleSheet() const {
    QString css = QString::fromUtf8(R"CSS(
QWidget { color: @text; font-family: "@ui"; font-size: @baseFontpx; font-weight: @regularWeight; selection-background-color: @selection; selection-color: @text; }
QWidget[sjFontRole="editorial"] { font-family: "@editorial"; font-size: @readingpx; }
QWidget[sjFontRole="mono"] { font-family: "@mono"; font-size: @codeSizepx; }
QWidget[sjRoot="true"] { background: @canvas; }
QLabel { background: transparent; }
QLabel[sjRole="heading"] { font-size: @sectionpx; font-weight: @strongWeight; }
QTextBrowser[sjRole="readingSurface"] { border: 0; padding: 0; background: @surface; }
QLabel[sjRole="shellTitle"] { font-size: @labelSizepx; font-weight: @mediumWeight; }
QLabel[sjRole="contextTitle"] { background: @surface; font-size: @labelSizepx; font-weight: @strongWeight; padding: @controlPaddingYpx @controlPaddingXpx; border-bottom: @metricBorderWidthpx solid @border; }
QWidget[sjRole="shellHeader"] { background: @surface; border-bottom: @metricBorderWidthpx solid @border; }
QWidget[sjRole="shellHeader"] QComboBox, QWidget[sjRole="shellHeader"] QPushButton, QWidget[sjRole="shellHeader"] QLabel[sjRole="muted"] { font-size: @chromepx; }
QLabel[sjRole="muted"] { color: @muted; }
QLabel[sjRole="attribution"] { color: @muted; font-size: @captionpx; }
QFrame[sjRole="separator"] { border: 0; max-height: 1px; background: @border; }
QLabel[sjRole="productTitle"] { font-size: @headingpx; font-weight: @strongWeight; }
QLabel[sjRole="error"] { color: @danger; }
QPushButton, QToolButton { background: @surface; border: @metricBorderWidthpx solid @border; border-radius: @radiuspx; padding: @controlPaddingYpx @controlPaddingXpx; min-height: @innerHeightpx; }
QToolButton[sjRole="iconTarget"] { padding: 0; min-width: 0; min-height: 0; }
QToolButton[sjRole="railToggle"] { padding: 0; min-width: 0; min-height: @contextInnerHeightpx; max-height: @contextInnerHeightpx; }
QPushButton:hover, QToolButton:hover { background: @raised; border-color: @accent; }
QPushButton:pressed, QToolButton:pressed, QToolButton:checked { background: @selection; }
QPushButton:focus, QToolButton:focus, QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus, QTimeEdit:focus, QTextEdit:focus, QPlainTextEdit:focus { border: 2px solid @focus; }
QPushButton:checked { background: @selection; border-color: @accent; color: @accent; }
QPushButton[sjVariant="primary"] { color: @onAccent; background: @accent; border-color: @accent; font-weight: @mediumWeight; }
QPushButton[sjVariant="primary"]:hover { background: @accentHover; }
QPushButton[sjVariant="danger"] { color: @danger; border-color: @danger; }
QPushButton[sjVariant="quiet"], QToolButton[sjVariant="quiet"] { background: transparent; border-color: transparent; }
QPushButton[sjVariant="quiet"]:hover, QToolButton[sjVariant="quiet"]:hover { background: @raised; }
QPushButton:disabled, QToolButton:disabled { color: @muted; background: @surface; border-color: @border; }
QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit { background: @surface; border: @metricBorderWidthpx solid @border; border-radius: @radiuspx; padding: @controlPaddingYpx @controlPaddingXpx; min-height: @innerHeightpx; }
QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled { color: @muted; background: @raised; }
QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit { padding-right: 28px; }
QComboBox::drop-down, QDateEdit::drop-down { subcontrol-origin: border; subcontrol-position: top right; width: 24px; border-left: @metricBorderWidthpx solid @border; }
QComboBox::down-arrow, QDateEdit::down-arrow { image: url(:/stackjunkie/chevron-down-@glyphTone.svg); width: 14px; height: 14px; }
QSpinBox::up-button, QDoubleSpinBox::up-button, QDateEdit::up-button, QTimeEdit::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 24px; height: @halfControlpx; border-left: @metricBorderWidthpx solid @border; }
QSpinBox::down-button, QDoubleSpinBox::down-button, QDateEdit::down-button, QTimeEdit::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 24px; height: @halfControlpx; border-left: @metricBorderWidthpx solid @border; }
QSpinBox::up-button:hover, QSpinBox::down-button:hover, QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover, QTimeEdit::up-button:hover, QTimeEdit::down-button:hover { background: @selection; }
QSpinBox::up-arrow, QDoubleSpinBox::up-arrow, QDateEdit::up-arrow, QTimeEdit::up-arrow { image: url(:/stackjunkie/chevron-up-@glyphTone.svg); width: 12px; height: 12px; }
QSpinBox::down-arrow, QDoubleSpinBox::down-arrow, QDateEdit::down-arrow, QTimeEdit::down-arrow { image: url(:/stackjunkie/chevron-down-@glyphTone.svg); width: 12px; height: 12px; }
QWidget[sjInvalid="true"] { border-color: @danger; }
QComboBox QAbstractItemView { background: @elevated; color: @text; selection-background-color: @selection; border: @metricBorderWidthpx solid @borderStrong; }
QTextEdit, QPlainTextEdit { background: @surface; border: @metricBorderWidthpx solid @border; border-radius: @radiuspx; padding: 8px; }
QPlainTextEdit[sjRole="code"] { font-family: "@mono"; }
QCheckBox, QRadioButton { spacing: 8px; min-height: @controlHeightpx; background: transparent; }
QCheckBox::indicator, QRadioButton::indicator { width: 18px; height: 18px; border: @metricBorderWidthpx solid @muted; background: @surface; }
QCheckBox::indicator { border-radius: 3px; }
QCheckBox::indicator:checked { background: @accent; border-color: @accent; image: url(:/stackjunkie/check-@glyphTone.svg); }
QCheckBox::indicator:indeterminate { background: @accent; border-color: @accent; image: url(:/stackjunkie/mixed-@glyphTone.svg); }
QCheckBox::indicator:disabled, QRadioButton::indicator:disabled { border-color: @border; background: @raised; }
QCheckBox::indicator:checked:disabled { background: @muted; }
QCheckBox:focus, QRadioButton:focus { outline: 1px solid @focus; }
QRadioButton::indicator { border-radius: 10px; }
QRadioButton::indicator:checked { border: 6px solid @accent; width: 8px; height: 8px; background: @onAccent; }
QCheckBox[sjVariant="switch"]::indicator { width: 32px; height: 18px; border-radius: 9px; background: @border; border: @metricBorderWidthpx solid @border; }
QCheckBox[sjVariant="switch"]::indicator:checked { background: @accent; border-color: @accent; }
QGroupBox { border: @metricBorderWidthpx solid @border; border-radius: @radiuspx; margin-top: 12px; padding-top: 16px; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }
QTabWidget::pane { border: @metricBorderWidthpx solid @border; background: @surface; }
QTabBar::tab { background: @canvas; color: @muted; font-size: @chromepx; font-weight: @regularWeight; border-top: 2px solid transparent; border-bottom: 1px solid transparent; border-right: @metricBorderWidthpx solid @border; padding: 0 @controlPaddingXpx; min-height: @tabInnerHeightpx; margin: 0; }
QTabBar::tab:selected { background: @surface; color: @text; border-top-color: @accent; border-bottom: 1px solid @surface; }
QTabBar::tab:hover { background: @raised; }
QFrame[sjRole="pageTabRow"] { background: @canvas; border: 0; border-bottom: @metricBorderWidthpx solid @border; }
QTabBar QToolButton { background: @canvas; border: 0; border-left: @metricBorderWidthpx solid @border; border-radius: 0; padding: 0; min-height: 0; }
QTabBar QToolButton:hover { background: @raised; }
QTabBar QToolButton::left-arrow { image: url(:/stackjunkie/chevron-left-@glyphTone.svg); width: 12px; height: 12px; }
QTabBar QToolButton::right-arrow { image: url(:/stackjunkie/chevron-right-@glyphTone.svg); width: 12px; height: 12px; }
QTabBar::scroller { width: 48px; }
QListWidget, QTreeWidget, QTableWidget, QListView, QTreeView, QTableView { background: @surface; alternate-background-color: @raised; border: @metricBorderWidthpx solid @border; gridline-color: @border; selection-background-color: @selection; selection-color: @text; }
QListWidget::item, QTreeView::item, QTableView::item { padding: 6px; min-height: 20px; }
QListWidget::item:hover, QTreeView::item:hover { background: @raised; }
QListWidget[sjRole="navigation"], QListWidget[sjRole="rail"] { background: @canvas; border: 0; outline: 0; }
QListWidget[sjRole="navigation"] { font-size: @chromepx; }
QListWidget[sjRole="navigation"]::item { padding: 0 @controlPaddingXpx; min-height: @navRowHeightpx; border-left: 3px solid transparent; }
QListWidget[sjRole="rail"]::item { padding: 0; }
QListWidget[sjRole="navigation"]::item:selected, QListWidget[sjRole="rail"]::item:selected { background: @selection; border-left: 3px solid @accent; }
QTreeWidget[sjRole="navTree"] { background: @canvas; border: 0; outline: 0; font-size: @chromepx; }
QTreeWidget[sjRole="navTree"]::item { min-height: @navRowHeightpx; padding: 0 4px; }
QTreeWidget[sjRole="navTree"]::item:selected { background: @selection; color: @accent; }
QHeaderView::section { background: @raised; color: @muted; border: 0; border-bottom: @metricBorderWidthpx solid @border; padding: 8px; text-align: left; }
QScrollArea { border: 0; background: @surface; }
QScrollArea > QWidget, QScrollArea > QWidget > QWidget { background: @surface; }
QDialog { background: @elevated; }
QDialog QScrollArea, QDialog QScrollArea > QWidget, QDialog QScrollArea > QWidget > QWidget, QDialog QStackedWidget { background: @elevated; }
QFrame[sjRole="settingsNav"] { background: @canvas; border: 0; border-right: 1px solid @border; }
QToolButton[sjRole="tabClose"] { border: 0; padding: 0; min-height: 0; background: transparent; }
QToolButton[sjRole="tabClose"]:hover { background: @raised; }
QScrollBar:vertical { background: @canvas; width: 8px; margin: 0; }
QScrollBar::handle:vertical { background: @border; min-height: 28px; border-radius: 4px; margin: 2px; }
QScrollBar::handle:vertical:hover { background: @muted; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: @canvas; height: 8px; margin: 0; }
QScrollBar::handle:horizontal { background: @border; min-width: 28px; border-radius: 4px; margin: 2px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
QSlider::groove:horizontal { height: 4px; background: @border; border-radius: 2px; }
QSlider::handle:horizontal { background: @accent; width: 18px; margin: -7px 0; border-radius: 9px; }
QSlider::sub-page:horizontal { background: @accent; }
QSlider:focus { border: 1px solid @focus; }
QProgressBar { border: @metricBorderWidthpx solid @border; border-radius: @radiuspx; background: @raised; text-align: center; min-height: 20px; }
QProgressBar::chunk { background: @accent; border-radius: @radiuspx; }
QSplitter::handle { background: transparent; }
QSplitter::handle:horizontal { border-left: 1px solid @border; }
QSplitter::handle:vertical { border-top: 1px solid @border; }
QSplitter::handle:hover { background: @accent; }
QSplitter::handle:focus { background: @focus; border: 1px solid @text; }
QMenu, QToolTip { background: @elevated; color: @text; border: @metricBorderWidthpx solid @borderStrong; padding: 4px; }
QMenu::item { padding: 7px 24px; }
QMenu::item:selected { background: @selection; }
QMenu::separator { height: 1px; background: @border; margin: 4px 8px; }
QMenu { font-size: @chromepx; }
QMenu::item:disabled { color: @muted; }
QMenu::indicator { width: 14px; height: 14px; left: 5px; }
QMenu::indicator:checked { background: @accent; border-radius: 3px; image: url(:/stackjunkie/check-@glyphTone.svg); }
QMenuBar { background: transparent; border: 0; font-size: @chromepx; spacing: 0; }
QMenuBar::item { background: transparent; color: @text; padding: 3px 8px; border-radius: @radiuspx; }
QMenuBar::item:selected, QMenuBar::item:pressed { background: @raised; }
QFrame[sjRole="statusBar"] { background: @canvas; border: 0; border-top: @metricBorderWidthpx solid @border; }
QFrame[sjRole="statusBar"] QLabel { color: @muted; font-size: @captionpx; }
QFrame[sjRole="statusBar"] QLabel[sjRole="statusState"] { color: @text; }
QFrame[sjRole="bottomPanel"] { background: @surface; border: 0; }
QFrame[sjRole="bottomPanel"] QStackedWidget { background: @surface; }
QFrame[sjRole="panel"], QFrame[sjRole="code"], QFrame[sjRole="disclosure"] { background: @surface; border: @metricBorderWidthpx solid @border; border-radius: @radiuspx; }
QFrame[sjRole="help"] { background: @elevated; border: @metricBorderWidthpx solid @borderStrong; border-radius: @radiuspx; }
QFrame[sjRole="panel"][sjVariant="inset"] { background: @raised; }
QFrame[sjRole="notice"] { background: @surface; border: @metricBorderWidthpx solid @border; border-left: 3px solid @accent; border-radius: @radiuspx; }
QFrame[sjRole="notice"][sjTone="success"] { border-left-color: @success; }
QFrame[sjRole="notice"][sjTone="warning"] { border-left-color: @warning; }
QFrame[sjRole="notice"][sjTone="danger"] { border-left-color: @danger; }
QLabel[sjRole="tag"] { background: @raised; color: @muted; border-radius: @radiuspx; padding: 3px 8px; }
QLabel[sjRole="tag"][sjTone="success"] { color: @success; }
QLabel[sjRole="tag"][sjTone="warning"] { color: @warning; }
QLabel[sjRole="tag"][sjTone="danger"] { color: @danger; }
QFrame[sjRole="separator"] { background: @border; max-height: 1px; border: 0; }
QFrame[sjRole="aboutLogo"] { background: @canvas; border: @metricBorderWidthpx solid @border; border-radius: 12px; }
QLabel[sjRole="versionChip"] { background: @raised; color: @muted; border: @metricBorderWidthpx solid @border; border-style: solid; border-radius: 9px; padding: 1px 8px; font-size: @captionpx; min-height: 16px; max-height: 20px; }
QLabel[sjRole="aboutLinks"] { font-size: @labelSizepx; }
QToolButton[sjRole="home"] { background: transparent; border: 0; padding: 2px 6px 2px 2px; font-size: @chromepx; font-weight: @mediumWeight; min-height: 0; }
QToolButton[sjRole="home"]:hover { background: @raised; }
QToolButton[sjRole="windowButton"] { background: transparent; border: 0; border-radius: 0; padding: 0; min-height: @headerHeightpx; max-height: @headerHeightpx; min-width: 46px; max-width: 46px; }
QToolButton[sjRole="windowButton"]:hover, QToolButton[sjRole="windowButton"][sjHover="true"] { background: @raised; }
QToolButton[sjRole="windowButton"][sjVariant="close"]:hover { background: @danger; }
)CSS");
    // Replace longer token names first so @accent cannot consume @accentHover.
    QStringList keys = colors.keys();
    std::sort(keys.begin(), keys.end(), [](const QString &a, const QString &b) { return a.size() > b.size(); });
    for (const auto &key : keys) css.replace(QStringLiteral("@") + key, colors.value(key).toString());
    css.replace(QStringLiteral("@caption"), QString::number(rawTypeSize(QStringLiteral("caption"), 12)));
    css.replace(QStringLiteral("@contextInnerHeight"), QString::number(rawDensity(QStringLiteral("contextHeight"), 36) - 2 * metric(QStringLiteral("borderWidth"), 1)));
    css.replace(QStringLiteral("@headerHeight"), QString::number(rawDensity(QStringLiteral("headerHeight"), 40)));
    css.replace(QStringLiteral("@heading"), QString::number(rawTypeSize(QStringLiteral("heading"), 22)));
    css.replace(QStringLiteral("@section"), QString::number(rawTypeSize(QStringLiteral("section"), 16)));
    css.replace(QStringLiteral("@labelSize"), QString::number(rawTypeSize(QStringLiteral("label"), 14)));
    css.replace(QStringLiteral("@chrome"), QString::number(rawTypeSize(QStringLiteral("chrome"), 13)));
    css.replace(QStringLiteral("@regularWeight"), QString::number(typography(QStringLiteral("regularWeight"), 400)));
    css.replace(QStringLiteral("@mediumWeight"), QString::number(typography(QStringLiteral("mediumWeight"), 500)));
    css.replace(QStringLiteral("@strongWeight"), QString::number(typography(QStringLiteral("strongWeight"), 600)));
    css.replace(QStringLiteral("@innerHeight"), QString::number(qMax(16, metric(QStringLiteral("controlHeight"), 32) - 2 * rawDensity(QStringLiteral("controlPaddingY"), 4) - 2 * metric(QStringLiteral("borderWidth"), 1))));
    css.replace(QStringLiteral("@controlPaddingY"), QString::number(rawDensity(QStringLiteral("controlPaddingY"), 4)));
    css.replace(QStringLiteral("@controlPaddingX"), QString::number(rawDensity(QStringLiteral("controlPaddingX"), 10)));
    css.replace(QStringLiteral("@tabInnerHeight"), QString::number(rawDensity(QStringLiteral("tabHeight"), 32) - 3));
    css.replace(QStringLiteral("@navRowHeight"), QString::number(rawDensity(QStringLiteral("navRowHeight"), 28)));
    css.replace(QStringLiteral("@controlHeight"), QString::number(metric(QStringLiteral("controlHeight"), 36)));
    css.replace(QStringLiteral("@halfControl"), QString::number(metric(QStringLiteral("controlHeight"), 36) / 2));
    css.replace(QStringLiteral("@metricBorderWidth"), QString::number(metric(QStringLiteral("borderWidth"), 1)));
    css.replace(QStringLiteral("@radius"), QString::number(metric(QStringLiteral("radius"), 6)));
    css.replace(QStringLiteral("@glyphTone"), direction + (dark ? QStringLiteral("-dark") : QStringLiteral("-light")));
    css.replace(QStringLiteral("@baseFont"), QString::number(metric(QStringLiteral("fontSize"), 14)));
    css.replace(QStringLiteral("@reading"), QString::number(rawTypeSize(QStringLiteral("reading"), 16)));
    css.replace(QStringLiteral("@codeSize"), QString::number(rawTypeSize(QStringLiteral("code"), 12)));
    css.replace(QStringLiteral("@ui"), font(QStringLiteral("ui")));
    css.replace(QStringLiteral("@editorial"), font(QStringLiteral("editorial")));
    css.replace(QStringLiteral("@mono"), font(QStringLiteral("mono")));
    // Every size in the sheet follows this theme's zoom; a border never drops below one pixel.
    if (zoom != 100) {
        static const QRegularExpression pixels(QStringLiteral("(\\d+)px"));
        QString zoomedCss; qsizetype at = 0;
        for (auto match = pixels.globalMatch(css); match.hasNext();) {
            const auto found = match.next();
            zoomedCss += css.mid(at, found.capturedStart() - at) + QString::number(scaled(found.captured(1).toInt())) + QStringLiteral("px");
            at = found.capturedEnd();
        }
        css = zoomedCss + css.mid(at);
    }
    return css;
}
void Theme::apply(QWidget *root) const {
    initializeResources();
    root->setProperty("sjRoot", true);
    root->setProperty("sjZoom", zoom);
    root->setProperty("sjDirection", direction);
    root->setProperty("sjDark", dark);
    root->setAutoFillBackground(true);
    QPalette p = root->palette();
    p.setColor(QPalette::Window, color(QStringLiteral("canvas")));
    p.setColor(QPalette::WindowText, color(QStringLiteral("text")));
    p.setColor(QPalette::Base, color(QStringLiteral("surface")));
    p.setColor(QPalette::AlternateBase, color(QStringLiteral("raised")));
    p.setColor(QPalette::Text, color(QStringLiteral("text")));
    p.setColor(QPalette::Button, color(QStringLiteral("surface")));
    p.setColor(QPalette::ButtonText, color(QStringLiteral("text")));
    p.setColor(QPalette::Highlight, color(QStringLiteral("selection")));
    p.setColor(QPalette::HighlightedText, color(QStringLiteral("text")));
    p.setColor(QPalette::PlaceholderText, color(QStringLiteral("muted")));
    p.setColor(QPalette::Link, color(QStringLiteral("accent")));
    p.setColor(QPalette::LinkVisited, color(QStringLiteral("accent")));
    p.setColor(QPalette::Disabled, QPalette::Text, color(QStringLiteral("muted")));
    root->setPalette(p);
    QFont f(font(QStringLiteral("ui")));
    f.setPixelSize(scaled(metric(QStringLiteral("fontSize"), 14)));
    f.setWeight(static_cast<QFont::Weight>(typography(QStringLiteral("regularWeight"), 400)));
    root->setFont(f);
    root->setStyleSheet(styleSheet());
    if (auto *reading = dynamic_cast<ReadingView *>(root)) reading->applyTheme(*this);
    for (auto *button : root->findChildren<QAbstractButton *>()) {
        const auto name = button->property("sjIcon").toString();
        if (!name.isEmpty()) button->setIcon(icon(name, color(QStringLiteral("text"))));
        // Icon buttons keep the density's target size at this zoom, unless they were given their own size.
        if (button->property("sjTarget").toBool()) {
            const int target = scaled(rawDensity(QStringLiteral("iconTarget"), 28));
            button->setFixedSize(target, target);
            button->setIconSize(QSize(scaled(16), scaled(16)));
        }
    }
    for (auto *list : root->findChildren<QListWidget *>()) {
        for (int i = 0; i < list->count(); ++i) {
            auto *item = list->item(i);
            const auto name = item->data(Qt::UserRole + 10).toString();
            if (!name.isEmpty()) item->setIcon(icon(name, color(QStringLiteral("text")), qMax(20, list->iconSize().width())));
        }
    }
    for (auto *widget : root->findChildren<QWidget *>()) {
        if (auto *label = qobject_cast<QLabel *>(widget); label && label->property("sjBrandSize").isValid()) label->setPixmap(brandMark(*this, label->property("sjBrandSize").toInt()));
        // A logo button without the app's own logo shows the interim SJ mark in this theme's colors.
        if (auto *button = qobject_cast<QAbstractButton *>(widget); button && button->property("sjBrandIcon").isValid()) button->setIcon(QIcon(brandMark(*this, button->property("sjBrandIcon").toInt() * 2)));
        if (auto *label = qobject_cast<QLabel *>(widget); label && label->property("sjAttributionHtml").isValid()) {
            // The style's credit keeps its links in the secondary text color; other links use the accent.
            const bool credit = label->property("sjRole").toString() == QStringLiteral("attribution");
            label->setText(label->property("sjAttributionHtml").toString().replace(QStringLiteral("<a "), QStringLiteral("<a style=\"color:%1\" ").arg(color(credit ? QStringLiteral("muted") : QStringLiteral("accent")).name())));
        }
        if (dynamic_cast<Drawer *>(widget) || widget->property("sjRole").toString() == QStringLiteral("help")) apply(widget);
        if (auto *canvas = dynamic_cast<Canvas *>(widget)) canvas->applyTheme(*this);
        if (auto *reading = dynamic_cast<ReadingView *>(widget)) reading->applyTheme(*this);
        const auto role = widget->property("sjFontRole").toString();
        if (!role.isEmpty()) {
            QFont roleFont(font(role));
            roleFont.setPixelSize(scaled(rawTypeSize(role == QStringLiteral("mono") ? QStringLiteral("code") : QStringLiteral("reading"), 14)));
            widget->setFont(roleFont);
        }
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    }
}
QPixmap brandMark(const Theme &theme, int size) {
    initializeResources();
    const QImage mask(QStringLiteral(":/stackjunkie/brand/sj-transparent-mask.png"));
    if (mask.isNull()) throw std::runtime_error("Stack Junkie transparent logo resource missing");
    auto tinted = mask.scaled(size * 2, size * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
    const auto text = theme.color(QStringLiteral("text"));
    const auto accent = theme.color(QStringLiteral("accent"));
    for (int y = 0; y < tinted.height(); ++y) {
        auto *pixels = reinterpret_cast<QRgb *>(tinted.scanLine(y));
        for (int x = 0; x < tinted.width(); ++x) {
            const int amount = qRed(pixels[x]);
            const auto mix = [amount](int a, int b) { return (a * (255 - amount) + b * amount + 127) / 255; };
            pixels[x] = qRgba(mix(text.red(), accent.red()), mix(text.green(), accent.green()), mix(text.blue(), accent.blue()), qAlpha(pixels[x]));
        }
    }
    return QPixmap::fromImage(tinted).scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
QIcon icon(const QString &name, const QColor &color, int size) {
    initializeResources();
    QFile source(QStringLiteral(":/stackjunkie/icons.svg"));
    if (!source.open(QIODevice::ReadOnly)) return {};
    QXmlStreamReader reader(source.readAll());
    QByteArray svg;
    QXmlStreamWriter writer(&svg);
    bool active = false;
    int depth = 0;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (!active && reader.name() == QStringLiteral("symbol") && reader.attributes().value(QStringLiteral("id")) == name) {
                active = true;
                writer.writeStartElement(QStringLiteral("svg"));
                writer.writeDefaultNamespace(QStringLiteral("http://www.w3.org/2000/svg"));
                for (const auto &a : reader.attributes()) if (a.name() != QStringLiteral("id")) writer.writeAttribute(a.name().toString(), a.value().toString().replace(QStringLiteral("currentColor"), color.name()));
                depth = 1;
            } else if (active) {
                writer.writeStartElement(reader.name().toString());
                for (const auto &a : reader.attributes()) writer.writeAttribute(a.name().toString(), a.value().toString().replace(QStringLiteral("currentColor"), color.name()));
                ++depth;
            }
        } else if (reader.isEndElement() && active) {
            writer.writeEndElement();
            if (--depth == 0) break;
        }
    }
    if (svg.isEmpty()) return {};
    QPixmap image(size * 2, size * 2);
    image.setDevicePixelRatio(2);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    QSvgRenderer renderer(svg);
    renderer.render(&painter, QRectF(0, 0, size, size));
    return QIcon(image);
}
}
