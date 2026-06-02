// ============================================================================
//  UsbDeviceMonitor.cpp  —  U 盘热插拔监听器实现
// ============================================================================

#include "UsbDeviceMonitor.h"

// <Windows.h> 已由 UsbDeviceMonitor.h 含 NOMINMAX 保护引入，此处不再重复
#include <dbt.h>        // DBT_DEVICEARRIVAL, DEV_BROADCAST_VOLUME, etc.

#include <chrono>

// ===========================================================================
//  静态成员初始化
// ===========================================================================

// 每实例一个唯一窗口类名，避免 RegisterClassEx 重复注册冲突
std::atomic<int> UsbDeviceMonitor::s_instanceCounter{ 0 };

// ===========================================================================
//  构造 / 析构
// ===========================================================================

UsbDeviceMonitor::UsbDeviceMonitor()
    : m_hWnd(nullptr)
    , m_hDevNotify(nullptr)
    , m_running(false)
{
    // 生成唯一窗口类名：UsbDevMon_N
    int id = s_instanceCounter.fetch_add(1, std::memory_order_relaxed);
    m_windowClassName = L"UsbDevMon_" + std::to_wstring(id);
}

UsbDeviceMonitor::~UsbDeviceMonitor() {
    stop();   // RAII：确保线程安全退出
}

// ===========================================================================
//  公开接口
// ===========================================================================

void UsbDeviceMonitor::setOnDeviceArrived(DeviceEventCallback callback) {
    m_onArrived = std::move(callback);
}

void UsbDeviceMonitor::setOnDeviceRemoved(DeviceEventCallback callback) {
    m_onRemoved = std::move(callback);
}

bool UsbDeviceMonitor::isMonitoring() const {
    return m_running.load(std::memory_order_acquire);
}

// ===========================================================================
//  启动监听
// ===========================================================================

