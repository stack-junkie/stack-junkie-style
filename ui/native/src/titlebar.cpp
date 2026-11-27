// The merged title bar: the frame's top row (logo, name, menus) is also the window's title bar, with its own
// window buttons, so the app's name shows once. Dragging the row moves the window, a double click maximizes it,
// and the edges resize it. On Windows the system does all of this through hit testing, so snap layouts appear
// on the maximize button and Aero snap works; elsewhere Qt's system move and resize do it.
#include <stackjunkie/components.hpp>
#include <QApplication>
#include <QBoxLayout>
#include <QEvent>
#include <QMenuBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWindow>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif

namespace StackJunkie {
static constexpr int resizeMargin = 5;

void Workspace::setMergedTitleBar(bool merged) {
    if (merged == merged_) return;
    merged_ = merged;
    const bool shown = isVisible();
    setWindowFlag(Qt::FramelessWindowHint, merged);
    windowButtons_->setVisible(merged);
    if (auto *layout = qobject_cast<QBoxLayout *>(toolbar_->layout())) {
        const auto margins = layout->contentsMargins();
        layout->setContentsMargins(margins.left(), 0, merged ? 0 : margins.left(), 0);
    }
    updateWindowButtons();
    if (shown) show();
}

void Workspace::updateWindowButtons() {
    if (!maxButton_) return;
    const bool maximized = isMaximized();
    maxButton_->setProperty("sjIcon", maximized ? "window-restore" : "window-maximize");
    maxButton_->setAccessibleName(maximized ? QStringLiteral("Restore") : QStringLiteral("Maximize"));
    maxButton_->setToolTip(maxButton_->accessibleName());
    if (property("sjDirection").isValid())
        maxButton_->setIcon(icon(maxButton_->property("sjIcon").toString(), Theme::load(property("sjDirection").toString(), property("sjDark").toBool()).color(QStringLiteral("text"))));
}

void Workspace::toggleMaximized() { if (isMaximized()) showNormal(); else showMaximized(); }

void Workspace::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) updateWindowButtons();
    QWidget::changeEvent(event);
}

// The parts of the top row that act as the title bar: the row itself, its labels and the empty part of the Menu bar.
bool Workspace::isTitleArea(QWidget *widget, const QPoint &globalPos) const {
    if (!merged_ || !widget) return false;
    if (widget == toolbar_ || widget == heading_) return true;
    if (widget == menuBar_) return !menuBar_->actionAt(menuBar_->mapFromGlobal(globalPos));
    return false;
}

void Workspace::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    restoreZoom();
#ifdef Q_OS_WIN
    // Keep the native frame styles behind the frameless window so the system still resizes, snaps and animates it.
    if (merged_ && isWindow()) {
        auto hwnd = reinterpret_cast<HWND>(winId());
        SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU);
        const MARGINS shadow{0, 0, 1, 0};
        DwmExtendFrameIntoClientArea(hwnd, &shadow);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
#endif
}

