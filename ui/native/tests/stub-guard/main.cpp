#include <stackjunkie/components.hpp>
#include <QApplication>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    layout->addWidget(StackJunkie::heading(QStringLiteral("Report")));
#ifdef STUB_GUARD_WITH_STUB
    layout->addWidget(STACKJUNKIE_STUB(QStringLiteral("Summary text goes here.")));
    layout->addWidget(STACKJUNKIE_STUB(new QLabel(QStringLiteral("42 open items"))));
#endif
    StackJunkie::Theme::load().apply(&window);
    return 0;
}
