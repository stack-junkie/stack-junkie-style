#include "gallery.hpp"
#include <stackjunkie/components.hpp>
#include <stackjunkie/version.hpp>
#include <QApplication>
#include <QCommandLineParser>
#include <QComboBox>
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
    parser.addOption({QStringLiteral("page"), QStringLiteral("Family ID, family/component ID, home or settings"), QStringLiteral("page")});
    parser.addOption({QStringLiteral("width"), QStringLiteral("Export width, at least 480 pixels."), QStringLiteral("pixels"), QStringLiteral("1440")});
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
        auto *keyboardPage = new SettingsPage(QStringLiteral("Keyboard"));
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
        const QString chosenTheme = parser.isSet(QStringLiteral("theme")) ? parser.value(QStringLiteral("theme")) : settings.value(QStringLiteral("theme"), QStringLiteral("dark")).toString();
        if (chosenTheme != QStringLiteral("light") && chosenTheme != QStringLiteral("dark")) { std::fprintf(stderr, "Unknown theme\n"); return 2; }
        mode->setCurrentIndex(chosenTheme == QStringLiteral("dark") ? 1 : 0);
        const bool exportSettings = parser.value(QStringLiteral("page")) == QStringLiteral("settings");
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
            if (exportSettings) settingsWindow->show();
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
