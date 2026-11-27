#include "gallery.hpp"
#include <stackjunkie/components.hpp>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsOpacityEffect>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QStyle>
#include <QTextBrowser>
#include <QTimeEdit>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <functional>

using namespace StackJunkie;
namespace {
QWidget *vertical(const QList<QWidget *> &widgets, QWidget *parent = nullptr) {
    auto *root = new QWidget(parent);
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Theme::spacing(2));
    for (auto *widget : widgets) layout->addWidget(widget);
    return root;
}
QWidget *scroll(QWidget *content, QWidget *parent = nullptr) {
    auto *area = new QScrollArea(parent);
    area->setWidgetResizable(true);
    area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    area->setWidget(content);
    return area;
}
Theme scopeTheme(QWidget *widget) {
    for (auto *scope = widget; scope; scope = scope->parentWidget())
        if (scope->property("sjDirection").isValid()) return Theme::load(scope->property("sjDirection").toString(), scope->property("sjDark").toBool());
    return Theme::load();
}
QWidget *slider() {
    auto *value = new QLabel(QStringLiteral("50%"));
    auto *control = new QSlider(Qt::Horizontal);
    control->setRange(0, 100);
    control->setValue(50);
    control->setAccessibleName(QStringLiteral("Level in percent"));
    control->setSingleStep(1);
    QObject::connect(control, &QSlider::valueChanged, value, [value](int n) { value->setText(QString::number(n) + QStringLiteral("%")); });
    auto *step = new QCheckBox(QStringLiteral("Steps of 10"));
    QObject::connect(step, &QCheckBox::toggled, control, [control](bool stepped) { control->setSingleStep(stepped ? 10 : 1); control->setPageStep(stepped ? 10 : 5); control->setTickInterval(stepped ? 10 : 0); control->setTickPosition(stepped ? QSlider::TicksBelow : QSlider::NoTicks); });
    auto *result = vertical({row({control, value}), step});
    result->layout()->setContentsMargins(0, 8, 0, 8);
    return result;
}
QWidget *table() {
    auto *search = new SearchField(QStringLiteral("Filter local records"));
    auto *data = new QTableWidget(4, 3);
    data->setAccessibleName(QStringLiteral("Local sample records"));
    data->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Type"), QStringLiteral("Size (KB)")});
    const QList<QStringList> records = {{"Notes", "Text", "12"}, {"Palette", "JSON", "8"}, {"Layout", "Sketch", "32"}, {"Controls", "Source", "24"}};
    for (int r = 0; r < records.size(); ++r) for (int c = 0; c < 3; ++c) {
        auto *item = new QTableWidgetItem;
        if (c == 2) item->setData(Qt::DisplayRole, records[r][c].toInt()); else item->setText(records[r][c]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        data->setItem(r, c, item);
    }
    data->setSelectionBehavior(QAbstractItemView::SelectRows);
    data->setAlternatingRowColors(true);
    data->setSortingEnabled(true);
    data->verticalHeader()->hide();
    data->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    data->setMinimumHeight(230);
    auto *status = new QLabel(QStringLiteral("4 local sample records"));
    QObject::connect(search, &QLineEdit::textChanged, data, [data, status](const QString &query) {
        int visible = 0;
        for (int r = 0; r < data->rowCount(); ++r) {
            const bool match = data->item(r, 0)->text().contains(query, Qt::CaseInsensitive) || data->item(r, 1)->text().contains(query, Qt::CaseInsensitive);
            data->setRowHidden(r, !match);
            visible += match;
        }
        status->setText(visible ? QStringLiteral("%1 matching records").arg(visible) : QStringLiteral("No matching records. Clear the filter to see all four."));
    });
    return vertical({search, data, status});
}
QWidget *tabs() {
    auto *tabs = new Tabs;
    tabs->setAccessibleName(QStringLiteral("Document tabs"));
    tabs->setMovable(true);
    for (const auto &name : {QStringLiteral("Notes"), QStringLiteral("Details")}) {
        auto *editor = new QPlainTextEdit;
        editor->setPlainText(name == QStringLiteral("Notes") ? QStringLiteral("Edit these notes. Switching tabs or pages keeps your text.") : QStringLiteral("Each tab retains its own document."));
        editor->setAccessibleName(name);
        tabs->addTab(editor, name);
    }
    tabs->setMinimumHeight(180);
    return tabs;
}
QWidget *splitPane() {
    auto *inspector = vertical({heading(QStringLiteral("Inspector")), new Field(QStringLiteral("Object name"), new QLineEdit(QStringLiteral("Untitled"))), new QCheckBox(QStringLiteral("Visible"))});
    inspector->setMinimumWidth(160);
    auto *split = new SplitPane;
    auto *editor = new QPlainTextEdit(QStringLiteral("Edit the document. Drag the divider to resize the inspector."));
    editor->setAccessibleName(QStringLiteral("Document"));
    split->addWidget(editor);
    split->addWidget(inspector);
    split->setChildrenCollapsible(false);
    split->setSizes({450, 220});
    split->setMinimumHeight(230);
    auto *toggle = new Button(QStringLiteral("Hide inspector"));
    auto *result = vertical({toggle, split});
    QObject::connect(toggle, &QPushButton::clicked, result, [split, inspector, toggle, width = 220]() mutable {
        if (inspector->isVisible()) { width = split->sizes().value(1, width); inspector->hide(); toggle->setText(QStringLiteral("Show inspector")); }
        else { inspector->show(); split->setSizes({qMax(240, split->width() - width), width}); toggle->setText(QStringLiteral("Hide inspector")); }
    });
    return result;
}
QWidget *progress() {
    auto *bar = new QProgressBar;
    bar->setAccessibleName(QStringLiteral("Local demonstration progress"));
    bar->setRange(0, 100);
    bar->setValue(0);
    auto *start = new Button(QStringLiteral("Start"), ButtonVariant::Primary);
    auto *pause = new Button(QStringLiteral("Pause"));
    auto *cancel = new Button(QStringLiteral("Cancel"));
    auto *label = new QLabel(QStringLiteral("Ready"));
    auto *result = vertical({bar, row({start, pause, cancel}), label});
    auto *timer = new QTimer(result);
    timer->setInterval(100);
    pause->setEnabled(false);
    cancel->setEnabled(false);
    QObject::connect(start, &QPushButton::clicked, result, [=] { bar->setValue(0); label->setText(QStringLiteral("Running local demonstration")); timer->start(); start->setEnabled(false); pause->setEnabled(true); cancel->setEnabled(true); pause->setText(QStringLiteral("Pause")); });
    QObject::connect(timer, &QTimer::timeout, result, [=] { bar->setValue(bar->value() + 2); if (bar->value() >= 100) { timer->stop(); label->setText(QStringLiteral("Complete")); start->setEnabled(true); pause->setEnabled(false); cancel->setEnabled(false); } });
    QObject::connect(pause, &QPushButton::clicked, result, [=] { const bool running = timer->isActive(); if (running) timer->stop(); else timer->start(); label->setText(running ? QStringLiteral("Paused") : QStringLiteral("Running local demonstration")); pause->setText(running ? QStringLiteral("Resume") : QStringLiteral("Pause")); });
    QObject::connect(cancel, &QPushButton::clicked, result, [=] { timer->stop(); label->setText(QStringLiteral("Cancelled")); start->setEnabled(true); pause->setEnabled(false); cancel->setEnabled(false); });
    return result;
}
QWidget *commandPalette() {
    auto *open = new Button(QStringLiteral("Open command palette"));
    QObject::connect(open, &QPushButton::clicked, open, [open] {
        auto *dialog = new QDialog(open);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowTitle(QStringLiteral("Commands"));
        auto *layout = new QVBoxLayout(dialog);
        auto *search = new SearchField(QStringLiteral("Find a command"));
        auto *list = new QListWidget;
        list->addItems({QStringLiteral("Copy selected command"), QStringLiteral("Clear search"), QStringLiteral("Close palette")});
        list->setAccessibleName(QStringLiteral("Matching commands"));
        auto *status = new QLabel;
        layout->addWidget(search); layout->addWidget(list); layout->addWidget(status);
        QObject::connect(search, &QLineEdit::textChanged, list, [list](const QString &text) { for (int i = 0; i < list->count(); ++i) list->item(i)->setHidden(!list->item(i)->text().contains(text, Qt::CaseInsensitive)); });
        QObject::connect(list, &QListWidget::itemActivated, dialog, [=](QListWidgetItem *item) {
            if (item->text().startsWith(QStringLiteral("Close"))) dialog->accept();
            else if (item->text().startsWith(QStringLiteral("Clear"))) { search->clear(); search->setFocus(); }
            else { QGuiApplication::clipboard()->setText(item->text()); status->setText(QStringLiteral("Command name copied")); }
        });
        QObject::connect(search, &QLineEdit::returnPressed, dialog, [list] { for (int i = 0; i < list->count(); ++i) if (!list->item(i)->isHidden()) { list->setCurrentRow(i); emit list->itemActivated(list->item(i)); break; } });
        scopeTheme(open).apply(dialog);
        dialog->resize(440, 260); dialog->open(); search->setFocus();
    });
    return open;
}
QWidget *confirmation(bool form) {
    auto *result = new QWidget;
    auto *open = new Button(form ? QStringLiteral("Edit document name") : QStringLiteral("Remove local sample"), form ? ButtonVariant::Secondary : ButtonVariant::Danger);
    auto *status = new QLabel(form ? QStringLiteral("Document: Untitled") : QStringLiteral("Local sample is present"));
    auto *layout = new QVBoxLayout(result); layout->setContentsMargins(0, 0, 0, 0); layout->addWidget(open); layout->addWidget(status);
    QObject::connect(open, &QPushButton::clicked, result, [=] {
        auto *dialog = new QDialog(open);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setWindowTitle(form ? QStringLiteral("Document name") : QStringLiteral("Remove local sample?"));
        auto *body = new QVBoxLayout(dialog);
        QLineEdit *name = form ? new QLineEdit(QStringLiteral("Untitled"), dialog) : nullptr;
        if (name) body->addWidget(new Field(QStringLiteral("Name"), name));
        else body->addWidget(paragraph(QStringLiteral("This removes only the demonstration item in this section.")));
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
        if (!form) buttons->button(QDialogButtonBox::Save)->setText(QStringLiteral("Remove"));
        body->addWidget(buttons);
        QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, [=] { if (name && name->text().trimmed().isEmpty()) { name->setFocus(); return; } status->setText(name ? QStringLiteral("Document: ") + name->text() : QStringLiteral("Local sample removed")); dialog->accept(); });
        QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
        QObject::connect(dialog, &QDialog::finished, open, [open] { open->setFocus(); });
        scopeTheme(open).apply(dialog);
        dialog->resize(420, 180); dialog->open();
    });
    return result;
}
QWidget *canvas() {
    auto *surface = new Canvas;
    surface->addRectangle(QStringLiteral("Rectangle"), QRectF(30, 40, 100, 70));
    surface->addEllipse(QStringLiteral("Circle"), QRectF(210, 70, 100, 100));
    return surface;
}
QWidget *component(const QString &id) {
    if (id == "colors") {
        auto *list = new QListWidget;
        list->setObjectName(QStringLiteral("semantic-colors"));
        list->setAccessibleName(QStringLiteral("Semantic color tokens"));
        list->setMinimumHeight(280);
        for (const auto &key : Theme::load().colors.keys()) list->addItem(key);
        return list;
    }
    if (id == "typography") {
        auto *ui = paragraph(QStringLiteral("Interface: labels, values, controls and navigation."));
        auto *editorial = heading(QStringLiteral("Reading deserves its own rhythm.")); editorial->setProperty("sjFontRole", "editorial");
        auto *mono = paragraph(QStringLiteral("MONO  0123456789  [] {} <>")); mono->setProperty("sjFontRole", "mono");
        return vertical({ui, editorial, mono});
    }
    if (id == "spacing") return paragraph(QStringLiteral("Shared spacing: 4, 8, 12, 16, 24, 32, 48 px. Direction tokens set control height, radius and border width. Text remains readable when the window narrows."));
    if (id == "icons") return row({new IconButton("search", "Search"), new IconButton("folder", "Open folder"), new IconButton("settings", "Settings"), new IconButton("help", "Help"), new IconButton("copy", "Copy"), new IconButton("check", "Confirm")});
    if (id == "motion") {
        auto *reduced = new QCheckBox(QStringLiteral("Reduce motion"));
        auto *toggle = new Button(QStringLiteral("Toggle detail"));
        auto *detail = new Notice(QStringLiteral("Optional detail revealed."));
        auto *effect = new QGraphicsOpacityEffect(detail); detail->setGraphicsEffect(effect); effect->setOpacity(1.0);
        auto *animation = new QPropertyAnimation(effect, "opacity", detail); animation->setDuration(Theme::motionDuration());
        auto *result = vertical({reduced, toggle, detail});
        QObject::connect(toggle, &QPushButton::clicked, result, [=] { animation->stop(); const qreal target = effect->opacity() > 0.5 ? 0.0 : 1.0; if (reduced->isChecked()) effect->setOpacity(target); else { animation->setStartValue(effect->opacity()); animation->setEndValue(target); animation->start(); } detail->setAccessibleDescription(target > 0.5 ? QStringLiteral("Visible") : QStringLiteral("Hidden")); });
        QObject::connect(reduced, &QCheckBox::toggled, result, [=](bool reduce) { if (reduce && animation->state() == QAbstractAnimation::Running) { const auto end = animation->endValue().toReal(); animation->stop(); effect->setOpacity(end); } });
        return result;
    }
    if (id == "button") {
        auto *primary = new Button(QStringLiteral("Save"), ButtonVariant::Primary);
        auto *secondary = new Button(QStringLiteral("Secondary"));
        auto *quiet = new Button(QStringLiteral("Quiet action"), ButtonVariant::Quiet);
        auto *danger = new Button(QStringLiteral("Remove"), ButtonVariant::Danger);
        auto *disabled = new Button(QStringLiteral("Unavailable")); disabled->setEnabled(false);
        auto *state = new QLabel(QStringLiteral("No action yet"));
        QObject::connect(primary, &QPushButton::clicked, state, [state, primary] { primary->setBusy(true, QStringLiteral("Saving...")); QTimer::singleShot(500, primary, [state, primary] { primary->setBusy(false); state->setText(QStringLiteral("Local sample saved")); }); });
        QObject::connect(quiet, &QPushButton::clicked, state, [state] { state->setText(QStringLiteral("Quiet action activated")); });
        QObject::connect(secondary, &QPushButton::clicked, state, [state] { state->setText(QStringLiteral("Secondary action activated")); });
        QObject::connect(danger, &QPushButton::clicked, state, [state] { state->setText(QStringLiteral("Local sample removed")); });
        return vertical({wrap({primary, secondary, quiet}), wrap({danger, disabled}), state});
    }
    if (id == "icon-button") { auto *toggle = new IconButton("pause", "Pause local preview"); toggle->setCheckable(true); auto *play = new IconButton("play", "Play local preview"); auto *close = new IconButton("close", "Stop local preview"); auto *status = new QLabel(QStringLiteral("Stopped")); QObject::connect(play, &QToolButton::clicked, status, [=] { status->setText(QStringLiteral("Playing")); toggle->setChecked(false); }); QObject::connect(toggle, &QToolButton::toggled, status, [status](bool paused) { status->setText(paused ? QStringLiteral("Paused") : QStringLiteral("Playing")); }); QObject::connect(close, &QToolButton::clicked, status, [status] { status->setText(QStringLiteral("Stopped")); }); return vertical({row({play, toggle, close}), status}); }
    if (id == "toolbar") {
        auto *bar = new QToolBar;
        bar->setAccessibleName(QStringLiteral("Editing tools"));
        auto *state = new QLabel(QStringLiteral("Choose a tool"));
        for (const auto &name : {QStringLiteral("Select"), QStringLiteral("Draw"), QStringLiteral("Move"), QStringLiteral("Undo"), QStringLiteral("Redo")}) { auto *action = bar->addAction(name); QObject::connect(action, &QAction::triggered, state, [state, name] { state->setText(name + QStringLiteral(" selected")); }); }
        QList<QWidget *> tools;
        for (const auto &name : {QStringLiteral("Select"), QStringLiteral("Draw"), QStringLiteral("Move"), QStringLiteral("Undo"), QStringLiteral("Redo")}) {
            auto *button = new Button(name);
            QObject::connect(button, &QPushButton::clicked, state, [state, name] { state->setText(name + QStringLiteral(" selected")); });
            tools << button;
        }
        return vertical({paragraph(QStringLiteral("Compact toolbar")), bar, paragraph(QStringLiteral("Wrapping toolbar")), wrap(tools), state});
    }
    if (id == "menu") {
        auto *button = new QToolButton; button->setText(QStringLiteral("Document actions")); button->setAccessibleName(button->text());
        auto *menu = new QMenu(button); auto *state = new QLabel(QStringLiteral("Choose an action"));
        for (const auto &name : {QStringLiteral("Duplicate"), QStringLiteral("Rename"), QStringLiteral("Archive")}) { auto *a = menu->addAction(name); QObject::connect(a, &QAction::triggered, state, [=] { state->setText(name + QStringLiteral(" selected for the local sample")); }); }
        button->setMenu(menu); button->setPopupMode(QToolButton::InstantPopup);
        auto *context = new QLineEdit(QStringLiteral("Local document")); context->setReadOnly(true); context->setAccessibleName(QStringLiteral("Document context menu target")); context->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(context, &QWidget::customContextMenuRequested, menu, [context, menu](const QPoint &point) { menu->popup(context->mapToGlobal(point)); });
        return vertical({button, context, paragraph(QStringLiteral("Right-click the local document or press the Menu key for context actions.")), state});
    }
    if (id == "command-palette") return commandPalette();
    if (id == "text-input") {
        auto *text = new QLineEdit(QStringLiteral("Untitled document"));
        auto *password = new QLineEdit; password->setEchoMode(QLineEdit::Password);
        auto *field = new Field(QStringLiteral("Document name"), text, QStringLiteral("The name stays in this local preview. Empty input shows an inline error."));
        QObject::connect(text, &QLineEdit::textChanged, field, [field](const QString &value) { field->setError(value.trimmed().isEmpty() ? QStringLiteral("Enter a document name.") : QString()); });
        return vertical({field, new Field(QStringLiteral("Password sample"), password), new Field(QStringLiteral("Search"), new SearchField)});
    }
    if (id == "text-area") { auto *edit = new QPlainTextEdit; edit->setPlainText(QStringLiteral("Your edits stay when you switch families or themes.")); return new Field(QStringLiteral("Notes"), edit); }
    if (id == "number-input") { auto *n = new QSpinBox; n->setRange(1, 100); n->setValue(12); auto *d = new QDoubleSpinBox; d->setRange(0, 10); d->setValue(1.25); d->setSuffix(QStringLiteral(" s")); return row({new Field(QStringLiteral("Count"), n), new Field(QStringLiteral("Duration"), d)}); }
    if (id == "select") { auto *select = new QComboBox; select->addItems({"Local folder", "Current document", "Selected items"}); return new Field(QStringLiteral("Destination"), select); }
    if (id == "checkbox") { auto *checked = new QCheckBox(QStringLiteral("Include notes")); checked->setChecked(true); auto *mixed = new QCheckBox(QStringLiteral("Some items selected")); mixed->setTristate(true); mixed->setCheckState(Qt::PartiallyChecked); return vertical({new QCheckBox(QStringLiteral("Include metadata")), checked, mixed}); }
    if (id == "radio-group") { auto *box = new QGroupBox(QStringLiteral("Export scope")); auto *v = new QVBoxLayout(box); auto *first = new QRadioButton(QStringLiteral("Selected item")); first->setChecked(true); v->addWidget(first); v->addWidget(new QRadioButton(QStringLiteral("All items"))); return box; }
    if (id == "switch") { auto *control = new QCheckBox(QStringLiteral("Enable local preview")); control->setProperty("sjVariant", "switch"); auto *status = new QLabel(QStringLiteral("Off")); QObject::connect(control, &QCheckBox::toggled, status, [status](bool on) { status->setText(on ? QStringLiteral("On") : QStringLiteral("Off")); }); return row({control, status}); }
    if (id == "slider") return slider();
    if (id == "date-time") { auto *date = new QDateEdit(QDate::currentDate()); date->setCalendarPopup(true); auto *time = new QTimeEdit(QTime(9, 0)); return row({new Field(QStringLiteral("Date"), date), new Field(QStringLiteral("Time"), time)}); }
    if (id == "file-input") { auto *open = new Button(QStringLiteral("Choose local files")); auto *multiple = new QCheckBox(QStringLiteral("Allow multiple files")); auto *value = new QLabel(QStringLiteral("No file selected")); value->setWordWrap(true); QObject::connect(open, &QPushButton::clicked, value, [=] { const QStringList paths = multiple->isChecked() ? QFileDialog::getOpenFileNames(open, QStringLiteral("Choose files")) : QStringList{QFileDialog::getOpenFileName(open, QStringLiteral("Choose a file"))}; QStringList names; for (const auto &path : paths) if (!path.isEmpty()) names << QFileInfo(path).fileName(); if (!names.isEmpty()) { value->setText(names.join(QStringLiteral(", "))); value->setToolTip(paths.join(QStringLiteral("\n"))); } }); return vertical({row({open, multiple}), value}); }
    if (id == "color-input") { auto *open = new Button(QStringLiteral("Choose color")); auto *value = new QLabel(QStringLiteral("#285ac9")); QObject::connect(open, &QPushButton::clicked, value, [=] { const auto color = QColorDialog::getColor(QColor(value->text()), open, QStringLiteral("Choose color")); if (color.isValid()) value->setText(color.name()); }); return row({open, value}); }
    if (id == "rail" || id == "sidebar") {
        auto *shell = new Workspace; shell->setHeader(QStringLiteral("Tool workspace"));
        shell->addPage(QStringLiteral("Document"), new QPlainTextEdit(QStringLiteral("Write here, navigate away, then return.")), QStringLiteral("file"));
        shell->addPage(QStringLiteral("Settings"), new Field(QStringLiteral("Display name"), new QLineEdit), QStringLiteral("settings"));
        shell->setMinimumHeight(290); if (id == "rail") shell->setSidebarVisible(false); return shell;
    }
    if (id == "tabs") {
        auto *groups = new GroupedTabs;
        const int work = groups->addGroup(QStringLiteral("Workspace"));
        const int reference = groups->addGroup(QStringLiteral("Reference"));
        groups->addTab(work, QStringLiteral("Draft"), new QPlainTextEdit(QStringLiteral("Editable draft in the Workspace group.")));
        groups->addTab(work, QStringLiteral("Notes"), new QPlainTextEdit(QStringLiteral("Notes remain here when another group is selected.")));
        groups->addTab(reference, QStringLiteral("Tokens"), new CodeBlock(QStringLiteral("Shared tokens are embedded from tokens.json.")));
        groups->addTab(reference, QStringLiteral("Usage"), paragraph(QStringLiteral("Group chips sit in the same row. Choose one to collapse its tabs.")));
        groups->setMinimumHeight(160);
        auto *open = new Tabs; open->setAccessibleName(QStringLiteral("Open documents")); open->setDocumentTabs(true);
        for (const auto &name : {QStringLiteral("Spacing study"), QStringLiteral("Control states"), QStringLiteral("Layout notes")}) open->addTab(new QPlainTextEdit(name + QStringLiteral(" stays as you left it while you switch tabs.")), name);
        open->setMinimumHeight(160);
        return vertical({heading(QStringLiteral("Page tabs")), tabs(), heading(QStringLiteral("Grouped in one row")), groups, heading(QStringLiteral("Open documents")), open});
    }
    if (id == "breadcrumbs") { auto *home = new Button(QStringLiteral("Library"), ButtonVariant::Quiet); auto *project = new Button(QStringLiteral("Project"), ButtonVariant::Quiet); auto *status = new QLabel(QStringLiteral("Current: Document")); QObject::connect(home, &QPushButton::clicked, status, [status] { status->setText(QStringLiteral("Current: Library")); }); QObject::connect(project, &QPushButton::clicked, status, [status] { status->setText(QStringLiteral("Current: Project")); }); return vertical({row({home, new QLabel(QStringLiteral("/")), project, new QLabel(QStringLiteral("/ Document"))}), status}); }
    if (id == "pagination") { auto *page = new QSpinBox; page->setRange(1, 12); page->setAccessibleName(QStringLiteral("Current page")); auto *previous = new Button(QStringLiteral("Previous")); auto *next = new Button(QStringLiteral("Next")); QObject::connect(previous, &QPushButton::clicked, page, [page] { page->setValue(page->value() - 1); }); QObject::connect(next, &QPushButton::clicked, page, [page] { page->setValue(page->value() + 1); }); auto update = [=](int n) { previous->setEnabled(n > 1); next->setEnabled(n < 12); }; QObject::connect(page, &QSpinBox::valueChanged, page, update); update(1); return row({previous, page, new QLabel(QStringLiteral("of 12")), next}); }
    if (id == "panel") {
        auto *panel = new QFrame; panel->setProperty("sjRole", "panel"); auto *v = new QVBoxLayout(panel);
        v->setContentsMargins(Theme::spacing(3), Theme::spacing(3), Theme::spacing(3), Theme::spacing(3));
        v->addWidget(heading(QStringLiteral("Document options"))); v->addWidget(new Field(QStringLiteral("Name"), new QLineEdit(QStringLiteral("Field notes"))));
        auto *inset = new QFrame; inset->setProperty("sjRole", "panel"); inset->setProperty("sjVariant", "inset"); auto *inner = new QVBoxLayout(inset);
        inner->addWidget(paragraph(QStringLiteral("Inset options"))); inner->addWidget(new QCheckBox(QStringLiteral("Include metadata"))); v->addWidget(inset); return panel;
    }
    if (id == "disclosure") return new Disclosure(QStringLiteral("Export options"), vertical({new QCheckBox(QStringLiteral("Include notes")), new Field(QStringLiteral("Prefix"), new QLineEdit)}));
    if (id == "split-pane") return splitPane();
    if (id == "drawer") {
        auto *edge = new QComboBox; edge->addItems({QStringLiteral("Right"), QStringLiteral("Left")}); edge->setAccessibleName(QStringLiteral("Drawer edge"));
        auto *open = new Button(QStringLiteral("Open detail drawer"));
        QObject::connect(open, &QPushButton::clicked, open, [open, edge, drawer = QPointer<Drawer>()]() mutable {
            if (!drawer) drawer = new Drawer(open->window(), QStringLiteral("Document details"), vertical({new Field(QStringLiteral("Name"), new QLineEdit(QStringLiteral("Field notes"))), new Field(QStringLiteral("Notes"), new QPlainTextEdit), new QCheckBox(QStringLiteral("Include in collection"))}));
            drawer->setEdge(edge->currentIndex() ? Drawer::Edge::Left : Drawer::Edge::Right); drawer->open();
        });
        return row({edge, open});
    }
    if (id == "stack") {
        auto *first = new QLineEdit; auto *last = new QLineEdit; auto *save = new Button(QStringLiteral("Save profile"), ButtonVariant::Primary); auto *status = new QLabel;
        auto *result = vertical({paragraph(QStringLiteral("Vertical stack")), new Field(QStringLiteral("First name"), first), new Field(QStringLiteral("Last name"), last), paragraph(QStringLiteral("Horizontal actions")), row({save, status}), paragraph(QStringLiteral("Wrapping options")), wrap({new QCheckBox(QStringLiteral("Include notes")), new QCheckBox(QStringLiteral("Include metadata")), new QCheckBox(QStringLiteral("Remember choices"))})});
        QObject::connect(save, &QPushButton::clicked, result, [=] { status->setText(QStringLiteral("Saved locally: ") + (first->text() + QStringLiteral(" ") + last->text()).trimmed()); }); return result;
    }
    if (id == "settings-row") return vertical({paragraph(QStringLiteral("Inline setting")), row({new QLabel(QStringLiteral("Keep a local history")), new HelpButton(QStringLiteral("About local history"), QStringLiteral("Only edits made inside this preview are affected.")), new QCheckBox(QStringLiteral("Enabled"))}), paragraph(QStringLiteral("Stacked setting")), new Field(QStringLiteral("Collection name"), new QLineEdit(QStringLiteral("Local notes")), QStringLiteral("A stacked setting gives longer values a full line."))});
    if (id == "list") { auto *list = new QListWidget; list->addItems({"Notes", "Palette", "Controls", "Layout"}); list->setAccessibleName(QStringLiteral("Local documents")); list->setMinimumHeight(160); auto *empty = new QCheckBox(QStringLiteral("Show empty collection")); auto *message = new QLabel(QStringLiteral("No documents in this collection.")); message->hide(); QObject::connect(empty, &QCheckBox::toggled, list, [=](bool isEmpty) { list->clear(); if (!isEmpty) list->addItems({"Notes", "Palette", "Controls", "Layout"}); message->setVisible(isEmpty); }); return vertical({empty, list, message}); }
    if (id == "table") return table();
    if (id == "tree") { auto *tree = new QTreeWidget; tree->setHeaderHidden(true); tree->setAccessibleName(QStringLiteral("Project files")); auto *root = new QTreeWidgetItem(tree, {QStringLiteral("Project")}); new QTreeWidgetItem(root, {QStringLiteral("Notes.md")}); auto *folder = new QTreeWidgetItem(root, {QStringLiteral("References")}); new QTreeWidgetItem(folder, {QStringLiteral("Palette.json")}); root->setExpanded(true); tree->setMinimumHeight(180); return tree; }
    if (id == "grid") { auto *list = new QListWidget; list->setViewMode(QListView::IconMode); list->setResizeMode(QListView::Adjust); list->setMovement(QListView::Static); list->setGridSize(QSize(112, 90)); list->setIconSize(QSize(36, 36)); list->setAccessibleName(QStringLiteral("Document grid")); for (const auto &name : {"Notes", "Palette", "Layout", "Controls"}) { auto *item = new QListWidgetItem(icon(QStringLiteral("file"), Theme::load().color(QStringLiteral("text")), 36), QString::fromLatin1(name), list); item->setData(Qt::UserRole + 10, QStringLiteral("file")); } list->setMinimumHeight(210); return list; }
    if (id == "badge") return row({new Tag(QStringLiteral("Draft")), new Tag(QStringLiteral("Ready"), QStringLiteral("success")), new Tag(QStringLiteral("Needs attention"), QStringLiteral("warning")), new Tag(QStringLiteral("Failed"), QStringLiteral("danger"))});
    if (id == "alert") return vertical({new Notice(QStringLiteral("Local preview changes are not uploaded.")), new Notice(QStringLiteral("Changes saved locally."), QStringLiteral("success")), new Notice(QStringLiteral("Choose a destination before exporting."), QStringLiteral("warning")), new Notice(QStringLiteral("The sample destination is unavailable. Choose another folder."), QStringLiteral("danger"))});
    if (id == "progress") return progress();
    if (id == "toast") { auto *show = new Button(QStringLiteral("Show notification")); auto *notice = new Notice(QStringLiteral("Local changes saved."), QStringLiteral("success")); auto *close = new IconButton(QStringLiteral("close"), QStringLiteral("Dismiss notification")); notice->layout()->addWidget(close); notice->hide(); auto *result = vertical({show, notice}); QObject::connect(show, &QPushButton::clicked, notice, &QWidget::show); QObject::connect(close, &QToolButton::clicked, notice, &QWidget::hide); return result; }
    if (id == "empty-state") {
        auto *mode = new QComboBox; mode->addItems({QStringLiteral("Empty"), QStringLiteral("No results"), QStringLiteral("Unavailable"), QStringLiteral("Error")}); mode->setAccessibleName(QStringLiteral("Empty-state scenario"));
        auto *title = heading(QStringLiteral("No notes yet")); auto *detail = paragraph(QStringLiteral("Create a note to start this local collection.")); auto *action = new Button(QStringLiteral("Create local note"), ButtonVariant::Primary);
        auto *result = vertical({mode, title, detail, action});
        QObject::connect(mode, &QComboBox::currentIndexChanged, result, [=](int index) { const QStringList titles{"No notes yet", "No matching notes", "Source unavailable", "Could not load notes"}; const QStringList details{"Create a note to start this local collection.", "Clear the search to see all notes.", "The local sample source is disconnected.", "The local sample operation did not finish."}; const QStringList actions{"Create local note", "Clear search", "Reconnect", "Retry"}; title->setText(titles[index]); detail->setText(details[index]); action->setText(actions[index]); action->setEnabled(true); });
        QObject::connect(action, &QPushButton::clicked, result, [=] { title->setText(QStringLiteral("Local sample ready")); detail->setText(QStringLiteral("The demonstration action completed.")); action->setEnabled(false); });
        return result;
    }
    if (id == "loading") { auto *bar = new QProgressBar; bar->setRange(0, 0); bar->setAccessibleName(QStringLiteral("Indeterminate local loading preview")); auto *toggle = new QCheckBox(QStringLiteral("Show loading preview")); toggle->setChecked(true); auto *skeleton = new QFrame; skeleton->setProperty("sjRole", "panel"); auto *sl = new QVBoxLayout(skeleton); for (int i = 0; i < 3; ++i) { auto *line = new QLabel(QStringLiteral("Loading row %1").arg(i + 1)); line->setProperty("sjRole", "tag"); sl->addWidget(line); } QObject::connect(toggle, &QCheckBox::toggled, bar, &QWidget::setVisible); QObject::connect(toggle, &QCheckBox::toggled, skeleton, &QWidget::setVisible); return vertical({toggle, bar, skeleton, paragraph(QStringLiteral("Loading local preview. Duration unknown."))}); }
    if (id == "dialog") return vertical({confirmation(false), confirmation(true)});
    if (id == "help" || id == "tooltip") return row({new QLabel(QStringLiteral("Optional context")), new HelpButton(QStringLiteral("About optional context"), QStringLiteral("Hover or focus to preview this help. Click to pin it while using other controls. Press Escape or click outside to dismiss."))});
    if (id == "prose") {
        auto *text = new ReadingView(QStringLiteral("<h1>A useful working surface</h1><p>Keep the active document in view. Put secondary controls where their purpose is clear, and retain the user's place when they move between tasks.</p><h2>Reading with a clear hierarchy</h2><p>This local sample compares reading rhythm, heading sizes and line length.</p><h3>Details at the right level</h3><p>A subsection heading stays larger than the body text. The surrounding interface remains compact so the document has room to breathe.</p><ul><li>Readable labels</li><li>Clear actions</li><li>Details when needed</li></ul>"));
        text->setAccessibleName(QStringLiteral("Reading sample")); text->setMinimumHeight(320); return text;
    }
    if (id == "code") return new CodeBlock(QStringLiteral("# Local example\nfind_package(StackJunkie CONFIG REQUIRED)\ntarget_link_libraries(my_app PRIVATE StackJunkie::Widgets)\n\nauto theme = StackJunkie::Theme::load(\"clean\", false);\ntheme.apply(window);"));
    if (id == "media") { auto *image = new QLabel; QPixmap picture(480, 180); picture.fill(QColor("#e6edff")); QPainter p(&picture); p.setPen(QPen(QColor("#285ac9"), 3)); p.drawPolyline(QPolygon{QPoint(20, 140), QPoint(110, 55), QPoint(200, 115), QPoint(300, 40), QPoint(460, 100)}); p.end(); image->setPixmap(picture); image->setScaledContents(false); image->setAlignment(Qt::AlignCenter); image->setAccessibleName(QStringLiteral("Local geometric image: blue line with alternating peaks")); image->setMinimumSize(120, 80); return scroll(vertical({image, paragraph(QStringLiteral("Local generated image sample. Blue line with alternating peaks."))})); }
    if (id == "chart") {
        auto *chart = new Chart;
        chart->setData({{QStringLiteral("Notes"), 12}, {QStringLiteral("Palette"), 8}, {QStringLiteral("Layout"), 32}, {QStringLiteral("Controls"), 24}}, QStringLiteral("KB"));
        auto *mode = new SegmentedControl({QStringLiteral("Bars"), QStringLiteral("Line")});
        QObject::connect(mode, &SegmentedControl::currentChanged, chart, [chart](int index) { chart->setMode(index ? Chart::Mode::Line : Chart::Mode::Bars); });
        return vertical({mode, chart, paragraph(QStringLiteral("Local sample data. Values and units are shown on the chart."))});
    }
    if (id == "canvas") return canvas();
    return new Notice(QStringLiteral("This catalog item has no native preview."), QStringLiteral("warning"));
}
}

