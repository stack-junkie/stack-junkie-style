#include "gallery.hpp"
#include "starter.hpp"
#include <stackjunkie/components.hpp>
#include <stackjunkie/version.hpp>
#include <QApplication>
#include <QCommandLineParser>
#include <QCheckBox>
#include <QComboBox>
#include <QGuiApplication>
#include <QStyleHints>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QRegularExpression>
#include <QStyle>
#include <QVBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QSettings>
#include <QTimer>
#include <QShortcut>
#include <cstdio>
#include <cstring>
#include <exception>

using namespace StackJunkie;
// A settings row like the web SettingsRow: label and visible description on the left, the control on the right.
static QWidget *settingsRow(const QString &label, const QString &description, QWidget *control, QLabel **descriptionLabel = nullptr, QLabel **captionLabel = nullptr) {
    auto *result = new QWidget;
    auto *layout = new QHBoxLayout(result); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(Theme::spacing(3));
    auto *text = new QVBoxLayout; text->setSpacing(2);
    auto *caption = new QLabel(label); caption->setBuddy(control);
    auto *help = new QLabel(description); help->setProperty("sjRole", "muted"); help->setWordWrap(true);
    text->addWidget(caption); text->addWidget(help);
    layout->addLayout(text, 1); layout->addWidget(control, 0, Qt::AlignVCenter);
    control->setAccessibleName(label); control->setAccessibleDescription(description);
    if (descriptionLabel) *descriptionLabel = help;
    if (captionLabel) *captionLabel = caption;
    return result;
}
// Several controls side by side in one settings row.
static QWidget *controlGroup(const QList<QWidget *> &widgets) {
    auto *group = new QWidget;
    auto *layout = new QHBoxLayout(group); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(Theme::spacing(2));
    for (auto *widget : widgets) layout->addWidget(widget);
    return group;
}
// Custom presets are saved as their preset file text, the same text Export file writes.
static QList<CustomPreset> savedCustomPresets(const QSettings &settings) {
    QList<CustomPreset> result;
    for (const auto &text : settings.value(QStringLiteral("layout/customPresets")).toStringList()) {
        CustomPreset preset;
        if (CustomPreset::parse(text.toUtf8(), &preset).isEmpty()) result << preset;
    }
    return result;
}
static QString readPresetFile(const QString &path, CustomPreset *preset) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return QStringLiteral("The file could not be read.");
    return CustomPreset::parse(file.readAll(), preset);
}
static const QString writeFailed = QStringLiteral("The preset file could not be written.");
static QString writePresetFile(const QString &path, const CustomPreset &preset) {
    const QByteArray text = preset.toFile();
    if (text.isEmpty()) return CustomPreset::check(preset);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) || file.write(text) != text.size()) return writeFailed;
    return {};
}
static bool sameName(const QString &a, const QString &b) { return a.trimmed().compare(b.trimmed(), Qt::CaseInsensitive) == 0; }
int main(int argc, char *argv[]) {
    bool exporting = false;
    for (int i = 1; i < argc; ++i) if (std::strcmp(argv[i], "--export-preview") == 0 || std::strncmp(argv[i], "--export-preview=", 17) == 0
        || std::strcmp(argv[i], "--export-starter") == 0 || std::strncmp(argv[i], "--export-starter=", 17) == 0
        || std::strcmp(argv[i], "--export-preset") == 0 || std::strncmp(argv[i], "--export-preset=", 16) == 0) exporting = true;
    if (exporting && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("StackJunkie"));
    app.setApplicationName(QStringLiteral("ComponentShowcase"));
    app.setApplicationVersion(QStringLiteral(STACKJUNKIE_VERSION));
    app.setStyle(QStringLiteral("Fusion"));
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Stack Junkie native component library showcase"));
    parser.addHelpOption(); parser.addVersionOption();
    parser.addOption({QStringLiteral("export-preview"), QStringLiteral("Render a PNG using the offscreen platform, then exit."), QStringLiteral("file")});
    parser.addOption({QStringLiteral("export-starter"), QStringLiteral("Write the starter project for the preset and choices into a folder, then exit."), QStringLiteral("folder")});
    parser.addOption({QStringLiteral("starter"), QStringLiteral("Starter for --export-starter: web or native."), QStringLiteral("platform"), QStringLiteral("web")});
    parser.addOption({QStringLiteral("theme"), QStringLiteral("light or dark"), QStringLiteral("theme")});
    parser.addOption({QStringLiteral("page"), QStringLiteral("Family ID, family/component ID, home, settings or settings/<section>"), QStringLiteral("page")});
    parser.addOption({QStringLiteral("menu"), QStringLiteral("Show one Menu bar menu open in the export: file, edit, view, library or help."), QStringLiteral("menu")});
    parser.addOption({QStringLiteral("choice"), QStringLiteral("Change one preset choice, such as sidebar=list, the same way Settings does. Repeatable."), QStringLiteral("name=value")});
    parser.addOption({QStringLiteral("preset"), QStringLiteral("App frame preset: web-app, desktop-app, document-editor or website. Saved choices are not used."), QStringLiteral("id")});
    parser.addOption({QStringLiteral("import-preset"), QStringLiteral("Import a custom preset file and use it, the same way Settings > Layout > Import file does. Not with --preset."), QStringLiteral("file")});
    parser.addOption({QStringLiteral("export-preset"), QStringLiteral("Write the current preset and choices as a custom preset file, then exit."), QStringLiteral("file")});
    parser.addOption({QStringLiteral("preset-name"), QStringLiteral("Name for --export-preset. Defaults to the imported or selected custom preset's name, else Custom preset."), QStringLiteral("name")});
    parser.addOption({QStringLiteral("width"), QStringLiteral("Export width, at least 480 pixels."), QStringLiteral("pixels"), QStringLiteral("1440")});
    parser.addOption({QStringLiteral("height"), QStringLiteral("Export height, at least 460 pixels."), QStringLiteral("pixels"), QStringLiteral("1000")});
    parser.process(app);
    try {
        initializeResources();
        if (parser.isSet(QStringLiteral("import-preset")) && parser.isSet(QStringLiteral("preset"))) { std::fprintf(stderr, "Use --import-preset or --preset, not both\n"); return 2; }
        // An imported preset file is checked first: a bad file stops here with its message, before anything is applied or written.
        CustomPreset imported;
        if (parser.isSet(QStringLiteral("import-preset"))) {
            const QString error = readPresetFile(parser.value(QStringLiteral("import-preset")), &imported);
            if (!error.isEmpty()) { std::fprintf(stderr, "Preset file not imported: %s\n", qPrintable(error)); return 2; }
        }
        if (parser.isSet(QStringLiteral("export-starter")) || parser.isSet(QStringLiteral("export-preset"))) {
            // The same files Settings > Layout writes: an imported preset, --preset alone, or the saved preset and choices, then each --choice.
            QSettings saved;
            const bool fromImport = parser.isSet(QStringLiteral("import-preset"));
            const QString id = fromImport ? imported.base : parser.isSet(QStringLiteral("preset")) ? parser.value(QStringLiteral("preset")) : saved.value(QStringLiteral("layout/preset"), QStringLiteral("web-app")).toString();
            if (!Preset::exists(id)) { std::fprintf(stderr, "Unknown preset\n"); return 2; }
            auto choices = Preset::resolve(id, fromImport ? imported.choices : parser.isSet(QStringLiteral("preset")) ? QVariantMap() : saved.value(QStringLiteral("layout/choices")).toMap());
            for (const auto &assignment : parser.values(QStringLiteral("choice")))
                if (!choices.setValue(assignment.section(QLatin1Char('='), 0, 0), assignment.section(QLatin1Char('='), 1))) { std::fprintf(stderr, "Unknown choice or value\n"); return 2; }
            if (parser.isSet(QStringLiteral("export-preset"))) {
                const QString savedName = fromImport || parser.isSet(QStringLiteral("preset")) ? QString() : saved.value(QStringLiteral("layout/custom")).toString();
                const QString name = parser.isSet(QStringLiteral("preset-name")) ? parser.value(QStringLiteral("preset-name")) : fromImport ? imported.name : savedName.isEmpty() ? QStringLiteral("Custom preset") : savedName;
                const CustomPreset preset{name, id, Preset::overrides(id, choices)};
                const QString path = parser.value(QStringLiteral("export-preset"));
                const QString error = writePresetFile(path, preset);
                if (!error.isEmpty()) { std::fprintf(stderr, "%s\n", qPrintable(error)); return error == writeFailed ? 3 : 2; }
                std::printf("Wrote preset file %s\n", qPrintable(QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath())));
                if (!parser.isSet(QStringLiteral("export-starter"))) return 0;
            }
            const auto starter = generateStarter(parser.value(QStringLiteral("starter")), id, Preset::overrides(id, choices));
            if (!starter.error.isEmpty()) { std::fprintf(stderr, "%s\n", qPrintable(starter.error)); return 2; }
            if (!starter.unsupported.isEmpty()) { std::fprintf(stderr, "%s\n", qPrintable(starter.unsupported)); return 2; }
            const QString error = writeStarter(starter, parser.value(QStringLiteral("export-starter")));
            if (!error.isEmpty()) { std::fprintf(stderr, "%s\n", qPrintable(error)); return 3; }
            std::printf("Wrote %d files to %s\n", int(starter.files.size()), qPrintable(QDir::toNativeSeparators(QFileInfo(parser.value(QStringLiteral("export-starter"))).absoluteFilePath())));
            return 0;
        }
        Workspace window;
        window.setWindowTitle(QStringLiteral("Stack Junkie Components %1").arg(QStringLiteral(STACKJUNKIE_VERSION)));
        window.setHeader(QStringLiteral("Stack Junkie UI"));
        window.resize(1280, 900);
        window.setMinimumSize(480, 460);
        QSettings settings;
        auto *mode = new QComboBox;
        mode->setAccessibleName(QStringLiteral("Color theme"));
        mode->addItems({QStringLiteral("Light"), QStringLiteral("Dark")});
        auto *toolbar = qobject_cast<QHBoxLayout *>(window.toolbar()->layout());
        auto *brandIcon = new QLabel;
        brandIcon->setAccessibleName(QStringLiteral("Stack Junkie"));
        const int brandSize = Theme::density(QStringLiteral("brandMarkSize"), 25);
        brandIcon->setFixedSize(brandSize, brandSize);
        brandIcon->setProperty("sjBrandSize", brandSize);
        auto *homeButton = new Button(QString(), ButtonVariant::Quiet);
        homeButton->setAccessibleName(QStringLiteral("Home")); homeButton->setToolTip(QStringLiteral("Home"));
        auto *homeBrandLayout = new QHBoxLayout(homeButton); homeBrandLayout->setContentsMargins(2, 2, 2, 2);
        brandIcon->setAttribute(Qt::WA_TransparentForMouseEvents); homeBrandLayout->addWidget(brandIcon);
        toolbar->insertWidget(0, homeButton);

        QStringList pageIds, pageNames, pageFamilies;
        const auto families = Theme::catalog().value(QStringLiteral("families")).toArray();
        for (const auto &value : families) {
            const auto family = value.toObject();
            const auto familyId = family.value(QStringLiteral("id")).toString();
            // Settings owns the only gear, so Product layouts uses its own icon.
            const QString iconName = family.value(QStringLiteral("icon")).toString(QStringLiteral("grid"));
            const int section = window.addSection(family.value(QStringLiteral("label")).toString(), iconName);
            if (familyId == QStringLiteral("foundations")) window.setSectionVisible(section, false);
            for (const auto &entry : family.value(QStringLiteral("components")).toArray()) {
                const auto spec = entry.toObject();
                auto one = family; one.insert(QStringLiteral("components"), QJsonArray{spec});
                const QString id = spec.value(QStringLiteral("id")).toString();
                const int page = window.addPage(spec.value(QStringLiteral("label")).toString(), makeFamily(one), QStringLiteral("file"), section);
                Q_UNUSED(page);
                pageIds << familyId + QStringLiteral("/") + id;
                pageNames << spec.value(QStringLiteral("label")).toString(); pageFamilies << family.value(QStringLiteral("label")).toString();
            }
        }
        // Settings is a window over the app, never a page: section list and search, changes apply immediately.
        auto *settingsWindow = new SettingsWindow(&window);
        auto *appearancePage = new SettingsPage(QStringLiteral("Appearance"));
        appearancePage->contentLayout()->addWidget(new Field(QStringLiteral("Theme"), mode, QStringLiteral("Choose the light or dark appearance.")));
        settingsWindow->addSection(QStringLiteral("Appearance"), appearancePage, QStringLiteral("theme dark light mode color"));
        auto *layoutPage = new SettingsPage(QStringLiteral("Layout"));
        settingsWindow->addSection(QStringLiteral("Layout"), layoutPage, QStringLiteral("preset rail sidebar menu status panel tabs command palette theme density dashboard"));        auto *keyboardPage = new SettingsPage(QStringLiteral("Keyboard"));
        keyboardPage->contentLayout()->addWidget(paragraph(QStringLiteral("Drag a sidebar divider to resize it. Arrow keys adjust a focused divider.")));
        keyboardPage->contentLayout()->addWidget(paragraph(QStringLiteral("Control comma opens Settings. In a row of open documents, Control W closes the current tab.")));
        keyboardPage->contentLayout()->addWidget(paragraph(QStringLiteral("With the Menu bar on, F10 opens the first menu and arrow keys move through the menus. Control B shows or hides the sidebar and Control J the bottom panel.")));
        settingsWindow->addSection(QStringLiteral("Keyboard"), keyboardPage, QStringLiteral("shortcuts keys resize"));
        auto *aboutLogo = new QLabel; aboutLogo->setFixedSize(44, 44); aboutLogo->setProperty("sjBrandSize", 44); aboutLogo->setAccessibleName(QStringLiteral("Stack Junkie"));
        settingsWindow->addSection(QStringLiteral("About"), new AboutPage(QStringLiteral("Stack Junkie UI"), QStringLiteral("A living collection of reusable interface components, layouts and interactions."), QStringLiteral(STACKJUNKIE_VERSION), aboutLogo), QStringLiteral("version copyright"));
        auto openSettings = [settingsWindow] { settingsWindow->show(); settingsWindow->raise(); settingsWindow->activateWindow(); };
        window.setSettingsHandler(openSettings);
        // Help > About opens Settings at About; Help > Keyboard shortcuts at Keyboard.
        window.setAboutHandler([settingsWindow, openSettings] { settingsWindow->showSection(3); openSettings(); });
        QObject::connect(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma), &window), &QShortcut::activated, &window, openSettings);
        const int homeSection = window.addSection(QStringLiteral("Home"));
        window.setSectionVisible(homeSection, false);
        auto *homePage = new SettingsPage(QStringLiteral("Stack Junkie UI"));
        homePage->contentLayout()->addWidget(paragraph(QStringLiteral("Explore the shared components, layouts and foundations.")));
        for (int familyIndex = 0; familyIndex < families.size(); ++familyIndex) {
            auto *openFamily = new Button(families[familyIndex].toObject().value(QStringLiteral("label")).toString());
            homePage->contentLayout()->addWidget(openFamily);
            QObject::connect(openFamily, &QPushButton::clicked, &window, [&window, familyIndex] { window.setCurrentSection(familyIndex); });
        }
        const int homeIndex = window.addPage(QStringLiteral("Overview"), homePage, QStringLiteral("grid"), homeSection);
        pageIds << QStringLiteral("home"); pageNames << QStringLiteral("Home"); pageFamilies << QStringLiteral("Stack Junkie UI");