bool UsbDeviceMonitor::start() {
    // 幂等：已在监听则直接返回成功
    if (m_running.load(std::memory_order_acquire)) {
        return true;
    }

    m_running.store(true, std::memory_order_release);
    m_thread = std::thread(&UsbDeviceMonitor::messageLoop, this);

    // 自旋等待窗口创建完成（通常 < 10ms）
    for (int i = 0; i < 200 && m_hWnd == nullptr; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    if (m_hWnd == nullptr) {
        // 窗口创建失败 —— 这可能发生在极端条件下
        m_running.store(false, std::memory_order_release);
        if (m_thread.joinable()) {
            m_thread.join();   // 线程可能已 exit，安全 join
        }
        return false;
    }

    return true;
}

// ===========================================================================
//  停止监听（线程安全，支持从回调内部自停）
// ===========================================================================

void UsbDeviceMonitor::stop() {
    if (!m_running.load(std::memory_order_acquire)) {
        return;   // 未运行，无需操作
    }

    m_running.store(false, std::memory_order_release);

    // ---- 情形 A：从监听线程内部调用（例如在回调中 stop）-------------------
    if (m_thread.joinable() && m_thread.get_id() == std::this_thread::get_id()) {
        // 不能 join 自己，否则死锁。直接发送 WM_QUIT 退出消息循环。
        // 消息循环收到 WM_QUIT 后 GetMessage 返回 0，线程退出。
        PostQuitMessage(0);
        m_thread.detach();
        return;
    }

    // ---- 情形 B：从外部线程调用（正常路径）---------------------------------
    HWND hWnd = m_hWnd;
    if (hWnd != nullptr) {
        // 向消息窗口发送自定义退出消息
        // WndProc 收到后 → DestroyWindow → WM_DESTROY → PostQuitMessage
        PostMessageW(hWnd, WM_MONITOR_EXIT, 0, 0);
    }

    if (m_thread.joinable()) {
        // 最多等待 3 秒（正常情况下瞬间返回）
        // 超时保护：防止极端情况下线程卡死导致主程序无法退出
        auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (std::chrono::steady_clock::now() < timeout) {
            // 使用 timed_join 风格的轮询
            // std::thread 没有 timed_join，使用变通方案
            if (m_hWnd == nullptr) {
                // 线程已清理窗口，即将退出
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        m_thread.join();
    }
}

// ===========================================================================
//  工具函数：位掩码 → 盘符
// ===========================================================================

std::wstring UsbDeviceMonitor::maskToDriveLetter(DWORD unitMask) {
    std::wstring result;
    result.reserve(3);   // e.g., L"D:"

    for (int i = 0; i < 26; ++i) {
        if (unitMask & (1 << i)) {
            result.push_back(static_cast<wchar_t>(L'A' + i));
            result.push_back(L':');
            break;   // 一次设备事件通常只涉及一个盘符
        }
    }
    return result;
}

// ===========================================================================
//  后台线程消息循环
// ===========================================================================

void UsbDeviceMonitor::messageLoop() {
    // ---------- 1. 注册窗口类 -----------------------------------------------
    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc   = UsbDeviceMonitor::WndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.lpszClassName = m_windowClassName.c_str();
    wc.style         = 0;

    ATOM atom = RegisterClassExW(&wc);
    if (atom == 0) {
        // 注册失败（可能类名冲突），设置标志后退出
        m_running.store(false, std::memory_order_release);
        return;
    }

    // ---------- 2. 创建仅消息窗口 -------------------------------------------
    //  HWND_MESSAGE 作为父窗口 → 系统创建一个仅用于接收消息的隐藏窗口
    //  lpParam 传递 this 指针，在 WM_NCCREATE 中提取并绑定到窗口用户数据
    m_hWnd = CreateWindowExW(
        0,                              // dwExStyle
        m_windowClassName.c_str(),      // 窗口类名
        L"",                            // 窗口标题（不需要）
        0,                              // dwStyle（消息窗口无需样式）
        0, 0, 0, 0,                    // 位置和大小（无意义）
        HWND_MESSAGE,                   // 父窗口 = 仅消息窗口句柄
        nullptr,                        // 菜单
        GetModuleHandleW(nullptr),      // 实例句柄
        this                            // lpParam → WM_NCCREATE
    );

    if (m_hWnd == nullptr) {
        UnregisterClassW(m_windowClassName.c_str(), GetModuleHandleW(nullptr));
        m_running.store(false, std::memory_order_release);
        return;
    }

    // ---------- 3. 注册设备通知（卷设备）------------------------------------
    DEV_BROADCAST_VOLUME dbv = {};
    dbv.dbcv_size       = sizeof(DEV_BROADCAST_VOLUME);
    dbv.dbcv_devicetype  = DBT_DEVTYP_VOLUME;
    dbv.dbcv_unitmask    = 0;   // 监听所有逻辑卷

    m_hDevNotify = RegisterDeviceNotificationW(
        m_hWnd,
        &dbv,
        DEVICE_NOTIFY_WINDOW_HANDLE
    );
    // 如果注册失败，m_hDevNotify 为 nullptr。
    // 窗口仍然存在，只是收不到 WM_DEVICECHANGE。不中断，让调用者自行判断。

    // ---------- 4. 消息泵 --------------------------------------------------
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        DispatchMessageW(&msg);
    }

    // ---------- 5. 清理（GetMessage 因 WM_QUIT 返回 0 后到达）---------------
    // 注意：如果 WM_DESTROY 已处理，设备通知已注销，窗口已销毁。
    // 这里做兜底清理。

    if (m_hDevNotify != nullptr) {
        UnregisterDeviceNotification(m_hDevNotify);
        m_hDevNotify = nullptr;
    }

    if (m_hWnd != nullptr) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    UnregisterClassW(m_windowClassName.c_str(), GetModuleHandleW(nullptr));

    m_running.store(false, std::memory_order_release);
}

// ===========================================================================
//  窗口过程
// ===========================================================================

LRESULT CALLBACK UsbDeviceMonitor::WndProc(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UsbDeviceMonitor* monitor = nullptr;

    // ---- WM_NCCREATE：窗口创建时最早收到的消息，提取实例指针 ---------------
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        monitor = static_cast<UsbDeviceMonitor*>(cs->lpCreateParams);
        // 将实例指针存入窗口用户数据，后续消息均可取出
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(monitor));
        // 返回 TRUE 继续窗口创建；FALSE 则 CreateWindowEx 返回 NULL
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    // ---- 其他消息：从用户数据中取出实例指针 --------------------------------
    monitor = reinterpret_cast<UsbDeviceMonitor*>(
        GetWindowLongPtrW(hWnd, GWLP_USERDATA));

    if (monitor == nullptr) {
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    switch (msg) {

    // ---- 设备变更通知 ------------------------------------------------------
    case WM_DEVICECHANGE:
        monitor->handleDeviceChange(wParam, lParam);
        return TRUE;   // 已处理，阻止进一步传播

    // ---- 自定义退出消息（由 stop() 发送）-----------------------------------
    case WM_MONITOR_EXIT:
        DestroyWindow(hWnd);
        return 0;

    // ---- 窗口销毁 ----------------------------------------------------------
    case WM_DESTROY:
        // 注销设备通知
        if (monitor->m_hDevNotify != nullptr) {
            UnregisterDeviceNotification(monitor->m_hDevNotify);
            monitor->m_hDevNotify = nullptr;
        }
        // 清空窗口句柄（防止 stop() 重复 PostMessage）
        monitor->m_hWnd = nullptr;
        // 向本线程发送 WM_QUIT，使 GetMessage 返回 0，退出消息循环
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ===========================================================================
//  处理 WM_DEVICECHANGE
// ===========================================================================

void UsbDeviceMonitor::handleDeviceChange(WPARAM wParam, LPARAM lParam) {
    // 仅处理插入和完全移除事件
    if (wParam != DBT_DEVICEARRIVAL && wParam != DBT_DEVICEREMOVECOMPLETE) {
        return;
    }

    // lParam 为 DEV_BROADCAST_HDR 指针，为 0 则忽略
    if (lParam == 0) {
        return;
    }

    PDEV_BROADCAST_HDR header = reinterpret_cast<PDEV_BROADCAST_HDR>(lParam);

    // ---- 仅处理逻辑卷 ------------------------------------------------------
    if (header->dbch_devicetype != DBT_DEVTYP_VOLUME) {
        return;   // 忽略：DBT_DEVTYP_DEVICE（键盘/鼠标等）、DBT_DEVTYP_PORT 等
    }

    PDEV_BROADCAST_VOLUME vol = reinterpret_cast<PDEV_BROADCAST_VOLUME>(header);

    // 从位掩码中提取盘符
    std::wstring driveLetter = maskToDriveLetter(vol->dbcv_unitmask);
    if (driveLetter.empty()) {
        return;
    }

    // ---- 分发到对应回调 ----------------------------------------------------
    switch (wParam) {
    case DBT_DEVICEARRIVAL:
        if (m_onArrived) {
            m_onArrived(driveLetter);
        }
        break;

    case DBT_DEVICEREMOVECOMPLETE:
        if (m_onRemoved) {
            m_onRemoved(driveLetter);
        }
        break;
    }
}
