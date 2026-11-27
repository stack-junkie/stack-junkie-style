#include "starter.hpp"
#include <stackjunkie/presets.hpp>
#include <stackjunkie/version.hpp>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

using namespace StackJunkie;

static QJsonObject manifest() {
    QFile file(QStringLiteral(":/starters/starters.json"));
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(file.readAll()).object();
}
static QString readTemplate(const QString &path, QString *error) {
    QFile file(QStringLiteral(":/starters/") + path);
    if (!file.open(QIODevice::ReadOnly)) { *error = QStringLiteral("Missing starter template ") + path; return {}; }
    return QString::fromUtf8(file.readAll()).remove(QLatin1Char('\r'));
}

// The starter's areas and pages as code, exactly as starters.mjs writes them.
static QString sectionsCode(bool web, const QJsonObject &structure) {
    QStringList lines;
    const auto areas = structure.value(QStringLiteral("areas")).toArray();
    for (int i = 0; i < areas.size(); ++i) {
        const auto area = areas[i].toObject();
        const auto pages = area.value(QStringLiteral("pages")).toArray();
        const QString label = area.value(QStringLiteral("label")).toString(), icon = area.value(QStringLiteral("icon")).toString();
        if (web) lines << QStringLiteral("  { id: '%1', label: stub('%2'), icon: '%3', pages: [").arg(area.value(QStringLiteral("id")).toString(), label, icon);
        else lines << QStringLiteral("    const int area%1 = shell.addSection(STACKJUNKIE_STUB_TEXT(\"%2\"), QStringLiteral(\"%3\"));").arg(i + 1).arg(label, icon);
        for (const auto &value : pages) {
            const auto page = value.toObject();
            const QString id = page.value(QStringLiteral("id")).toString(), pageLabel = page.value(QStringLiteral("label")).toString(), content = page.value(QStringLiteral("content")).toString();
            if (web) lines << QStringLiteral("    { id: '%1', label: stub('%2'), content: <Stub block>%3</Stub> },").arg(id, pageLabel, content);
            else lines << QStringLiteral("    shell.addPage(STACKJUNKIE_STUB_TEXT(\"%1\"), STACKJUNKIE_STUB(\"%2\"), QStringLiteral(\"file\"), area%3);").arg(pageLabel, content).arg(i + 1);
        }
        if (web) lines << QStringLiteral("  ] },");
    }
    return lines.join(QLatin1Char('\n'));
}

QString starterLabel(const QString &platform) { return manifest().value(platform).toObject().value(QStringLiteral("label")).toString(); }
QString starterDescription(const QString &platform) { return manifest().value(platform).toObject().value(QStringLiteral("description")).toString(); }

Starter generateStarter(const QString &platform, const QString &presetId, const QVariantMap &overrides) {
    Starter result;
    const auto doc = manifest();
    const auto kind = doc.value(platform).toObject();
    if (kind.isEmpty()) { result.error = QStringLiteral("Unknown starter platform: ") + platform; return result; }
    if (!Preset::exists(presetId)) { result.error = QStringLiteral("Unknown preset: ") + presetId; return result; }
    result.unsupported = kind.value(QStringLiteral("unsupported")).toObject().value(presetId).toString();
    if (!result.unsupported.isEmpty()) return result;

    // The changed choices in presets.json order, written as a TypeScript object or a QVariantMap initializer.
    const Preset preset = Preset::find(presetId);
    PresetChoices choices = preset.choices;
    QStringList changed;
    const bool web = platform == QStringLiteral("web");
    for (const auto &info : Preset::choiceInfo()) {
        if (!overrides.contains(info.id)) continue;
        PresetChoices candidate = preset.choices;
        if (!candidate.setValue(info.id, overrides.value(info.id)) || candidate.value(info.id) == preset.choices.value(info.id)) continue;
        choices.setValue(info.id, overrides.value(info.id));
        const QVariant value = candidate.value(info.id);
        const bool isBool = info.isSwitch();
        const QString text = isBool ? (value.toBool() ? QStringLiteral("true") : QStringLiteral("false"))
            : web ? QStringLiteral("'%1'").arg(value.toString()) : QStringLiteral("QStringLiteral(\"%1\")").arg(value.toString());
        changed << (web ? QStringLiteral("%1: %2").arg(info.id, text) : QStringLiteral("{QStringLiteral(\"%1\"), %2}").arg(info.id, text));
    }
    const QString literal = changed.isEmpty() ? QStringLiteral("{}") : web ? QStringLiteral("{ %1 }").arg(changed.join(QStringLiteral(", "))) : QStringLiteral("{%1}").arg(changed.join(QStringLiteral(", ")));
    const QString version = QStringLiteral(STACKJUNKIE_VERSION);
    const auto core = QRegularExpression(QStringLiteral("^\\d+\\.\\d+\\.\\d+")).match(version);
    const QHash<QString, QString> values{
        {QStringLiteral("version"), version}, {QStringLiteral("tag"), QStringLiteral("v") + version},
        {QStringLiteral("cmakeVersion"), core.hasMatch() ? core.captured(0) : version},
        {QStringLiteral("repository"), doc.value(QStringLiteral("repository")).toString()},
        {QStringLiteral("preset"), preset.id}, {QStringLiteral("presetLabel"), preset.label},
        {QStringLiteral("overrides"), literal}, {QStringLiteral("theme"), choices.theme}, {QStringLiteral("density"), choices.density},
        {QStringLiteral("sections"), sectionsCode(web, doc.value(QStringLiteral("structure")).toObject())}};

    QJsonObject files = kind.value(QStringLiteral("files")).toObject();
    const auto presetFiles = kind.value(QStringLiteral("presetFiles")).toObject().value(presetId).toObject();
    for (auto it = presetFiles.begin(); it != presetFiles.end(); ++it) files.insert(it.key(), it.value());
    static const QRegularExpression placeholder(QStringLiteral("\\{\\{(\\w+)\\}\\}"));
    for (auto it = files.begin(); it != files.end(); ++it) {
        const QString source = it.value().toString();
        const QString text = readTemplate(source, &result.error);
        if (!result.error.isEmpty()) return result;
        QString filled;
        qsizetype at = 0;
        for (auto match = placeholder.globalMatch(text); match.hasNext();) {
            const auto found = match.next();
            if (!values.contains(found.captured(1))) { result.error = QStringLiteral("Unknown placeholder %1 in %2").arg(found.captured(0), source); return result; }
            filled += text.mid(at, found.capturedStart() - at) + values.value(found.captured(1));
            at = found.capturedEnd();
        }
        filled += text.mid(at);
        result.files.append({it.key(), filled});
    }
    return result;
}

QString writeStarter(const Starter &starter, const QString &directory) {
    const QDir root(directory);
    for (const auto &file : starter.files) {
        const QString path = root.filePath(file.path);
        if (!QDir().mkpath(QFileInfo(path).absolutePath())) return QStringLiteral("Cannot create the folder for ") + file.path;
        QFile out(path);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate) || out.write(file.text.toUtf8()) < 0) return QStringLiteral("Cannot write ") + file.path;
    }
    return {};
}
