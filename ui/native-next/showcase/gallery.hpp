#pragma once
#include <stackjunkie/theme.hpp>
#include <QJsonObject>
#include <QWidget>
QWidget *makeFamily(const QJsonObject &family, QWidget *parent = nullptr);
QWidget *makeComparison(QWidget *parent = nullptr);
void applyComparisonThemes(QWidget *comparison, bool dark);
QWidget *makeLayout(const QString &id, QWidget *parent = nullptr);
