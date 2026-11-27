#include <stackjunkie/presets.hpp>
#include <stackjunkie/theme.hpp>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStyleHints>
#include <stdexcept>

namespace StackJunkie {
struct PresetSource { QJsonObject document; QStringList choiceOrder; };
// Read once: the definition never changes while the app runs.
static const PresetSource &presetSource() {
    static const PresetSource source = [] {
        initializeResources();
        QFile input(QStringLiteral(":/stackjunkie/presets.json"));
        if (!input.open(QIODevice::ReadOnly)) throw std::runtime_error("Missing embedded Stack Junkie presets");
        const QByteArray bytes = input.readAll();
        QJsonParseError error;
        const auto doc = QJsonDocument::fromJson(bytes, &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) throw std::runtime_error("Invalid embedded Stack Junkie presets");
        PresetSource result{doc.object(), {}};
        // QJsonObject sorts its keys, so take the choices' order from the file text: each choice entry starts with its label.
        const auto choices = result.document.value(QStringLiteral("choices")).toObject();
        auto matches = QRegularExpression(QStringLiteral("\"(\\w+)\"\\s*:\\s*\\{\\s*\"label\"")).globalMatch(QString::fromUtf8(bytes));
        while (matches.hasNext()) { const QString id = matches.next().captured(1); if (choices.contains(id) && !result.choiceOrder.contains(id)) result.choiceOrder << id; }
        for (const auto &id : choices.keys()) if (!result.choiceOrder.contains(id)) result.choiceOrder << id;
        return result;
    }();
    return source;
}
static QJsonObject presetDocument() { return presetSource().document; }
// Settings stores can return booleans as "true" or "false" text, so both forms are accepted.
static bool readBool(const QVariant &value, bool *ok) {
    *ok = true;
    if (value.typeId() == QMetaType::Bool) return value.toBool();
    const QString text = value.toString();
    if (text == QStringLiteral("true")) return true;
    if (text == QStringLiteral("false")) return false;
    *ok = false;
    return false;
}
QVariant PresetChoices::value(const QString &choice) const {
    if (choice == QStringLiteral("rail")) return rail;
    if (choice == QStringLiteral("sidebar")) return sidebar;
    if (choice == QStringLiteral("menuBar")) return menuBar;
    if (choice == QStringLiteral("statusBar")) return statusBar;
    if (choice == QStringLiteral("bottomPanel")) return bottomPanel;
    if (choice == QStringLiteral("documentTabs")) return documentTabs;
    if (choice == QStringLiteral("commandPalette")) return commandPalette;
    if (choice == QStringLiteral("theme")) return theme;
    if (choice == QStringLiteral("density")) return density;
    if (choice == QStringLiteral("dashboardHome")) return dashboardHome;
    return {};
}
bool PresetChoices::setValue(const QString &choice, const QVariant &value) {
    const auto info = presetDocument().value(QStringLiteral("choices")).toObject().value(choice).toObject();
    const auto options = info.value(QStringLiteral("options")).toArray();
    if (options.isEmpty()) return false;
    if (options.first().isBool()) {
        bool ok = false;
        const bool on = readBool(value, &ok);
        if (!ok) return false;
        if (choice == QStringLiteral("rail")) rail = on;
        else if (choice == QStringLiteral("menuBar")) menuBar = on;
        else if (choice == QStringLiteral("statusBar")) statusBar = on;
        else if (choice == QStringLiteral("bottomPanel")) bottomPanel = on;
        else if (choice == QStringLiteral("documentTabs")) documentTabs = on;
        else if (choice == QStringLiteral("commandPalette")) commandPalette = on;
        else if (choice == QStringLiteral("dashboardHome")) dashboardHome = on;
        else return false;
        return true;
    }
    const QString text = value.toString();
    if (!options.contains(text)) return false;
    if (choice == QStringLiteral("sidebar")) sidebar = text;
    else if (choice == QStringLiteral("theme")) theme = text;
    else if (choice == QStringLiteral("density")) density = text;
    else return false;
    return true;
}
QVariantMap PresetChoices::toMap() const {
    QVariantMap result;
    for (const auto &info : Preset::choiceInfo()) result.insert(info.id, value(info.id));
    return result;
}
bool PresetChoiceInfo::isSwitch() const { return !options.isEmpty() && options.first().typeId() == QMetaType::Bool; }

QList<PresetChoiceInfo> Preset::choiceInfo() {
    QList<PresetChoiceInfo> result;
    const auto choices = presetDocument().value(QStringLiteral("choices")).toObject();
    for (const auto &id : presetSource().choiceOrder) {
        const auto info = choices.value(id).toObject();
        result.append({id, info.value(QStringLiteral("label")).toString(), info.value(QStringLiteral("description")).toString(), info.value(QStringLiteral("options")).toArray().toVariantList()});
    }
    return result;
}
QList<Preset> Preset::all() {
    QList<Preset> result;
    for (const auto &value : presetDocument().value(QStringLiteral("presets")).toArray()) {
        const auto object = value.toObject();
        Preset preset{object.value(QStringLiteral("id")).toString(), object.value(QStringLiteral("label")).toString(), object.value(QStringLiteral("description")).toString(), {}};
        const auto choices = object.value(QStringLiteral("choices")).toObject();
        for (auto it = choices.begin(); it != choices.end(); ++it) preset.choices.setValue(it.key(), it.value().toVariant());
        result.append(preset);
    }
    if (result.isEmpty()) throw std::runtime_error("Embedded Stack Junkie presets list no presets");
    return result;
}
bool Preset::exists(const QString &id) {
    for (const auto &preset : all()) if (preset.id == id) return true;
    return false;
}
Preset Preset::find(const QString &id) {
    const auto presets = all();
    for (const auto &preset : presets) if (preset.id == id) return preset;
    return presets.first();
}
PresetChoices Preset::resolve(const QString &id, const QVariantMap &overrides) {
    PresetChoices choices = find(id).choices;
    for (auto it = overrides.begin(); it != overrides.end(); ++it) choices.setValue(it.key(), it.value());
    return choices;
}
QVariantMap Preset::overrides(const QString &id, const PresetChoices &choices) {
    const auto base = resolve(id).toMap();
    const auto chosen = choices.toMap();
    QVariantMap result;
    for (auto it = chosen.begin(); it != chosen.end(); ++it) if (base.value(it.key()) != it.value()) result.insert(it.key(), it.value());
    return result;
}
bool systemPrefersDark() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (qGuiApp) return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#endif
    return false;
}
bool prefersDark(const QString &themeChoice) {
    if (themeChoice == QStringLiteral("system")) return systemPrefersDark();
    return themeChoice != QStringLiteral("light");
}
}
