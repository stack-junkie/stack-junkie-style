#include "builder.hpp"
#include "starter.hpp"
#include <stackjunkie/version.hpp>
#include <stackjunkie/presets.hpp>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QStyle>

using namespace StackJunkie;

// The element each choice changes, by the frame's object names. The "?" help outlines it.
static const QHash<QString, QString> targets{
    {QStringLiteral("rail"), QStringLiteral("sjRail")}, {QStringLiteral("sidebar"), QStringLiteral("sjSidebar")}, {QStringLiteral("menuBar"), QStringLiteral("sjMenuBar")},
    {QStringLiteral("statusBar"), QStringLiteral("sjStatusBar")}, {QStringLiteral("bottomPanel"), QStringLiteral("sjBottomPanel")}, {QStringLiteral("documentTabs"), QStringLiteral("sjTabRow")},
    {QStringLiteral("commandPalette"), QStringLiteral("sjHeader")}, {QStringLiteral("theme"), QString()}, {QStringLiteral("density"), QString()},
    {QStringLiteral("dashboardHome"), QStringLiteral("sjPages")}, {QStringLiteral("logo"), QStringLiteral("sjHome")}};
static QString optionText(const QVariant &value) { const QString text = value.toString(); return text.left(1).toUpper() + text.mid(1); }

// Draws a rectangle over the element a choice changes; mouse events pass through it.
class Outline : public QWidget {
public:
    explicit Outline(QWidget *parent) : QWidget(parent) { setAttribute(Qt::WA_TransparentForMouseEvents); setAttribute(Qt::WA_NoSystemBackground); hide(); }
    void showAround(const QRect &rect, const QColor &color) { area_ = rect; color_ = color; setGeometry(parentWidget()->rect()); raise(); show(); update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        QPainterPath dim; dim.addRect(rect()); QPainterPath hole; hole.addRect(area_); painter.fillPath(dim.subtracted(hole), QColor(0, 0, 0, 70));
        painter.setPen(QPen(color_, 2)); painter.drawRect(area_.adjusted(1, 1, -1, -1));
    }
private:
    QRect area_; QColor color_;
};

// The starter's own App frame from starters.json: two areas of three Stub pages and a Stub bottom panel tab.
static AppShell *starterFrame(const QString &presetId, const QVariantMap &overrides, const QString &logoPath, QWidget *parent) {
    auto *shell = new AppShell(QStringLiteral("My app"), QStringLiteral("What this app does."), QStringLiteral("0.1.0"), parent);
    const auto structure = starterManifest().value(QStringLiteral("structure")).toObject();
    for (const auto &areaValue : structure.value(QStringLiteral("areas")).toArray()) {
        const auto area = areaValue.toObject();
        const int section = shell->addSection(area.value(QStringLiteral("label")).toString(), area.value(QStringLiteral("icon")).toString());
        for (const auto &pageValue : area.value(QStringLiteral("pages")).toArray()) {
            const auto page = pageValue.toObject();
            // The showcase builds the Stub class directly, so it shows in Release builds too, as on the Stub page.
            shell->addPage(page.value(QStringLiteral("label")).toString(), new Stub(Stub::Site{__FILE__, __LINE__}, page.value(QStringLiteral("content")).toString()), QStringLiteral("file"), section);
        }
    }
    for (const auto &panelValue : structure.value(QStringLiteral("panels")).toArray()) {
        const auto panel = panelValue.toObject();
        shell->bottomPanel()->addTab(new Stub(Stub::Site{__FILE__, __LINE__}, panel.value(QStringLiteral("content")).toString()), panel.value(QStringLiteral("label")).toString());
    }
    if (!logoPath.isEmpty()) shell->setLogo(QPixmap(logoPath));
    shell->setPreset(presetId, overrides);
    return shell;
}