bool Workspace::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    if (merged_ && isWindow() && eventType == "windows_generic_MSG") {
        auto *msg = static_cast<MSG *>(message);
        switch (msg->message) {
        case WM_NCCALCSIZE:
            if (msg->wParam) {
                // The whole window is client area; a maximized window keeps its content on the screen.
                if (IsZoomed(msg->hwnd)) {
                    auto *params = reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
                    const UINT dpi = GetDpiForWindow(msg->hwnd);
                    const int frameX = GetSystemMetricsForDpi(SM_CXFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
                    const int frameY = GetSystemMetricsForDpi(SM_CYFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
                    params->rgrc[0].left += frameX; params->rgrc[0].right -= frameX;
                    params->rgrc[0].top += frameY; params->rgrc[0].bottom -= frameY;
                }
                *result = 0;
                return true;
            }
            break;
        case WM_NCHITTEST: {
            RECT window; GetWindowRect(msg->hwnd, &window);
            const qreal ratio = devicePixelRatioF();
            const QPoint physical(GET_X_LPARAM(msg->lParam) - window.left, GET_Y_LPARAM(msg->lParam) - window.top);
            const QPoint local(qRound(physical.x() / ratio), qRound(physical.y() / ratio));
            const QPoint global = mapToGlobal(local);
            if (!IsZoomed(msg->hwnd)) {
                const bool left = local.x() < resizeMargin, right = local.x() >= width() - resizeMargin;
                const bool top = local.y() < resizeMargin, bottom = local.y() >= height() - resizeMargin;
                if (top && left) { *result = HTTOPLEFT; return true; }
                if (top && right) { *result = HTTOPRIGHT; return true; }
                if (bottom && left) { *result = HTBOTTOMLEFT; return true; }
                if (bottom && right) { *result = HTBOTTOMRIGHT; return true; }
                if (left) { *result = HTLEFT; return true; }
                if (right) { *result = HTRIGHT; return true; }
                if (top) { *result = HTTOP; return true; }
                if (bottom) { *result = HTBOTTOM; return true; }
            }
            // The maximize button reports itself, which is what shows the Windows 11 snap layouts.
            if (maxButton_->isVisible() && maxButton_->rect().contains(maxButton_->mapFromGlobal(global))) { *result = HTMAXBUTTON; return true; }
            if (isTitleArea(childAt(local), global)) { *result = HTCAPTION; return true; }
            *result = HTCLIENT;
            return true;
        }
        case WM_NCMOUSEMOVE:
            maxButton_->setProperty("sjHover", msg->wParam == HTMAXBUTTON); maxButton_->style()->polish(maxButton_);
            break;
        case WM_NCMOUSELEAVE: case WM_MOUSELEAVE:
            maxButton_->setProperty("sjHover", false); maxButton_->style()->polish(maxButton_);
            break;
        case WM_NCLBUTTONDOWN:
            if (msg->wParam == HTMAXBUTTON) { *result = 0; return true; }
            break;
        case WM_NCLBUTTONUP:
            if (msg->wParam == HTMAXBUTTON) { toggleMaximized(); *result = 0; return true; }
            break;
        default: break;
        }
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

// Outside Windows: drag the title area to move, double-click it to maximize, and drag an edge to resize.
bool Workspace::eventFilter(QObject *object, QEvent *event) {
    auto *widget = qobject_cast<QWidget *>(object);
    if (event->type() == QEvent::Wheel && zoomWheel(widget, static_cast<QWheelEvent *>(event))) return true;
#ifdef Q_OS_WIN
    // On Windows the system moves and resizes the merged title bar's window (nativeEvent).
    return QWidget::eventFilter(object, event);
#else
    if (!merged_ || !widget || widget->window() != this || !windowHandle()) return QWidget::eventFilter(object, event);
    const auto type = event->type();
    if (type != QEvent::MouseMove && type != QEvent::MouseButtonPress && type != QEvent::MouseButtonDblClick) return QWidget::eventFilter(object, event);
    auto *mouse = static_cast<QMouseEvent *>(event);
    const QPoint global = mouse->globalPosition().toPoint(), local = mapFromGlobal(global);
    Qt::Edges edges;
    if (!isMaximized()) {
        if (local.x() < resizeMargin) edges |= Qt::LeftEdge;
        if (local.x() >= width() - resizeMargin) edges |= Qt::RightEdge;
        if (local.y() < resizeMargin) edges |= Qt::TopEdge;
        if (local.y() >= height() - resizeMargin) edges |= Qt::BottomEdge;
    }
    if (type == QEvent::MouseMove && !mouse->buttons()) {
        const auto shape = edges == (Qt::LeftEdge | Qt::TopEdge) || edges == (Qt::RightEdge | Qt::BottomEdge) ? Qt::SizeFDiagCursor
            : edges == (Qt::RightEdge | Qt::TopEdge) || edges == (Qt::LeftEdge | Qt::BottomEdge) ? Qt::SizeBDiagCursor
            : edges & (Qt::LeftEdge | Qt::RightEdge) ? Qt::SizeHorCursor : edges ? Qt::SizeVerCursor : Qt::ArrowCursor;
        if (edges && !edgeCursor_) { QGuiApplication::setOverrideCursor(shape); edgeCursor_ = true; }
        else if (edges) QGuiApplication::changeOverrideCursor(shape);
        else if (edgeCursor_) { QGuiApplication::restoreOverrideCursor(); edgeCursor_ = false; }
        return false;
    }
    if (mouse->button() != Qt::LeftButton) return false;
    if (type == QEvent::MouseButtonPress && edges) { windowHandle()->startSystemResize(edges); return true; }
    if (!isTitleArea(widget, global)) return false;
    if (type == QEvent::MouseButtonDblClick) { toggleMaximized(); return true; }
    windowHandle()->startSystemMove();
    return true;
#endif
}
}
