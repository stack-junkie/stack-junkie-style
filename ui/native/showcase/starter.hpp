#pragma once
#include <QList>
#include <QString>
#include <QJsonObject>
#include <QVariantMap>

// Showcase only: Export's starter generator. It fills the shared templates in ui/shared/starters
// (embedded under :/starters) exactly as starters.mjs does for the web showcase, so both write the same files.
/** A text file, or the logo image's bytes in data. */
struct StarterFile { QString path; QString text; QByteArray data; };
struct Starter {
    QList<StarterFile> files;
    /** Why the preset has no starter on this platform, such as Website on native. Empty otherwise. */ QString unsupported;
    /** A broken request or template. Empty on success. */ QString error;
};
/** platform is "web" or "native". Overrides that are invalid or match the preset are left out. */
/** logoPath is an optional png, jpg, jpeg, svg or webp file for the logo slot. */
Starter generateStarter(const QString &platform, const QString &presetId, const QVariantMap &overrides, const QString &logoPath = {});
/** The starter's file name for a logo, such as logo.png, or an empty string and an error for another type. */
QString logoFileName(const QString &name, QString *error = nullptr);
/** Writes every file under directory, creating folders. Returns an error message, or an empty string on success. */
QString writeStarter(const Starter &starter, const QString &directory);
/** The `new` command that writes the same starter, pinned to this version's tag, exactly as starters.mjs writes it. */
QString starterCommand(const QString &platform, const QString &presetId, const QVariantMap &overrides, const QString &logoName = {}, const QString &folder = QStringLiteral("my-app"));
/** The starter structure from starters.json: areas with their pages, the bottom panel's tabs, the Website's articles. */
QJsonObject starterManifest();
/** The button label and short description of a platform's starter, from starters.json. */
QString starterLabel(const QString &platform);
QString starterDescription(const QString &platform);