Builder::Builder(QWidget *parent) : QWidget(parent) {
    setWindowTitle(QStringLiteral("Builder - Stack Junkie UI"));
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(0);
    auto *bar = new QWidget(this); bar->setProperty("sjRole", "shellHeader"); bar_ = bar;
    auto *barLayout = new QHBoxLayout(bar); barLayout->setContentsMargins(8, 4, 8, 4); barLayout->setSpacing(8);
    auto *back = new Button(QStringLiteral("Back"), ButtonVariant::Quiet); back->setProperty("sjIcon", "chevron-left");
    connect(back, &QPushButton::clicked, this, &Builder::backRequested);
    barLayout->addWidget(back, 0, Qt::AlignTop);
    presetBox_ = new QComboBox; presetBox_->setAccessibleName(QStringLiteral("Preset")); presetBox_->setMinimumWidth(170);
    connect(presetBox_, &QComboBox::currentIndexChanged, this, [this](int) {
        const QString value = presetBox_->currentData().toString();
        if (value.startsWith(QStringLiteral("custom:"))) emit customChosen(value.mid(7)); else emit presetChosen(value);
    });
    madeFor_ = new QLabel; madeFor_->setProperty("sjRole", "muted"); madeFor_->setWordWrap(true); madeFor_->setFixedWidth(190);
    auto *presetColumn = new QVBoxLayout; presetColumn->setSpacing(2); presetColumn->addWidget(presetBox_); presetColumn->addWidget(madeFor_);
    barLayout->addLayout(presetColumn);
    // One row of choices that wraps when narrow.
    QList<QWidget *> items;
    auto helpButton = [this](const QString &id) {
        auto *help = new QToolButton; help->setText(QStringLiteral("?")); help->setProperty("sjVariant", "quiet"); help->setProperty("sjChoice", id);
        help->setAccessibleName(QStringLiteral("What %1 does").arg(id)); help->setFixedSize(18, 24); help->setFocusPolicy(Qt::StrongFocus);
        help->installEventFilter(this);
        return help;
    };
    for (const auto &info : Preset::choiceInfo()) {
        auto *wrap = new QWidget; auto *wrapLayout = new QHBoxLayout(wrap); wrapLayout->setContentsMargins(0, 0, 0, 0); wrapLayout->setSpacing(0);
        if (info.isSwitch()) {
            auto *toggle = new Button(info.label); toggle->setCheckable(true); toggle->setProperty("sjRole", "builderToggle");
            connect(toggle, &QPushButton::toggled, this, [this, id = info.id](bool on) { emit choiceChanged(id, on); });
            wrapLayout->addWidget(toggle); controls_.insert(info.id, toggle);
        } else {
            auto *label = new QLabel(info.label); label->setProperty("sjRole", "muted");
            auto *combo = new QComboBox; combo->setAccessibleName(info.label);
            for (const auto &option : info.options) combo->addItem(optionText(option), option.toString());
            connect(combo, &QComboBox::currentIndexChanged, this, [this, combo, id = info.id](int) { emit choiceChanged(id, combo->currentData()); });
            wrapLayout->addWidget(label); wrapLayout->addSpacing(4); wrapLayout->addWidget(combo); controls_.insert(info.id, combo);
        }
        wrap->setToolTip(info.description);
        wrapLayout->addWidget(helpButton(info.id));
        wraps_.insert(info.id, wrap); items << wrap;
    }
    auto *logoWrap = new QWidget; auto *logoLayout = new QHBoxLayout(logoWrap); logoLayout->setContentsMargins(0, 0, 0, 0); logoLayout->setSpacing(0);
    logoButton_ = new Button(QStringLiteral("Logo: SJ mark")); logoLayout->addWidget(logoButton_); logoLayout->addWidget(helpButton(QStringLiteral("logo")));
    connect(logoButton_, &QPushButton::clicked, this, [this] {
        if (!logoPath_.isEmpty()) { setLogoPath({}); return; }
        const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Choose a logo"), QString(), QStringLiteral("Images (*.png *.jpg *.jpeg *.svg *.webp)"));
        if (!path.isEmpty()) setLogoPath(path);
    });
    items << logoWrap;
    barLayout->addWidget(wrap(items), 1);
    auto *customs = new Button(QStringLiteral("Custom presets"));
    connect(customs, &QPushButton::clicked, this, &Builder::customPresetsRequested);
    auto *exportButton = new Button(QStringLiteral("Export"), ButtonVariant::Primary);
    connect(exportButton, &QPushButton::clicked, this, &Builder::showExport);
    barLayout->addWidget(customs, 0, Qt::AlignTop); barLayout->addWidget(exportButton, 0, Qt::AlignTop);
    layout->addWidget(bar);
    stage_ = new QWidget(this);
    auto *stageLayout = new QVBoxLayout(stage_); stageLayout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(stage_, 1);
    outline_ = new Outline(stage_);
    // The help card: the choice, what it does, and a mini preview of the frame with the element outlined.
    helpCard_ = new QFrame(this, Qt::ToolTip | Qt::FramelessWindowHint); helpCard_->setProperty("sjRole", "help");
    auto *cardLayout = new QVBoxLayout(helpCard_); cardLayout->setContentsMargins(12, 10, 12, 12); cardLayout->setSpacing(6);
    helpTitle_ = new QLabel; helpTitle_->setProperty("sjRole", "heading"); helpText_ = new QLabel; helpText_->setWordWrap(true); helpText_->setFixedWidth(320);
    helpPreview_ = new QLabel;
    cardLayout->addWidget(helpTitle_); cardLayout->addWidget(helpText_); cardLayout->addWidget(helpPreview_);
    helpCard_->hide();
}

