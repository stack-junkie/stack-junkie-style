#pragma once
#include <QList>
#include <QString>
#include <QVariantMap>

// Showcase only: Export's starter generator. It fills the shared templates in ui/shared/starters
// (embedded under :/starters) exactly as starters.mjs does for the web showcase, so both write the same files.
struct StarterFile { QString path; QString text; };
struct Starter {
    QList<StarterFile> files;
    /** Why the preset has no starter on this platform, such as Website on native. Empty otherwise. */ QString unsupported;
    /** A broken request or template. Empty on success. */ QString error;
};
/** platform is "web" or "native". Overrides that are invalid or match the preset are left out. */
Starter generateStarter(const QString &platform, const QString &presetId, const QVariantMap &overrides);
/** Writes every file under directory, creating folders. Returns an error message, or an empty string on success. */
QString writeStarter(const Starter &starter, const QString &directory);
/** The button label and short description of a platform's starter, from starters.json. */
QString starterLabel(const QString &platform);
QString starterDescription(const QString &platform);
