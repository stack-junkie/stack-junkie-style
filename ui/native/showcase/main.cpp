#include "gallery.hpp"
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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QListWidget>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QShortcut>
#include <cstdio>
#include <cstring>
#include <exception>

using namespace StackJunkie;
// A settings row like the web SettingsRow: label and visible description on the left, the control on the right.
static QWidget *settingsRow(const QString &label, const QString &description, QWidget *control, QLabel **descriptionLabel = nullptr) {
    auto *result = new QWidget;
    auto *layout = new QHBoxLayout(result); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(Theme::spacing(3));
    auto *text = new QVBoxLayout; text->setSpacing(2);
    auto *caption = new QLabel(label); caption->setBuddy(control);
    auto *help = new QLabel(description); help->setProperty("sjRole", "muted"); help->setWordWrap(true);
    text->addWidget(caption); text->addWidget(help);
    layout->addLayout(text, 1); layout->addWidget(control, 0, Qt::AlignVCenter);
    control->setAccessibleName(label); control->setAccessibleDescription(description);
    if (descriptionLabel) *descriptionLabel = help;
    return result;
}
int main(int argc, char *argv[]) {
    bool exporting = false;
    for (int i = 1; i < argc; ++i) if (std::strcmp(argv[i], "--export-preview") == 0 || std::strncmp(argv[i], "--export-preview=", 17) == 0) exporting = true;
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
    parser.addOption({QStringLiteral("theme"), QStringLiteral("light or dark"), QStringLiteral("theme")});
    parser.addOption({QStringLiteral("page"), QStringLiteral("Family ID, family/component ID, home, settings or settings/<section>"), QStringLiteral("page")});
    parser.addOption({QStringLiteral("choice"), QStringLiteral("Change one preset choice, such as sidebar=list, the same way Settings does. Repeatable."), QStringLiteral("name=value")});
    parser.addOption({QStringLiteral("preset"), QStringLiteral("App frame preset: web-app, desktop-app, document-editor or website. Saved choices are not used."), QStringLiteral("id")});    parser.addOption({QStringLiteral("width"), QStringLiteral("Export width, at least 480 pixels."), QStringLiteral("pixels"), QStringLiteral("1440")});
    parser.addOption({QStringLiteral("height"), QStringLiteral("Export height, at least 460 pixels."), QStringLiteral("pixels"), QStringLiteral("1000")});
    parser.process(app);
    try {
        initializeResources();
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

        QStringList pageIds;
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
        settingsWindow->addSection(QStringLiteral("Keyboard"), keyboardPage, QStringLiteral("shortcuts keys resize"));
        auto *aboutLogo = new QLabel; aboutLogo->setFixedSize(44, 44); aboutLogo->setProperty("sjBrandSize", 44); aboutLogo->setAccessibleName(QStringLiteral("Stack Junkie"));
        settingsWindow->addSection(QStringLiteral("About"), new AboutPage(QStringLiteral("Stack Junkie UI"), QStringLiteral("A living collection of reusable interface components, layouts and interactions."), QStringLiteral(STACKJUNKIE_VERSION), aboutLogo), QStringLiteral("version copyright"));
        auto openSettings = [settingsWindow] { settingsWindow->show(); settingsWindow->raise(); settingsWindow->activateWindow(); };
        window.setSettingsHandler(openSettings);
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
        pageIds << QStringLiteral("home");
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
        pageIds << QStringLiteral("stub");
#endif
        QObject::connect(homeButton, &QPushButton::clicked, &window, [&window, homeIndex] { window.setCurrentPage(homeIndex); });
        window.setCurrentPage(homeIndex);
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
        for (const auto &preset : Preset::all()) presetBox->addItem(preset.label, preset.id);
        QLabel *presetHelp = nullptr;
        layoutPage->contentLayout()->addWidget(settingsRow(QStringLiteral("Preset"), Preset::find(presetId).description, presetBox, &presetHelp));
        // Choices drawn by later components keep their switch so the preset is complete, and the note says what is missing.
        const QHash<QString, QString> notes{{QStringLiteral("menuBar"), QStringLiteral("Drawn in a later preview.")}, {QStringLiteral("statusBar"), QStringLiteral("Drawn in a later preview.")},
            {QStringLiteral("bottomPanel"), QStringLiteral("Drawn in a later preview.")}, {QStringLiteral("dashboardHome"), QStringLiteral("Drawn in a later preview.")},
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
        auto syncLayout = [&] {
            const auto choices = Preset::resolve(presetId, overrides);
            { QSignalBlocker blocker(presetBox); presetBox->setCurrentIndex(presetBox->findData(presetId)); }
            presetHelp->setText(Preset::find(presetId).description);
            for (auto it = controls.begin(); it != controls.end(); ++it) {
                QSignalBlocker blocker(it.value());
                if (auto *check = qobject_cast<QCheckBox *>(it.value())) check->setChecked(choices.value(it.key()).toBool());
                else if (auto *combo = qobject_cast<QComboBox *>(it.value())) combo->setCurrentIndex(combo->findData(choices.value(it.key()).toString()));
            }
            reset->setText(QStringLiteral("Reset to ") + Preset::find(presetId).label);
            reset->setVisible(!overrides.isEmpty());
        };
        auto applyLayout = [&] {
            window.setChoices(Preset::resolve(presetId, overrides));
            syncLayout();
            if (!exporting) { settings.setValue(QStringLiteral("layout/preset"), presetId); settings.setValue(QStringLiteral("layout/choices"), overrides); }
        };
        // The theme choice also switches the showcase's own theme; "system" follows the device.
        auto showTheme = [&](const QString &theme) { mode->setCurrentIndex(prefersDark(theme) ? 1 : 0); };
        auto choosePreset = [&](const QString &id) { presetId = id; overrides.clear(); applyLayout(); showTheme(Preset::find(id).choices.theme); };
        setChoice = [&](const QString &id, const QVariant &value) {
            auto choices = Preset::resolve(presetId, overrides);
            if (!choices.setValue(id, value)) return;
            overrides = Preset::overrides(presetId, choices);
            applyLayout();
            if (id == QStringLiteral("theme")) showTheme(value.toString());
        };
        QObject::connect(presetBox, &QComboBox::currentIndexChanged, &window, [&](int) { choosePreset(presetBox->currentData().toString()); });
        QObject::connect(reset, &QPushButton::clicked, &window, [&] { choosePreset(presetId); });
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
            QTimer::singleShot(150, &window, [&window, destination, &app, settingsWindow, exportSettings] {
                const bool saved = (exportSettings ? settingsWindow->grab() : window.grab()).save(destination, "PNG");
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