void Builder::setState(const QString &presetId, const QVariantMap &overrides, const QStringList &customNames, const QString &selectedCustom) {
    const bool changed = presetId != presetId_ || overrides != overrides_ || !frame_;
    presetId_ = Preset::find(presetId).id; overrides_ = overrides; selectedCustom_ = selectedCustom;
    {
        QSignalBlocker blocker(presetBox_);
        presetBox_->clear();
        for (const auto &preset : Preset::all()) presetBox_->addItem(preset.label, preset.id);
        if (!customNames.isEmpty()) presetBox_->insertSeparator(presetBox_->count());
        for (const auto &name : customNames) presetBox_->addItem(name + QStringLiteral(" (custom)"), QStringLiteral("custom:") + name);
        presetBox_->setCurrentIndex(presetBox_->findData(selectedCustom.isEmpty() ? presetId_ : QStringLiteral("custom:") + selectedCustom));
    }
    syncControls();
    if (changed) rebuildFrame();
}
void Builder::setLogoPath(const QString &path) {
    logoPath_ = path;
    logoButton_->setText(path.isEmpty() ? QStringLiteral("Logo: SJ mark") : QStringLiteral("Logo: %1 (clear)").arg(QFileInfo(path).fileName()));
    rebuildFrame();
}
void Builder::syncControls() {
    const Preset preset = Preset::find(presetId_);
    madeFor_->setText(preset.madeFor);
    const auto choices = Preset::resolve(presetId_, overrides_);
    const bool website = presetId_ == QStringLiteral("website");
    for (const auto &info : Preset::choiceInfo()) {
        auto *control = controls_.value(info.id);
        QSignalBlocker blocker(control);
        if (auto *toggle = qobject_cast<QPushButton *>(control)) toggle->setChecked(choices.value(info.id).toBool());
        else if (auto *combo = qobject_cast<QComboBox *>(control)) combo->setCurrentIndex(combo->findData(choices.value(info.id).toString()));
        // Choices for the other layout are greyed out: App frame choices on Website, Website choices on the app presets.
        const bool applies = info.group == QStringLiteral("shared") || (info.group == QStringLiteral("website")) == website;
        wraps_.value(info.id)->setEnabled(applies);
    }
}
void Builder::rebuildFrame() {
    if (frame_) frame_->deleteLater();
    QWidget *next = nullptr;
    if (presetId_ == QStringLiteral("website")) {
        // The native library has no Website layout, so the Builder says what Export offers instead.
        auto *page = new SettingsPage(QStringLiteral("Website"));
        page->contentLayout()->addWidget(new Notice(starterManifest().value(QStringLiteral("native")).toObject().value(QStringLiteral("unsupported")).toObject().value(QStringLiteral("website")).toString()));
        page->contentLayout()->addWidget(paragraph(QStringLiteral("Export writes the web starter for this Website: the same files as the web showcase's Builder.")));
        next = page;
    } else next = starterFrame(presetId_, overrides_, logoPath_, stage_);
    stage_->layout()->addWidget(next);
    frame_ = next;
    // Transparent parts of the frame show what is behind it, so the stage carries the frame's own canvas color.
    if (auto *shell = dynamic_cast<AppShell *>(next)) {
        QPalette ground = stage_->palette(); ground.setColor(QPalette::Window, Theme::load(QStringLiteral("clean"), shell->isDark()).color(QStringLiteral("canvas")));
        stage_->setPalette(ground); stage_->setAutoFillBackground(true);
    }
    if (property("sjDirection").isValid() && presetId_ == QStringLiteral("website")) Theme::load(property("sjDirection").toString(), property("sjDark").toBool()).apply(next);
    outline_->raise();
    if (!highlighted_.isEmpty()) QMetaObject::invokeMethod(this, [this] { highlight(highlighted_); }, Qt::QueuedConnection);
}
QWidget *Builder::targetIn(QWidget *frame, const QString &choice) const {
    const QString name = targets.value(choice);
    if (!frame) return nullptr;
    if (name.isEmpty()) return frame;
    auto *target = frame->findChild<QWidget *>(name);
    return target && target->isVisible() ? target : nullptr;
}
void Builder::highlight(const QString &choice) {
    highlighted_ = choice;
    auto *target = choice.isEmpty() ? nullptr : targetIn(frame_, choice);
    if (!target) { outline_->hide(); return; }
    const QRect area(target->mapTo(stage_, QPoint(0, 0)), target->size());
    static_cast<Outline *>(outline_)->showAround(area, Theme::load().color(QStringLiteral("accent")));
}
// A mini preview: the frame with the choice shown, drawn offscreen, with the element outlined.
QPixmap Builder::miniPreview(const QString &choice) {
    if (presetId_ == QStringLiteral("website")) return {};
    auto choices = Preset::resolve(presetId_, overrides_);
    const QStringList shown{QStringLiteral("rail"), QStringLiteral("menuBar"), QStringLiteral("statusBar"), QStringLiteral("bottomPanel"), QStringLiteral("documentTabs"), QStringLiteral("commandPalette"), QStringLiteral("dashboardHome")};
    if (shown.contains(choice)) choices.setValue(choice, true);
    if ((choice == QStringLiteral("sidebar") || choice == QStringLiteral("documentTabs")) && choices.sidebar == QStringLiteral("off")) choices.setValue(QStringLiteral("sidebar"), QStringLiteral("tree"));
    auto *shell = starterFrame(presetId_, Preset::overrides(presetId_, choices), logoPath_, nullptr);
    shell->setAttribute(Qt::WA_DontShowOnScreen); shell->resize(1000, 620); shell->show();
    QApplication::processEvents();
    QPixmap image = shell->grab();
    if (auto *target = targetIn(shell, choice)) {
        QPainter painter(&image); painter.setPen(QPen(Theme::load().color(QStringLiteral("accent")), 5));
        painter.drawRect(QRect(target->mapTo(shell, QPoint(0, 0)), target->size()).adjusted(2, 2, -3, -3));
    }
    shell->hide(); shell->deleteLater();
    return image.scaledToWidth(320, Qt::SmoothTransformation);
}
void Builder::showHelp(const QString &choice) {
    QString label = QStringLiteral("Logo"), description = QStringLiteral("The top-left logo slot. The SJ mark until you choose a PNG, JPEG, SVG or WebP file; Export copies the file into the starter. Clicking the logo goes Home.");
    for (const auto &info : Preset::choiceInfo()) if (info.id == choice) { label = info.label; description = info.description; }
    if (auto *wrap = wraps_.value(choice); wrap && !wrap->isEnabled()) description += QStringLiteral(" Not used by the %1 preset.").arg(Preset::find(presetId_).label);
    helpTitle_->setText(label); helpText_->setText(description);
    const QPixmap preview = miniPreview(choice);
    helpPreview_->setPixmap(preview); helpPreview_->setVisible(!preview.isNull());
    QWidget *anchor = choice == QStringLiteral("logo") ? logoButton_ : wraps_.value(choice);
    if (property("sjDirection").isValid()) Theme::load(property("sjDirection").toString(), property("sjDark").toBool()).apply(helpCard_);
    helpCard_->adjustSize();
    helpCard_->move(anchor->mapToGlobal(QPoint(0, anchor->height() + 6)));
    helpCard_->show();
    highlight(choice);
}
bool Builder::eventFilter(QObject *object, QEvent *event) {
    const QString choice = object->property("sjChoice").toString();
    if (!choice.isEmpty()) {
        if (event->type() == QEvent::Enter || event->type() == QEvent::FocusIn) showHelp(choice);
        else if (event->type() == QEvent::Leave || event->type() == QEvent::FocusOut) { helpCard_->hide(); highlight({}); }
    }
    return QWidget::eventFilter(object, event);
}
// Export: save the starter into a new folder, or copy the `new` command that writes the same files.
void Builder::showExport() {
    auto *dialog = new QDialog(this); dialog->setWindowTitle(QStringLiteral("Export")); dialog->setAttribute(Qt::WA_DeleteOnClose); dialog->resize(720, 360);
    auto *layout = new QVBoxLayout(dialog); layout->setContentsMargins(20, 16, 20, 16); layout->setSpacing(10);
    auto *platform = new QComboBox; platform->setAccessibleName(QStringLiteral("Starter"));
    for (const auto &p : {QStringLiteral("web"), QStringLiteral("native")}) platform->addItem(starterLabel(p) + QStringLiteral(": ") + starterDescription(p), p);
    if (presetId_ == QStringLiteral("website")) platform->setCurrentIndex(0);
    auto *note = paragraph({}); note->setProperty("sjRole", "muted");
    auto *commandLine = new QLineEdit; commandLine->setReadOnly(true); commandLine->setProperty("sjFontRole", "mono"); commandLine->setAccessibleName(QStringLiteral("Command"));
    auto *copy = new Button(QStringLiteral("Copy command"));
    auto *save = new Button(QStringLiteral("Save to a new folder..."), ButtonVariant::Primary);
    auto *result = paragraph({}); result->setVisible(false);
    auto refresh = [=, this] {
        const QString p = platform->currentData().toString();
        const auto starter = generateStarter(p, presetId_, overrides_, logoPath_);
        const bool ok = starter.error.isEmpty() && starter.unsupported.isEmpty();
        note->setText(ok ? QStringLiteral("The %1 preset%2%3, installing the library from the v%4 release tag. The folder and the command write the same %5 files.")
            .arg(Preset::find(presetId_).label, overrides_.isEmpty() ? QString() : QStringLiteral(" with %1 changed choices").arg(overrides_.size()),
                 logoPath_.isEmpty() ? QString() : QStringLiteral(" and the logo ") + QFileInfo(logoPath_).fileName(), QStringLiteral(STACKJUNKIE_VERSION)).arg(starter.files.size())
            : starter.unsupported.isEmpty() ? starter.error : starter.unsupported);
        commandLine->setText(ok ? starterCommand(p, presetId_, overrides_, logoPath_.isEmpty() ? QString() : QFileInfo(logoPath_).fileName()) : QString());
        copy->setEnabled(ok); save->setEnabled(ok);
    };
    connect(platform, &QComboBox::currentIndexChanged, dialog, refresh);
    connect(copy, &QPushButton::clicked, dialog, [commandLine] { QGuiApplication::clipboard()->setText(commandLine->text()); });
    connect(save, &QPushButton::clicked, dialog, [=, this] {
        const QString folder = QFileDialog::getExistingDirectory(dialog, QStringLiteral("Choose an empty folder for the starter"));
        if (folder.isEmpty()) return;
        // Like the new command, never write into a folder that already has files.
        if (!QDir(folder).isEmpty()) { result->setText(QStringLiteral("%1 is not empty. Choose an empty folder.").arg(QDir::toNativeSeparators(folder))); result->setVisible(true); return; }
        const auto starter = generateStarter(platform->currentData().toString(), presetId_, overrides_, logoPath_);
        const QString error = writeStarter(starter, folder);
        result->setText(error.isEmpty() ? QStringLiteral("Wrote %1 files to %2.").arg(starter.files.size()).arg(QDir::toNativeSeparators(folder)) : error);
        result->setVisible(true);
    });
    layout->addWidget(new Field(QStringLiteral("Starter"), platform));
    layout->addWidget(note);
    layout->addWidget(new Field(QStringLiteral("Command"), commandLine, QStringLiteral("Paste it into a terminal, or give it to your coding agent. It needs Node.js 20 or later and no npm account.")));
    layout->addWidget(row({copy, save}));
    layout->addWidget(result);
    layout->addStretch();
    refresh();
    if (property("sjDirection").isValid()) Theme::load(property("sjDirection").toString(), property("sjDark").toBool()).apply(dialog);
    dialog->show();
}
void Builder::applyShowcaseTheme(const Theme &theme) {
    setProperty("sjDirection", theme.direction); setProperty("sjDark", theme.dark);
    stage_->setAutoFillBackground(false);
    theme.apply(bar_);
    if (presetId_ == QStringLiteral("website") && frame_) theme.apply(frame_);
}
