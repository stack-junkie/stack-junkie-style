// Zoom: Control and the mouse wheel, like Control plus and minus, zoom the whole app. With "Zoom only the area under
// the pointer" on (Settings > Appearance > Advanced), they zoom just the Sidebar, the pages or the bottom panel, and
// each area remembers its zoom. Both are saved for the app, so they return after a restart.
#include <stackjunkie/components.hpp>
#include <QApplication>
#include <QSettings>
#include <QWheelEvent>

namespace StackJunkie {
static const QStringList zoomAreas{QStringLiteral("sidebar"), QStringLiteral("pages"), QStringLiteral("panel")};

QWidget *Workspace::areaWidget(const QString &area) const {
    if (area == QStringLiteral("sidebar")) return sidebar_;
    if (area == QStringLiteral("pages")) return pages_->parentWidget();
    if (area == QStringLiteral("panel")) return bottomPanel_;
    return nullptr;
}
QString Workspace::areaAt(QWidget *widget) const {
    for (const auto &area : zoomAreas) if (auto *root = areaWidget(area); root && (widget == root || root->isAncestorOf(widget))) return area;
    return {};
}
// Saved per app, under the name in the logo slot.
static QSettings zoomSettings(const QString &app) { return QSettings(QStringLiteral("Stack Junkie Style"), app.isEmpty() ? QStringLiteral("App") : app); }

void Workspace::setAreaZoomEnabled(bool enabled) {
    areaZoomEnabled_ = enabled;
    auto settings = zoomSettings(heading_->text());
    settings.setValue(QStringLiteral("zoom/perArea"), enabled);
    reapplyAreaZoom();
    emit areaZoomEnabledChanged(enabled);
}
int Workspace::areaZoom(const QString &area) const { return areaZooms_.value(area, zoom()); }
void Workspace::setAreaZoom(const QString &area, int percent) {
    if (!zoomAreas.contains(area)) return;
    areaZooms_.insert(area, qBound(50, percent, 200));
    auto settings = zoomSettings(heading_->text());
    settings.setValue(QStringLiteral("zoom/") + area, areaZooms_.value(area));
    reapplyAreaZoom();
}
// Each zoomed area carries its own copy of the theme at its zoom; with area zoom off, areas follow the whole app.
void Workspace::reapplyAreaZoom() {
    if (!property("sjDirection").isValid()) return;
    const auto theme = Theme::load(property("sjDirection").toString(), property("sjDark").toBool());
    for (const auto &area : zoomAreas) {
        auto *widget = areaWidget(area);
        const bool own = areaZoomEnabled_ && areaZooms_.contains(area) && areaZooms_.value(area) != theme.zoom;
        if (own) { theme.zoomed(areaZooms_.value(area)).apply(widget); widget->setProperty("sjAreaZoomed", true); }
        else if (widget->property("sjAreaZoomed").toBool()) { widget->setStyleSheet(QString()); widget->setFont(QFont()); widget->setAutoFillBackground(false); theme.apply(widget); widget->setStyleSheet(QString()); widget->setProperty("sjAreaZoomed", false); }
    }
}
void Workspace::restoreZoom() {
    if (zoomRestored_) return;
    zoomRestored_ = true;
    auto settings = zoomSettings(heading_->text());
    areaZoomEnabled_ = settings.value(QStringLiteral("zoom/perArea"), false).toBool();
    for (const auto &area : zoomAreas) if (settings.contains(QStringLiteral("zoom/") + area)) areaZooms_.insert(area, settings.value(QStringLiteral("zoom/") + area).toInt());
    const int whole = settings.value(QStringLiteral("zoom/app"), 100).toInt();
    if (whole != zoom()) setZoom(whole); else reapplyAreaZoom();
}
void Workspace::saveZoom() const { auto settings = zoomSettings(heading_->text()); settings.setValue(QStringLiteral("zoom/app"), zoom()); }

// Control and the wheel over this window: one zoom step for the whole app, or for the area under the pointer.
bool Workspace::zoomWheel(QWidget *widget, QWheelEvent *wheel) {
    if (!(wheel->modifiers() & Qt::ControlModifier) || !widget || widget->window() != this) return false;
    const int direction = wheel->angleDelta().y() > 0 ? 1 : wheel->angleDelta().y() < 0 ? -1 : 0;
    if (!direction) return true;
    const QString area = areaZoomEnabled_ ? areaAt(widget) : QString();
    if (!area.isEmpty()) setAreaZoom(area, Theme::zoomStep(areaZoom(area), direction));
    else setZoom(Theme::zoomStep(zoom(), direction));
    return true;
}
}
