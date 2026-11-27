#pragma once
#include <stackjunkie/export.hpp>
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QJsonObject>
#include <QStringList>
class QWidget;

namespace StackJunkie {
struct STACKJUNKIE_EXPORT Theme {
    QString direction = QStringLiteral("clean");
    bool dark = false;
    QJsonObject definition;
    QJsonObject colors;
    /** Zoom percentage this theme draws at. load() takes the app's zoom (setZoom); zoomed() makes a copy for one area. */
    int zoom = 100;
    static Theme load(const QString &direction = QStringLiteral("clean"), bool dark = false);
    static QStringList directions();
    static QJsonObject catalog();
    static int spacing(int index);
    static int typeSize(const QString &role, int fallback = 14);
    static int density(const QString &role, int fallback = 0);
    /** "compact" (the default) or "comfortable". One mode for the whole app: density() and the control height read it.
        Workspace::setChoices sets it; re-apply the theme after setting it yourself. */
    static void setDensityMode(const QString &mode);
    static QString densityMode();
    /** The whole app's zoom percentage, 50 to 200. spacing(), typeSize(), density() and new themes follow it;
        Workspace::setZoom sets it and re-applies the theme. */
    static void setZoom(int percent);
    static int globalZoom();
    /** The next zoom step up (direction 1) or down (-1): 50, 67, 75, 80, 90, 100, 110, 125, 150, 175, 200, as on web. */
    static int zoomStep(int current, int direction);
    /** This theme at another zoom, for zooming one area. */
    Theme zoomed(int percent) const;
    /** A size in pixels at this theme's zoom. */
    int scaled(int pixels) const;
    static qreal typography(const QString &role, qreal fallback = 0);
    static int motionDuration(const QString &role = QStringLiteral("normal"));
    QColor color(const QString &name) const;
    int metric(const QString &name, int fallback) const;
    QString font(const QString &role) const;
    QString label() const;
    QString styleSheet() const;
    void apply(QWidget *root) const;
};
STACKJUNKIE_EXPORT QIcon icon(const QString &name, const QColor &color, int size = 20);
STACKJUNKIE_EXPORT QPixmap brandMark(const Theme &theme, int size);
/** Registers the embedded resources and the style's fonts, makes the style's UI font the application font, and turns on
    the shared pointer and tooltip behavior: the pointing hand over anything clickable, and tooltips beside their element.
    AppShell and Theme::apply call it, so an app needs no extra setup. Safe to call more than once. */
STACKJUNKIE_EXPORT void initializeResources();
}
