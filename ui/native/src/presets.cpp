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
    if (choice == QStringLiteral("siteNavigation")) return siteNavigation;
    if (choice == QStringLiteral("siteFooter")) return siteFooter;
    if (choice == QStringLiteral("headerSearch")) return headerSearch;
    if (choice == QStringLiteral("headerButton")) return headerButton;
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
        else if (choice == QStringLiteral("siteFooter")) siteFooter = on;
        else if (choice == QStringLiteral("headerSearch")) headerSearch = on;
        else if (choice == QStringLiteral("headerButton")) headerButton = on;
        else return false;
        return true;
    }
    const QString text = value.toString();
    if (!options.contains(text)) return false;
    if (choice == QStringLiteral("sidebar")) sidebar = text;
    else if (choice == QStringLiteral("theme")) theme = text;
    else if (choice == QStringLiteral("density")) density = text;
    else if (choice == QStringLiteral("siteNavigation")) siteNavigation = text;
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
        QString group = QStringLiteral("shared");
        const auto groups = presetDocument().value(QStringLiteral("groups")).toObject();
        for (auto it = groups.begin(); it != groups.end(); ++it) if (it.value().toObject().value(QStringLiteral("choices")).toArray().contains(id)) group = it.key();
        result.append({id, info.value(QStringLiteral("label")).toString(), info.value(QStringLiteral("description")).toString(), info.value(QStringLiteral("options")).toArray().toVariantList(), group});
    }
    return result;
}
QList<Preset> Preset::all() {
    QList<Preset> result;
    for (const auto &value : presetDocument().value(QStringLiteral("presets")).toArray()) {
        const auto object = value.toObject();
        Preset preset{object.value(QStringLiteral("id")).toString(), object.value(QStringLiteral("label")).toString(), object.value(QStringLiteral("description")).toString(), {}, object.value(QStringLiteral("madeFor")).toString()};
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
// Custom preset files mirror ui/shared/preset-file.mjs: the same rules, messages and bytes.
static const QString presetFileFormat = QStringLiteral("stack-junkie-preset");
static constexpr int presetFileVersion = 1;
static constexpr int presetNameMaxLength = 60;
static QString shown(const QJsonValue &value) {
    if (value.isString()) return QLatin1Char('"') + value.toString() + QLatin1Char('"');
    if (value.isUndefined()) return QStringLiteral("undefined");
    const QString text = QString::fromUtf8(QJsonDocument(QJsonArray{value}).toJson(QJsonDocument::Compact));
    return text.mid(1, text.size() - 2);
}
static QString optionsText(const QJsonArray &options) {
    QStringList result;
    for (const auto &option : options) result << (option.isBool() ? (option.toBool() ? QStringLiteral("true") : QStringLiteral("false")) : option.toString());
    return result.join(QStringLiteral(", "));
}
static QString presetIdsText() {
    QStringList ids;
    for (const auto &preset : Preset::all()) ids << preset.id;
    return ids.join(QStringLiteral(", "));
}
QString CustomPreset::nameError(const QString &name) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return QStringLiteral("The preset needs a name.");
    if (trimmed.size() > presetNameMaxLength) return QStringLiteral("The preset name is longer than %1 characters.").arg(presetNameMaxLength);
    for (const QChar c : name) if (c.unicode() < 0x20 || c.unicode() == 0x7f) return QStringLiteral("The preset name cannot contain line breaks or control characters.");
    return {};
}
// Checks JSON values, so a bool written as text is rejected exactly as on web.
static QString checkJson(const QString &name, const QJsonValue &base, const QJsonValue &choicesValue, CustomPreset *checked) {
    const QString nameProblem = CustomPreset::nameError(name);
    if (!nameProblem.isEmpty()) return nameProblem;
    if (!base.isString()) return QStringLiteral("The preset needs a base preset: one of %1.").arg(presetIdsText());
    if (!Preset::exists(base.toString())) return QStringLiteral("Unknown base preset %1. Use one of: %2.").arg(shown(base), presetIdsText());
    if (!choicesValue.isObject()) return QStringLiteral("The preset choices must be an object.");
    const auto choices = choicesValue.toObject();
    const auto definitions = presetDocument().value(QStringLiteral("choices")).toObject();
    for (auto it = choices.begin(); it != choices.end(); ++it) {
        if (!definitions.contains(it.key())) return QStringLiteral("Unknown choice %1.").arg(shown(it.key()));
        const auto options = definitions.value(it.key()).toObject().value(QStringLiteral("options")).toArray();
        if (!options.contains(it.value())) return QStringLiteral("The %1 choice cannot be %2. Use one of: %3.").arg(it.key(), shown(it.value()), optionsText(options));
    }
    const auto baseChoices = Preset::find(base.toString()).choices;
    CustomPreset result{name.trimmed(), base.toString(), {}};
    for (const auto &id : presetSource().choiceOrder) {
        if (!choices.contains(id)) continue;
        const QVariant value = choices.value(id).toVariant();
        if (value != baseChoices.value(id)) result.choices.insert(id, value);
    }
    if (checked) *checked = result;
    return {};
}
QString CustomPreset::check(const CustomPreset &preset, CustomPreset *checked) {
    QJsonObject choices;
    for (auto it = preset.choices.begin(); it != preset.choices.end(); ++it) {
        const auto type = it.value().typeId();
        // Only bool and text reach the file: other types (such as a number) are shown as they are and rejected.
        choices.insert(it.key(), type == QMetaType::Bool ? QJsonValue(it.value().toBool()) : type == QMetaType::QString ? QJsonValue(it.value().toString()) : QJsonValue::fromVariant(it.value()));
    }
    return checkJson(preset.name, QJsonValue(preset.base), choices, checked);
}
QString CustomPreset::parse(const QByteArray &text, CustomPreset *result) {
    QByteArray bytes = text;
    if (bytes.startsWith("\xEF\xBB\xBF")) bytes.remove(0, 3);
    QJsonParseError parseError;
    const auto doc = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) return QStringLiteral("The file is not valid JSON.");
    const auto data = doc.object();
    if (!doc.isObject() || data.value(QStringLiteral("format")) != QJsonValue(presetFileFormat)) return QStringLiteral("This is not a Stack Junkie preset file.");
    const auto version = data.value(QStringLiteral("version"));
    if (!version.isDouble() || version.toDouble() != presetFileVersion) return QStringLiteral("The preset file is version %1. This version reads version %2.").arg(shown(version)).arg(presetFileVersion);
    const QStringList fileKeys{QStringLiteral("format"), QStringLiteral("version"), QStringLiteral("name"), QStringLiteral("base"), QStringLiteral("choices")};
    for (const auto &key : data.keys()) if (!fileKeys.contains(key)) return QStringLiteral("Unknown field %1 in the preset file.").arg(shown(key));
    if (!data.contains(QStringLiteral("choices"))) return QStringLiteral("The preset file has no choices.");
    const auto name = data.value(QStringLiteral("name"));
    if (!name.isString()) return QStringLiteral("The preset needs a name.");
    CustomPreset checked;
    const QString error = checkJson(name.toString(), data.value(QStringLiteral("base")), data.value(QStringLiteral("choices")), &checked);
    if (!error.isEmpty()) return error;
    if (result) *result = checked;
    return {};
}
// JSON text the way JSON.stringify(value, null, 2) writes it. Names have no control characters, so only quotes and backslashes need escaping.
static QByteArray jsonText(const QString &text) {
    QString escaped = text;
    escaped.replace(QLatin1Char('\\'), QStringLiteral("\\\\")).replace(QLatin1Char('"'), QStringLiteral("\\\""));
    return '"' + escaped.toUtf8() + '"';
}
QByteArray CustomPreset::toFile() const {
    CustomPreset preset;
    if (!check(*this, &preset).isEmpty()) return {};
    QByteArray out = "{\n  \"format\": " + jsonText(presetFileFormat) + ",\n  \"version\": " + QByteArray::number(presetFileVersion)
        + ",\n  \"name\": " + jsonText(preset.name) + ",\n  \"base\": " + jsonText(preset.base) + ",\n  \"choices\": {";
    QList<QByteArray> lines;
    for (const auto &id : presetSource().choiceOrder) {
        if (!preset.choices.contains(id)) continue;
        const QVariant value = preset.choices.value(id);
        lines << "    " + jsonText(id) + ": " + (value.typeId() == QMetaType::Bool ? QByteArray(value.toBool() ? "true" : "false") : jsonText(value.toString()));
    }
    out += lines.isEmpty() ? QByteArray("}") : "\n" + lines.join(",\n") + "\n  }";
    return out + "\n}\n";
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
