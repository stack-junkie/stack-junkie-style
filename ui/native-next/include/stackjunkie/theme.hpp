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
    static Theme load(const QString &direction = QStringLiteral("clean"), bool dark = false);
    static QStringList directions();
    static QJsonObject catalog();
    static int spacing(int index);
    static int typeSize(const QString &role, int fallback = 14);
    static int density(const QString &role, int fallback = 0);
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
STACKJUNKIE_EXPORT void initializeResources();
}