// The reference organization for apps with many pages: rail for areas, a collapsible tree for every page,
// one row of open documents, and a bottom panel with its own tabs in a separate region.
static QWidget *workbench() {
    auto *frame = new QWidget; auto *outer = new QHBoxLayout(frame); outer->setContentsMargins(0, 0, 0, 0); outer->setSpacing(0);
    auto *rail = new QWidget; auto *railLayout = new QVBoxLayout(rail); railLayout->setContentsMargins(4, 4, 4, 4);
    for (const auto &[name, label] : QList<QPair<QString, QString>>{{QStringLiteral("folder"), QStringLiteral("Explorer")}, {QStringLiteral("search"), QStringLiteral("Search")}}) {
        auto *button = new IconButton(name, label); button->setProperty("sjVariant", "quiet"); railLayout->addWidget(button);
    }
    railLayout->addStretch();
    auto *tree = new NavTree(QStringLiteral("Explorer"));
    tree->addGroup(QStringLiteral("documents"), QStringLiteral("Documents"), {{QStringLiteral("spacing"), QStringLiteral("Spacing study")}, {QStringLiteral("states"), QStringLiteral("Control states")}, {QStringLiteral("notes"), QStringLiteral("Layout notes")}});
    tree->addGroup(QStringLiteral("references"), QStringLiteral("References"), {{QStringLiteral("keys"), QStringLiteral("Keyboard map")}, {QStringLiteral("colors"), QStringLiteral("Color roles")}});
    tree->setMinimumWidth(170);
    auto *documents = new Tabs; documents->setAccessibleName(QStringLiteral("Open documents")); documents->setDocumentTabs(true);
    auto *panel = new Tabs; panel->setAccessibleName(QStringLiteral("Panel"));
    auto *output = new CodeBlock(QStringLiteral("Opened Spacing study, Layout notes")); panel->addTab(output, QStringLiteral("Output"));
    panel->addTab(paragraph(QStringLiteral("No problems.")), QStringLiteral("Problems"));
    auto open = [documents](const QString &id, const QString &title) {
        for (int i = 0; i < documents->count(); ++i) if (documents->widget(i)->objectName() == id) { documents->setCurrentIndex(i); return; }
        auto *editor = new QPlainTextEdit(title + QStringLiteral("\n\nEdit freely; text stays while you switch documents.")); editor->setObjectName(id);
        documents->setCurrentIndex(documents->addTab(editor, title));
    };
    open(QStringLiteral("spacing"), QStringLiteral("Spacing study")); open(QStringLiteral("notes"), QStringLiteral("Layout notes")); documents->setCurrentIndex(0);
    QObject::connect(tree, &NavTree::pageChosen, documents, [tree, open](const QString &id) { if (auto *item = tree->currentItem()) open(id, item->text(0)); });
    auto *editorArea = new SplitPane(Qt::Vertical); editorArea->addWidget(documents); editorArea->addWidget(panel); editorArea->setSizes({300, 120});
    auto *main = new SplitPane; main->addWidget(tree); main->addWidget(editorArea); main->setStretchFactor(1, 1); main->setSizes({190, 600});
    outer->addWidget(rail); outer->addWidget(main, 1);
    frame->setMinimumHeight(420);
    return frame;
}
QWidget *makeLayout(const QString &id, QWidget *parent) {
    QWidget *result = nullptr;
    if (id == "focused-utility" || id == "embedded") {
        auto *toggle = new QCheckBox(QStringLiteral("Enable preview")); auto *options = slider(); options->hide();
        QObject::connect(toggle, &QCheckBox::toggled, options, &QWidget::setVisible);
        result = vertical({toggle, options, new Disclosure(QStringLiteral("Options"), new QCheckBox(QStringLiteral("Remember this setting")))});
    } else if (id == "multi-tool") {
        auto *shell = new Workspace; shell->setHeader(QStringLiteral("Local tools")); shell->addPage(QStringLiteral("Notes"), new QPlainTextEdit, QStringLiteral("file")); shell->addPage(QStringLiteral("Levels"), slider(), QStringLiteral("settings")); shell->setMinimumHeight(300); result = shell;
    } else if (id == "data") result = table();
    else if (id == "workbench") result = workbench();
    else if (id == "publication") result = component(QStringLiteral("prose"));
    else if (id == "guided") {
        auto *stack = new QStackedWidget;
        auto *name = new QLineEdit; auto *summary = new QLabel;
        stack->addWidget(new Field(QStringLiteral("Document name"), name)); stack->addWidget(summary);
        auto *back = new Button(QStringLiteral("Back")); auto *next = new Button(QStringLiteral("Review"), ButtonVariant::Primary);
        back->setEnabled(false);
        QObject::connect(next, &QPushButton::clicked, stack, [=] { if (name->text().trimmed().isEmpty()) { name->setPlaceholderText(QStringLiteral("Enter a name first")); name->setFocus(); return; } summary->setText(QStringLiteral("Ready to create: ") + name->text()); stack->setCurrentIndex(1); back->setEnabled(true); next->setEnabled(false); });
        QObject::connect(back, &QPushButton::clicked, stack, [=] { stack->setCurrentIndex(0); back->setEnabled(false); next->setEnabled(true); });
        result = vertical({stack, row({back, next})});
    } else result = vertical({progress(), component(QStringLiteral("chart"))});
    result->setParent(parent);
    return result;
}
QWidget *makeFamily(const QJsonObject &family, QWidget *parent) {
    const auto entries = family.value(QStringLiteral("components")).toArray();
    const bool readingPage = entries.size() == 1 && entries.first().toObject().value(QStringLiteral("id")).toString() == QStringLiteral("prose");
    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    const int contentPadding = Theme::density(QStringLiteral("contentPadding"), 20);
    layout->setContentsMargins(contentPadding, contentPadding, contentPadding, contentPadding);
    layout->setSpacing(Theme::spacing(4));
    if (family.value(QStringLiteral("id")).toString() == QStringLiteral("layouts")) {
        for (const auto &value : Theme::catalog().value(QStringLiteral("layouts")).toArray()) {
            const auto spec = value.toObject();
            layout->addWidget(heading(spec.value(QStringLiteral("label")).toString()));
            layout->addWidget(new TabBoundary(makeLayout(spec.value(QStringLiteral("id")).toString())));
        }
    } else for (const auto &value : family.value(QStringLiteral("components")).toArray()) {
        const auto spec = value.toObject();
        auto *sample = component(spec.value(QStringLiteral("id")).toString());
        sample->setObjectName(QStringLiteral("sample-") + spec.value(QStringLiteral("id")).toString());
        auto *state = new QComboBox;
        state->setAccessibleName(spec.value(QStringLiteral("label")).toString() + QStringLiteral(" preview state"));
        state->addItem(QStringLiteral("Interactive"), QStringLiteral("default"));
        const auto states = spec.value(QStringLiteral("states")).toArray();
        if (states.contains(QStringLiteral("disabled"))) state->addItem(QStringLiteral("Disabled"), QStringLiteral("disabled"));
        if (states.contains(QStringLiteral("read-only"))) state->addItem(QStringLiteral("Read only"), QStringLiteral("read-only"));
        if (states.contains(QStringLiteral("invalid"))) state->addItem(QStringLiteral("Invalid"), QStringLiteral("invalid"));
        state->setVisible(state->count() > 1);
        auto *reference = new HelpButton(QStringLiteral("About ") + spec.value(QStringLiteral("label")).toString(), spec.value(QStringLiteral("usage")).toString());
        layout->addWidget(row({reference, state}));
        layout->addWidget(new TabBoundary(sample), readingPage ? 1 : 0);
        auto *validation = new QLabel(QStringLiteral("Enter a valid value to continue."));
        validation->setProperty("sjRole", "error");
        validation->setWordWrap(true);
        validation->hide();
        layout->addWidget(validation);
        QObject::connect(state, &QComboBox::currentIndexChanged, sample, [sample, state, validation](int) {
            const QString selected = state->currentData().toString();
            sample->setEnabled(selected != QStringLiteral("disabled"));
            validation->setVisible(selected == QStringLiteral("invalid"));
            QList<QWidget *> controls = sample->findChildren<QWidget *>(); controls.prepend(sample);
            for (auto *widget : controls) {
                const bool readOnly = selected == QStringLiteral("read-only");
                bool isInput = false;
                if (auto *edit = qobject_cast<QLineEdit *>(widget)) { edit->setReadOnly(readOnly); isInput = true; }
                if (auto *edit = qobject_cast<QPlainTextEdit *>(widget)) { edit->setReadOnly(readOnly); isInput = true; }
                if (auto *edit = qobject_cast<QAbstractSpinBox *>(widget)) { edit->setReadOnly(readOnly); isInput = true; }
                if (qobject_cast<QComboBox *>(widget) || qobject_cast<QAbstractButton *>(widget)) isInput = true;
                if (isInput) widget->setProperty("sjInvalid", selected == QStringLiteral("invalid"));
                if (auto *field = dynamic_cast<Field *>(widget)) field->setError(selected == QStringLiteral("invalid") ? QStringLiteral("Enter a valid value.") : QString());
                widget->style()->unpolish(widget); widget->style()->polish(widget);
            }
            sample->setAccessibleDescription(selected == QStringLiteral("invalid") ? QStringLiteral("Invalid state preview. Enter a valid value.") : QString());
        });
    }
    if (!readingPage) layout->addStretch();
    auto *tabs = new Tabs(parent);
    tabs->addTab(scroll(content), QStringLiteral("Example"));
    auto *referenceContent = new QWidget;
    auto *referenceLayout = new QVBoxLayout(referenceContent);
    referenceLayout->setContentsMargins(contentPadding, contentPadding, contentPadding, contentPadding);
    for (const auto &value : family.value(QStringLiteral("components")).toArray()) {
        const auto spec = value.toObject();
        referenceLayout->addWidget(paragraph(spec.value(QStringLiteral("usage")).toString()));
        QStringList variants, states;
        for (const auto &variant : spec.value(QStringLiteral("variants")).toArray()) variants << variant.toString();
        for (const auto &stateValue : spec.value(QStringLiteral("states")).toArray()) states << stateValue.toString();
        referenceLayout->addWidget(paragraph(QStringLiteral("Variants: ") + variants.join(QStringLiteral(", "))));
        referenceLayout->addWidget(paragraph(QStringLiteral("States: ") + states.join(QStringLiteral(", "))));
    }
    referenceLayout->addStretch();
    tabs->addTab(scroll(referenceContent), QStringLiteral("Usage and states"));
    return tabs;
}
QWidget *makeComparison(QWidget *parent) {
    auto *split = new SplitPane(parent);
    split->setChildrenCollapsible(false);
    split->setStackBelowWidth(900);
    for (const auto &direction : {QStringLiteral("clean"), QStringLiteral("warm"), QStringLiteral("technical")}) {
        auto *name = new QLineEdit(QStringLiteral("Field notes"));
        auto *save = new Button(QStringLiteral("Save"), ButtonVariant::Primary);
        auto *cancel = new Button(QStringLiteral("Cancel"));
        auto *include = new QCheckBox(QStringLiteral("Include notes"));
        auto *status = new Notice(QStringLiteral("Local preview changes remain in this window."));
        auto *editor = tabs(); editor->setMaximumHeight(220);
        auto *settings = new Disclosure(QStringLiteral("Document settings"), new Field(QStringLiteral("Reading mode"), new QComboBox, QStringLiteral("This local setting stays with this direction's preview.")), true);
        auto *reading = settings->findChild<QComboBox *>(); reading->addItems({QStringLiteral("Comfortable"), QStringLiteral("Compact")});
        auto *panel = vertical({heading(Theme::load(direction).label()), new Field(QStringLiteral("Document name"), name, QStringLiteral("Hover or focus for help. Click to pin it; Escape dismisses it.")), row({save, cancel}), include, slider(), settings, status, editor});
        qobject_cast<QVBoxLayout *>(panel->layout())->addStretch();
        name->setProperty("savedValue", name->text());
        include->setProperty("savedValue", false);
        QObject::connect(save, &QPushButton::clicked, panel, [=] {
            if (name->text().trimmed().isEmpty()) { status->setText(QStringLiteral("Enter a document name.")); name->setFocus(); return; }
            name->setProperty("savedValue", name->text()); include->setProperty("savedValue", include->isChecked());
            status->setText(QStringLiteral("Saved locally: ") + name->text());
        });
        QObject::connect(cancel, &QPushButton::clicked, panel, [=] { name->setText(name->property("savedValue").toString()); include->setChecked(include->property("savedValue").toBool()); status->setText(QStringLiteral("Unsaved name and option changes discarded.")); });
        panel->setProperty("comparisonDirection", direction);
        panel->setMinimumWidth(290);
        panel->layout()->setContentsMargins(Theme::spacing(3), Theme::spacing(3), Theme::spacing(3), Theme::spacing(3));
        split->addWidget(scroll(panel));
    }
    applyComparisonThemes(split, false);
    return split;
}
void applyComparisonThemes(QWidget *comparison, bool dark) {
    for (auto *widget : comparison->findChildren<QWidget *>()) {
        if (widget->property("comparisonDirection").isValid()) Theme::load(widget->property("comparisonDirection").toString(), dark).apply(widget);
    }
}