#if STACKJUNKIE_STUBS_ALLOWED
        // Development builds only. A release build refuses to compile a Stub, so the release showcase has none.
        auto *stubPage = new SettingsPage(QStringLiteral("Stub"));
        stubPage->contentLayout()->addWidget(paragraph(QStringLiteral("Development stand-in content is striped. Release builds refuse to finish while any Stub remains.")));
        stubPage->contentLayout()->addWidget(STACKJUNKIE_STUB(QStringLiteral("Release notes go here once the first release exists.")));
        stubPage->contentLayout()->addWidget(STACKJUNKIE_STUB(new Field(QStringLiteral("Owner"), new QLineEdit(QStringLiteral("Sample owner")))));
        auto *hinted = new QLineEdit; hinted->setPlaceholderText(QStringLiteral("e.g. Field notes"));
        stubPage->contentLayout()->addWidget(new Field(QStringLiteral("Project name"), hinted, QStringLiteral("Hint text is not a Stub: it stays plain.")));
        stubPage->contentLayout()->addStretch();
        window.addPage(QStringLiteral("Stub"), stubPage, QStringLiteral("file"), homeSection);
        pageIds << QStringLiteral("stub"); pageNames << QStringLiteral("Stub"); pageFamilies << QStringLiteral("Stack Junkie UI");
