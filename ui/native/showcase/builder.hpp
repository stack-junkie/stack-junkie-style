#pragma once
#include <stackjunkie/components.hpp>
#include <QPointer>

class QComboBox;
class QHBoxLayout;
class QLabel;

// Showcase only: the Builder. Pick a Preset, switch its choices from one toolbar row, and watch a blank App frame
// change. The frame is built from the starter's own structure (starters.json), so Export writes what it shows,
// as a folder or as the matching `new` command. It mirrors the web showcase's Builder.
class Builder : public QWidget {
    Q_OBJECT
public:
    explicit Builder(QWidget *parent = nullptr);
    /** The preset, its changed choices and the custom presets listed in the picker (selected is a custom name or empty). */
    void setState(const QString &presetId, const QVariantMap &overrides, const QStringList &customNames, const QString &selectedCustom);
    QString logoPath() const { return logoPath_; }
    void setLogoPath(const QString &path);
    /** Shows a choice's help card, as hovering its "?" does, for previews. */
    void showHelp(const QString &choice);
    /** Opens Export, for previews. */
    void showExport();
    /** The showcase's theme for the toolbar, help card and Export. The frame keeps its own preset theme. */
    void applyShowcaseTheme(const StackJunkie::Theme &theme);
signals:
    void backRequested();
    void presetChosen(const QString &presetId);
    void customChosen(const QString &name);
    void choiceChanged(const QString &choice, const QVariant &value);
    void customPresetsRequested();
protected:
    bool eventFilter(QObject *object, QEvent *event) override;
private:
    void rebuildFrame();
    void syncControls();
    void highlight(const QString &choice);
    QPixmap miniPreview(const QString &choice);
    QWidget *targetIn(QWidget *frame, const QString &choice) const;
    QString presetId_ = QStringLiteral("web-app");
    QVariantMap overrides_;
    QString selectedCustom_, logoPath_, highlighted_;
    QWidget *bar_ = nullptr;
    QComboBox *presetBox_ = nullptr;
    QLabel *madeFor_ = nullptr;
    QHash<QString, QWidget *> controls_;
    QHash<QString, QWidget *> wraps_;
    QPushButton *logoButton_ = nullptr;
    QWidget *stage_ = nullptr;
    QPointer<QWidget> frame_;
    QWidget *outline_ = nullptr;
    QFrame *helpCard_ = nullptr;
    QLabel *helpTitle_ = nullptr, *helpText_ = nullptr, *helpPreview_ = nullptr;
};
