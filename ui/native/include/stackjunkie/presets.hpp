#pragma once
#include <stackjunkie/export.hpp>
#include <QList>
#include <QString>
#include <QVariant>
#include <QVariantMap>

// Presets are read from the embedded shared/presets.json, the only source of preset choices for web, native and the skill.
namespace StackJunkie {
/** One full set of App frame choices. Default values are what a Workspace without a preset shows: the rail and a list sidebar. */
struct STACKJUNKIE_EXPORT PresetChoices {
    bool rail = true;
    /** "tree", "list" or "off". */ QString sidebar = QStringLiteral("list");
    bool menuBar = false;
    bool statusBar = false;
    bool bottomPanel = false;
    bool documentTabs = false;
    bool commandPalette = true;
    /** "dark", "light" or "system". */ QString theme = QStringLiteral("dark");
    /** "compact" or "comfortable". */ QString density = QStringLiteral("compact");
    bool dashboardHome = false;

    /** The value of one choice by its presets.json name, such as "sidebar". Invalid for an unknown name. */
    QVariant value(const QString &choice) const;
    /** Sets one choice. Returns false, and changes nothing, for an unknown name or a value presets.json does not allow. */
    bool setValue(const QString &choice, const QVariant &value);
    QVariantMap toMap() const;
    bool operator==(const PresetChoices &other) const { return toMap() == other.toMap(); }
    bool operator!=(const PresetChoices &other) const { return !(*this == other); }
};
/** Label, description and allowed values of one choice, for building settings controls. */
struct STACKJUNKIE_EXPORT PresetChoiceInfo {
    QString id;
    QString label;
    QString description;
    QVariantList options;
    /** True for an on or off choice, false for a choice between named options. */ bool isSwitch() const;
};
struct STACKJUNKIE_EXPORT Preset {
    QString id;
    QString label;
    QString description;
    PresetChoices choices;

    /** Every preset, in presets.json order. The first is Web app. */ static QList<Preset> all();
    /** Every choice, in presets.json order. */ static QList<PresetChoiceInfo> choiceInfo();
    static bool exists(const QString &id);
    /** The preset with this id. Unknown ids fall back to Web app. */ static Preset find(const QString &id);
    /** The preset's choices with any overrides applied, such as {{"sidebar", "list"}}. Invalid overrides are ignored. */
    static PresetChoices resolve(const QString &id, const QVariantMap &overrides = {});
    /** Only the choices that differ from the preset, for saving a compact set of overrides. */
    static QVariantMap overrides(const QString &id, const PresetChoices &choices);
};
/**
 * A custom preset: a name, a base preset and only the choices that differ from it.
 * Its file is the same on web and native; the format and rules are in ui/shared/PRESET-FILE.md.
 */
struct STACKJUNKIE_EXPORT CustomPreset {
    QString name;
    /** A preset id, such as "desktop-app". */ QString base;
    /** Choice name to value, such as {{"sidebar", "tree"}}. Booleans must be bool, not text. */ QVariantMap choices;

    /** A plain error message for a name, or an empty string when it is usable. Names are trimmed first. */
    static QString nameError(const QString &name);
    /** Checks the preset. Returns an error message, or an empty string and, in *checked, the preset with its name trimmed and only the changed choices. */
    static QString check(const CustomPreset &preset, CustomPreset *checked = nullptr);
    /** Reads a preset file. Returns an error message and leaves *result unchanged, or an empty string and sets *result. */
    static QString parse(const QByteArray &text, CustomPreset *result);
    /** The preset file text: the same bytes web writes for this preset. Empty when check() fails. */
    QByteArray toFile() const;
    /** The base preset's choices with this preset's choices applied. */
    PresetChoices resolve() const { return Preset::resolve(base, choices); }
};
/** True when the device's color scheme is dark. Unknown schemes, such as the offscreen platform's, count as light. */
STACKJUNKIE_EXPORT bool systemPrefersDark();
/** Resolves a theme choice: "dark", "light", or "system", which follows the device. */
STACKJUNKIE_EXPORT bool prefersDark(const QString &themeChoice);
}