#endif
        QObject::connect(homeButton, &QPushButton::clicked, &window, [&window, homeIndex] { window.setCurrentPage(homeIndex); });
        // Menu bar items the showcase adds: the frame already owns Settings, About and the View toggles.
        window.addMenuItem(window.editMenu(), QStringLiteral("Copy component name"), [&window, &pageNames] { QGuiApplication::clipboard()->setText(pageNames.value(window.currentPage())); });
        auto *libraryMenu = window.addMenu(QStringLiteral("Library"));
        for (int familyIndex = 0; familyIndex < families.size(); ++familyIndex)
            window.addMenuItem(libraryMenu, families[familyIndex].toObject().value(QStringLiteral("label")).toString(), [&window, familyIndex] { window.setCurrentSection(familyIndex); });
        window.addMenuItem(window.helpMenu(), QStringLiteral("Keyboard shortcuts"), [settingsWindow, openSettings] { settingsWindow->showSection(2); openSettings(); });
        window.viewMenu()->addSeparator();
        auto *darkTheme = window.viewMenu()->addAction(QStringLiteral("Dark theme")); darkTheme->setCheckable(true);
        QObject::connect(darkTheme, &QAction::triggered, mode, [mode](bool on) { mode->setCurrentIndex(on ? 1 : 0); });
        QObject::connect(mode, &QComboBox::currentIndexChanged, darkTheme, [darkTheme](int index) { darkTheme->setChecked(index == 1); });
        // Status bar: the current component on the left, its family and the library version on the right.
        // Bottom panel: the pages opened this session, and problems (none in a showcase page).
        auto *output = new QPlainTextEdit; output->setReadOnly(true); output->setProperty("sjFontRole", "mono"); output->setAccessibleName(QStringLiteral("Output"));
        window.bottomPanel()->addTab(output, QStringLiteral("Output"));
        window.bottomPanel()->addTab(paragraph(QStringLiteral("No problems in this view.")), QStringLiteral("Problems"));
        auto showCurrent = [&window, &pageNames, &pageFamilies, output](int index) {
            window.statusBar()->setState(pageNames.value(index));
            window.statusBar()->setFacts({pageFamilies.value(index), QStringLiteral(STACKJUNKIE_VERSION)});
            output->appendPlainText(QStringLiteral("Opened ") + pageNames.value(index));
        };
        QObject::connect(&window, &Workspace::currentChanged, &window, showCurrent);
        window.setCurrentPage(homeIndex);
        if (window.statusBar()->state().isEmpty()) showCurrent(homeIndex);
        if (!exporting) {
            window.restoreSettings(QStringLiteral("workspace"));
            // The previous preview stored a family index rather than a component index.
            // Retain that family, plus the existing splitter width and collapse preference.
            // Schema 3 removed the comparison page that sat before Home, so older indexes at or past it mean Home.
            const int schema = settings.value(QStringLiteral("workspace/schema"), 1).toInt();
            if (schema == 2 && settings.value(QStringLiteral("workspace/page"), 0).toInt() >= homeIndex) window.setCurrentPage(homeIndex);
            if (schema < 2) {
                const int oldPage = settings.value(QStringLiteral("workspace/page"), 0).toInt();
                if (oldPage >= 0 && oldPage < families.size()) {
                    const QString prefix = families[oldPage].toObject().value(QStringLiteral("id")).toString() + QStringLiteral("/");
                    for (int i = 0; i < pageIds.size(); ++i) if (pageIds[i].startsWith(prefix)) { window.setCurrentPage(i); break; }
                }
            }
        }
        // The preset sets the App frame. --preset starts from the preset alone; otherwise the saved preset and choices return.
        QString presetId = parser.isSet(QStringLiteral("preset")) ? parser.value(QStringLiteral("preset")) : settings.value(QStringLiteral("layout/preset"), QStringLiteral("web-app")).toString();
        if (!Preset::exists(presetId)) {
            if (parser.isSet(QStringLiteral("preset"))) { std::fprintf(stderr, "Unknown preset\n"); return 2; }
            presetId = QStringLiteral("web-app");
        }
        QVariantMap overrides = parser.isSet(QStringLiteral("preset")) ? QVariantMap() : Preset::overrides(presetId, Preset::resolve(presetId, settings.value(QStringLiteral("layout/choices")).toMap()));
        // Custom presets are listed after the four presets. A selected one is a marker over its base preset and choices.
        QList<CustomPreset> customs = savedCustomPresets(settings);
        auto findCustom = [&customs](const QString &name) { for (int i = 0; i < customs.size(); ++i) if (customs[i].name == name) return i; return -1; };
        QString customName = parser.isSet(QStringLiteral("preset")) ? QString() : settings.value(QStringLiteral("layout/custom")).toString();
        if (findCustom(customName) < 0 || customs[findCustom(customName)].base != presetId) customName.clear();
        auto saveCustoms = [&] {
            if (exporting) return;
            QStringList texts;
            for (const auto &custom : customs) texts << QString::fromUtf8(custom.toFile());
            settings.setValue(QStringLiteral("layout/customPresets"), texts);
        };
        // Import adds the preset and selects it. A preset with the same name is replaced, so a file moved between machines updates it.
        auto addImported = [&](const CustomPreset &preset) {
            bool replaced = false;
            for (int i = customs.size() - 1; i >= 0; --i) if (sameName(customs[i].name, preset.name)) { customs.removeAt(i); replaced = true; }
            customs << preset;
            saveCustoms();
            return replaced;
        };
        QString startMessage;
        if (parser.isSet(QStringLiteral("import-preset"))) {
            const bool replaced = addImported(imported);
            presetId = imported.base; overrides = imported.choices; customName = imported.name;
            startMessage = (replaced ? QStringLiteral("Replaced ") : QStringLiteral("Imported ")) + imported.name + QStringLiteral(".");
        }
        window.setChoices(Preset::resolve(presetId, overrides));
        const QString presetTheme = Preset::resolve(presetId, overrides).theme;
        const QString savedTheme = settings.value(QStringLiteral("theme"), prefersDark(presetTheme) ? QStringLiteral("dark") : QStringLiteral("light")).toString();
        const QString chosenTheme = parser.isSet(QStringLiteral("theme")) ? parser.value(QStringLiteral("theme"))
            : parser.isSet(QStringLiteral("preset")) || presetTheme == QStringLiteral("system") ? (prefersDark(presetTheme) ? QStringLiteral("dark") : QStringLiteral("light")) : savedTheme;
        if (chosenTheme != QStringLiteral("light") && chosenTheme != QStringLiteral("dark")) { std::fprintf(stderr, "Unknown theme\n"); return 2; }
        mode->setCurrentIndex(chosenTheme == QStringLiteral("dark") ? 1 : 0);
        const QString requestedPage = parser.value(QStringLiteral("page"));
        const bool exportSettings = requestedPage == QStringLiteral("settings") || requestedPage.startsWith(QStringLiteral("settings/"));
        const QStringList settingsIds{QStringLiteral("appearance"), QStringLiteral("layout"), QStringLiteral("keyboard"), QStringLiteral("about")};
        const int settingsSection = requestedPage.startsWith(QStringLiteral("settings/")) ? settingsIds.indexOf(requestedPage.mid(9)) : 0;
        if (exportSettings && settingsSection < 0) { std::fprintf(stderr, "Unknown settings section\n"); return 2; }
        if (parser.isSet(QStringLiteral("page")) && !exportSettings) {
            const QString requested = parser.value(QStringLiteral("page"));
            int index = pageIds.indexOf(requested);
            if (index < 0) for (int i = 0; i < pageIds.size(); ++i) if (pageIds[i].startsWith(requested + QStringLiteral("/"))) { index = i; break; }
            if (index < 0) { std::fprintf(stderr, "Unknown page\n"); return 2; }
            window.setCurrentPage(index);
        }
        auto apply = [&] {
            const auto theme = Theme::load(QStringLiteral("clean"), mode->currentIndex() == 1);
            theme.apply(&window);
            window.setWindowIcon(QIcon(brandMark(theme, brandSize)));
            for (auto *colors : window.findChildren<QListWidget *>(QStringLiteral("semantic-colors"))) {
                colors->clear();
                for (const auto &key : theme.colors.keys()) {
                    auto *item = new QListWidgetItem(key + QStringLiteral("  ") + theme.color(key).name(), colors);
                    QPixmap swatch(24, 24); swatch.fill(theme.color(key)); item->setIcon(QIcon(swatch));
                }
            }
        };
        QObject::connect(mode, &QComboBox::currentIndexChanged, &window, apply);
        apply();

        // Settings > Layout: the preset picker and one control per choice, generated from presets.json and applied at once.
        layoutPage->contentLayout()->addWidget(new Notice(QStringLiteral("Showcase only. A preset sets the App frame for a new project; change any choice and the frame follows.")));
        auto *presetBox = new QComboBox; presetBox->setMinimumWidth(180);
        auto fillPresetBox = [&] {
            QSignalBlocker blocker(presetBox);
            presetBox->clear();
            for (const auto &preset : Preset::all()) presetBox->addItem(preset.label, preset.id);
            if (!customs.isEmpty()) presetBox->insertSeparator(presetBox->count());
            for (const auto &custom : customs) presetBox->addItem(custom.name + QStringLiteral(" (custom)"), QStringLiteral("custom:") + custom.name);
        };
        fillPresetBox();
        QLabel *presetHelp = nullptr;
        layoutPage->contentLayout()->addWidget(settingsRow(QStringLiteral("Preset"), Preset::find(presetId).description, presetBox, &presetHelp));
        // The selected custom preset's row, then Save as custom preset and Import preset file.
        auto *updateCustom = new Button(QStringLiteral("Update"));
        auto *renameCustom = new Button(QStringLiteral("Rename..."));
        auto *exportCustom = new Button(QStringLiteral("Export file..."));
        auto *deleteCustom = new Button(QStringLiteral("Delete"), ButtonVariant::Danger);
        QLabel *customCaption = nullptr, *customHelp = nullptr;
        auto *customRow = settingsRow(QStringLiteral("Custom preset"), QString(), controlGroup({updateCustom, renameCustom, exportCustom, deleteCustom}), &customHelp, &customCaption);
        layoutPage->contentLayout()->addWidget(customRow);
        auto *customNameEdit = new QLineEdit; customNameEdit->setPlaceholderText(QStringLiteral("e.g. My editor")); customNameEdit->setMaxLength(60); customNameEdit->setMinimumWidth(200);
        customNameEdit->setAccessibleName(QStringLiteral("Custom preset name"));
        auto *saveCustom = new Button(QStringLiteral("Save"));
        layoutPage->contentLayout()->addWidget(settingsRow(QStringLiteral("Save as custom preset"), QStringLiteral("Name the current preset and choices to pick them again. Saved on this computer."), controlGroup({customNameEdit, saveCustom})));
        auto *importCustom = new Button(QStringLiteral("Import file..."));
        layoutPage->contentLayout()->addWidget(settingsRow(QStringLiteral("Import preset file"), QStringLiteral("Add a custom preset from a preset file exported on web or native."), importCustom));
        auto *customMessage = new QLabel; customMessage->setWordWrap(true); customMessage->setVisible(false);
        layoutPage->contentLayout()->addWidget(customMessage);
        auto showMessage = [customMessage](const QString &text, bool error) {
            customMessage->setText(text);
            customMessage->setProperty("sjRole", error ? "error" : "muted");
            customMessage->style()->unpolish(customMessage); customMessage->style()->polish(customMessage);
            customMessage->setVisible(true);
        };
        auto changedText = [](int count) { return count == 0 ? QStringLiteral("no changed choices") : QStringLiteral("%1 changed %2").arg(count).arg(count == 1 ? QStringLiteral("choice") : QStringLiteral("choices")); };
        // Choices drawn by later components keep their switch so the preset is complete, and the note says what is missing.
        const QHash<QString, QString> notes{{QStringLiteral("dashboardHome"), QStringLiteral("Drawn in a later preview.")},
            {QStringLiteral("commandPalette"), QStringLiteral("Apps read this choice; the native showcase has no command palette yet.")}};
        QHash<QString, QWidget *> controls;
        std::function<void(const QString &, const QVariant &)> setChoice;
        for (const auto &info : Preset::choiceInfo()) {
            const QString description = notes.contains(info.id) ? info.description + QStringLiteral(" ") + notes.value(info.id) : info.description;
            QWidget *control = nullptr;
            if (info.isSwitch()) {
                auto *check = new QCheckBox; check->setProperty("sjVariant", "switch");
                QObject::connect(check, &QCheckBox::toggled, &window, [&setChoice, id = info.id](bool on) { setChoice(id, on); });
                control = check;
            } else {
                auto *combo = new QComboBox; combo->setMinimumWidth(180);
                for (const auto &option : info.options) { const QString value = option.toString(); combo->addItem(value.left(1).toUpper() + value.mid(1), value); }
                QObject::connect(combo, &QComboBox::currentIndexChanged, &window, [&setChoice, combo, id = info.id](int) { setChoice(id, combo->currentData()); });
                control = combo;
            }
            controls.insert(info.id, control);
            layoutPage->contentLayout()->addWidget(settingsRow(info.label, description, control));
        }
        auto *reset = new Button(QString());
        layoutPage->contentLayout()->addWidget(row({reset}));
        // Export writes a starter project for this preset and these choices into a folder the user picks.
        auto *starterBox = new QComboBox; starterBox->setMinimumWidth(180);
        for (const auto &platform : {QStringLiteral("web"), QStringLiteral("native")}) starterBox->addItem(starterLabel(platform), platform);
        QLabel *exportHelp = nullptr;
        const QString exportDescription = QStringLiteral("Write a starter project that installs the library at this version and uses this preset and these choices.");
        layoutPage->contentLayout()->addWidget(settingsRow(QStringLiteral("Export"), exportDescription, starterBox, &exportHelp));
        auto *exportButton = new Button(QStringLiteral("Export starter..."), ButtonVariant::Primary);
        auto *exportResult = new QLabel; exportResult->setProperty("sjRole", "muted"); exportResult->setWordWrap(true); exportResult->setVisible(false);
        layoutPage->contentLayout()->addWidget(row({exportButton}));
        layoutPage->contentLayout()->addWidget(exportResult);
        auto syncLayout = [&] {
            const auto choices = Preset::resolve(presetId, overrides);
            const int custom = findCustom(customName);
            const bool modified = custom >= 0 && (customs[custom].base != presetId || customs[custom].choices != overrides);
            { QSignalBlocker blocker(presetBox); presetBox->setCurrentIndex(presetBox->findData(custom >= 0 ? QStringLiteral("custom:") + customName : presetId)); }
            presetHelp->setText(custom >= 0 ? QStringLiteral("Your custom preset, based on %1.").arg(Preset::find(presetId).label) : Preset::find(presetId).description);
            customRow->setVisible(custom >= 0);
            if (custom >= 0) {
                customCaption->setText(customName);
                customHelp->setText(QStringLiteral("Based on %1, %2.%3").arg(Preset::find(customs[custom].base).label, changedText(int(customs[custom].choices.size())), modified ? QStringLiteral(" The current choices differ.") : QString()));
            }
            updateCustom->setEnabled(modified);
            for (auto it = controls.begin(); it != controls.end(); ++it) {
                QSignalBlocker blocker(it.value());
                if (auto *check = qobject_cast<QCheckBox *>(it.value())) check->setChecked(choices.value(it.key()).toBool());
                else if (auto *combo = qobject_cast<QComboBox *>(it.value())) combo->setCurrentIndex(combo->findData(choices.value(it.key()).toString()));
            }
            reset->setText(QStringLiteral("Reset to ") + (custom >= 0 ? customName : Preset::find(presetId).label));
            reset->setVisible(custom >= 0 ? modified : !overrides.isEmpty());
            const QString unsupported = generateStarter(starterBox->currentData().toString(), presetId, overrides).unsupported;
            exportHelp->setText(unsupported.isEmpty() ? exportDescription : unsupported);
            exportButton->setEnabled(unsupported.isEmpty());
        };
        auto applyLayout = [&] {
            window.setChoices(Preset::resolve(presetId, overrides));
            syncLayout();
            if (!exporting) { settings.setValue(QStringLiteral("layout/preset"), presetId); settings.setValue(QStringLiteral("layout/choices"), overrides); settings.setValue(QStringLiteral("layout/custom"), customName); }
        };
        // The theme choice also switches the showcase's own theme; "system" follows the device.
        auto showTheme = [&](const QString &theme) { mode->setCurrentIndex(prefersDark(theme) ? 1 : 0); };
        auto choosePreset = [&](const QString &id) { presetId = id; overrides.clear(); customName.clear(); applyLayout(); showTheme(Preset::find(id).choices.theme); };
        auto chooseCustom = [&](const QString &name) {
            const int index = findCustom(name);
            if (index < 0) return;
            presetId = customs[index].base; overrides = customs[index].choices; customName = name;
            applyLayout(); showTheme(customs[index].resolve().theme);
        };
        setChoice = [&](const QString &id, const QVariant &value) {
            auto choices = Preset::resolve(presetId, overrides);
            if (!choices.setValue(id, value)) return;
            overrides = Preset::overrides(presetId, choices);
            applyLayout();
            if (id == QStringLiteral("theme")) showTheme(value.toString());
        };
        QObject::connect(presetBox, &QComboBox::currentIndexChanged, &window, [&](int) {
            const QString value = presetBox->currentData().toString();
            if (value.startsWith(QStringLiteral("custom:"))) chooseCustom(value.mid(7)); else choosePreset(value);
        });
        QObject::connect(reset, &QPushButton::clicked, &window, [&] { if (findCustom(customName) >= 0) chooseCustom(customName); else choosePreset(presetId); });
        auto taken = [&](const QString &name, const QString &except) { for (const auto &c : customs) if (sameName(c.name, name) && !(!except.isEmpty() && sameName(c.name, except))) return true; return false; };
        auto saveNew = [&] {
            CustomPreset checked;
            const QString error = CustomPreset::check({customNameEdit->text(), presetId, overrides}, &checked);
            if (!error.isEmpty()) { showMessage(error, true); return; }
            if (taken(checked.name, QString())) { showMessage(QStringLiteral("A custom preset named \"%1\" already exists.").arg(checked.name), true); return; }
            customs << checked; customName = checked.name;
            saveCustoms(); fillPresetBox(); applyLayout();
            customNameEdit->clear(); showMessage(QStringLiteral("Saved %1.").arg(checked.name), false);
        };
        QObject::connect(saveCustom, &QPushButton::clicked, &window, saveNew);
        QObject::connect(customNameEdit, &QLineEdit::returnPressed, &window, saveNew);
        QObject::connect(updateCustom, &QPushButton::clicked, &window, [&] {
            const int index = findCustom(customName);
            CustomPreset checked;
            if (index < 0 || !CustomPreset::check({customName, presetId, overrides}, &checked).isEmpty()) return;
            customs[index] = checked; saveCustoms(); syncLayout(); showMessage(QStringLiteral("Updated %1.").arg(customName), false);
        });
        QObject::connect(renameCustom, &QPushButton::clicked, &window, [&] {
            const int index = findCustom(customName);
            if (index < 0) return;
            bool ok = false;
            const QString name = QInputDialog::getText(settingsWindow, QStringLiteral("Rename custom preset"), QStringLiteral("New name"), QLineEdit::Normal, customName, &ok);
            if (!ok) return;
            CustomPreset renamed = customs[index]; renamed.name = name;
            CustomPreset checked;
            const QString error = CustomPreset::check(renamed, &checked);
            if (!error.isEmpty()) { showMessage(error, true); return; }
            if (taken(checked.name, customName)) { showMessage(QStringLiteral("A custom preset named \"%1\" already exists.").arg(checked.name), true); return; }
            customs[index] = checked; customName = checked.name;
            saveCustoms(); fillPresetBox(); applyLayout(); showMessage(QStringLiteral("Renamed to %1.").arg(checked.name), false);
        });
        QObject::connect(exportCustom, &QPushButton::clicked, &window, [&] {
            const int index = findCustom(customName);
            if (index < 0) return;
            QString slug = customName.toLower();
            slug.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
            slug.remove(QRegularExpression(QStringLiteral("^-|-$")));
            const QString path = QFileDialog::getSaveFileName(settingsWindow, QStringLiteral("Export preset file"), (slug.isEmpty() ? QStringLiteral("custom") : slug) + QStringLiteral(".preset.json"), QStringLiteral("Preset files (*.json)"));
            if (path.isEmpty()) return;
            const QString error = writePresetFile(path, customs[index]);
            showMessage(error.isEmpty() ? QStringLiteral("Wrote %1.").arg(QDir::toNativeSeparators(path)) : error, !error.isEmpty());
        });
        QObject::connect(deleteCustom, &QPushButton::clicked, &window, [&] {
            const int index = findCustom(customName);
            if (index < 0 || QMessageBox::question(settingsWindow, QStringLiteral("Delete custom preset?"), QStringLiteral("Delete %1? The current choices stay as they are.").arg(customName)) != QMessageBox::Yes) return;
            const QString name = customName;
            customs.removeAt(index); customName.clear();
            saveCustoms(); fillPresetBox(); applyLayout(); showMessage(QStringLiteral("Deleted %1.").arg(name), false);
        });
        QObject::connect(importCustom, &QPushButton::clicked, &window, [&] {
            const QString path = QFileDialog::getOpenFileName(settingsWindow, QStringLiteral("Import preset file"), QString(), QStringLiteral("Preset files (*.json);;All files (*)"));
            if (path.isEmpty()) return;
            CustomPreset preset;
            const QString error = readPresetFile(path, &preset);
            if (!error.isEmpty()) { showMessage(QStringLiteral("Preset file not imported. ") + error, true); return; }
            const bool replaced = addImported(preset);
            fillPresetBox(); chooseCustom(preset.name);
            showMessage((replaced ? QStringLiteral("Replaced ") : QStringLiteral("Imported ")) + preset.name + QStringLiteral("."), false);
        });
        if (!startMessage.isEmpty()) showMessage(startMessage, false);
        QObject::connect(starterBox, &QComboBox::currentIndexChanged, &window, [&](int) { syncLayout(); exportResult->setVisible(false); });
        QObject::connect(exportButton, &QPushButton::clicked, &window, [&] {
            const QString platform = starterBox->currentData().toString();
            const auto starter = generateStarter(platform, presetId, overrides);
            if (!starter.error.isEmpty() || !starter.unsupported.isEmpty()) return;
            const QString folder = QFileDialog::getExistingDirectory(settingsWindow, QStringLiteral("Export %1 to a folder").arg(starterLabel(platform).toLower()));
            if (folder.isEmpty()) return;
            QStringList existing;
            for (const auto &file : starter.files) if (QFileInfo::exists(QDir(folder).filePath(file.path))) existing << file.path;
            if (!existing.isEmpty() && QMessageBox::question(settingsWindow, QStringLiteral("Replace files?"), QStringLiteral("The folder already has %1. Replace them?").arg(existing.join(QStringLiteral(", ")))) != QMessageBox::Yes) return;
            const QString error = writeStarter(starter, folder);
            exportResult->setText(error.isEmpty() ? QStringLiteral("Wrote %1 files to %2.").arg(starter.files.size()).arg(QDir::toNativeSeparators(folder)) : error);
            exportResult->setVisible(true);
        });
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, &window, [&] { if (Preset::resolve(presetId, overrides).theme == QStringLiteral("system")) showTheme(QStringLiteral("system")); });
#endif
        layoutPage->contentLayout()->addStretch();
        syncLayout();
        for (const auto &assignment : parser.values(QStringLiteral("choice"))) {
            const QString name = assignment.section(QLatin1Char('='), 0, 0), value = assignment.section(QLatin1Char('='), 1);
            if (!PresetChoices().setValue(name, value)) { std::fprintf(stderr, "Unknown choice or value\n"); return 2; }
            setChoice(name, value);
        }
        if (!exporting) QObject::connect(&app, &QCoreApplication::aboutToQuit, &window, [&] { window.saveSettings(QStringLiteral("workspace")); settings.setValue(QStringLiteral("workspace/schema"), 3); settings.setValue(QStringLiteral("theme"), mode->currentIndex() ? QStringLiteral("dark") : QStringLiteral("light")); });
        if (exporting) {
            const QString destination = QFileInfo(parser.value(QStringLiteral("export-preview"))).absoluteFilePath();
            if (!QDir().mkpath(QFileInfo(destination).absolutePath())) { std::fprintf(stderr, "Cannot create output directory\n"); return 3; }
            bool widthOk = false, heightOk = false;
            const int imageWidth = parser.value(QStringLiteral("width")).toInt(&widthOk);
            const int imageHeight = parser.value(QStringLiteral("height")).toInt(&heightOk);
            if (!widthOk || !heightOk || imageWidth < 480 || imageHeight < 460 || imageWidth > 8192 || imageHeight > 8192) { std::fprintf(stderr, "Invalid export dimensions\n"); return 2; }
            window.resize(imageWidth, imageHeight);
            window.show();
            if (exportSettings) {
                // A named section gets the export height, so a long section such as Layout fits.
                if (requestedPage.contains(QLatin1Char('/'))) settingsWindow->resize(760, imageHeight);
                settingsWindow->showSection(settingsSection);
                settingsWindow->show();
            }
            QMenu *openMenu = nullptr;
            if (parser.isSet(QStringLiteral("menu"))) {
                for (auto *action : window.menuBar()->actions())
                    if (action->menu() && action->isVisible() && action->text().compare(parser.value(QStringLiteral("menu")), Qt::CaseInsensitive) == 0) openMenu = action->menu();
                if (!openMenu || !window.menuBar()->isVisible()) { std::fprintf(stderr, "Unknown or hidden menu\n"); return 2; }
            }
            QTimer::singleShot(100, &window, [&window, openMenu] {
                if (!openMenu) return;
                window.menuBar()->setActiveAction(openMenu->menuAction());
            });
            QTimer::singleShot(300, &window, [&window, destination, &app, settingsWindow, exportSettings, openMenu] {
                QPixmap image = exportSettings ? settingsWindow->grab() : window.grab();
                if (openMenu && openMenu->isVisible()) {
                    QPainter painter(&image);
                    painter.drawPixmap(openMenu->pos() - window.mapToGlobal(QPoint(0, 0)), openMenu->grab());
                }
                const bool saved = image.save(destination, "PNG");
                if (!saved) std::fprintf(stderr, "Cannot save preview PNG\n");
                app.exit(saved ? 0 : 3);
            });
        } else window.show();
        return app.exec();
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
