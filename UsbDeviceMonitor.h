#pragma once

// ============================================================================
//  UsbDeviceMonitor.h  —  U 盘热插拔监听器（Headless，纯 Win32 API）
//  ============================================================================
//  职责：
//    1. 创建隐藏的消息窗口（Message-Only Window）运行独立消息循环
//    2. 通过 RegisterDeviceNotification 注册卷设备通知
//    3. 在 WM_DEVICECHANGE 中解析 DBT_DEVICEARRIVAL / DBT_DEVICEREMOVECOMPLETE
//    4. 仅处理逻辑卷（DBT_DEVTYP_VOLUME），忽略键盘/鼠标等其他设备
//    5. 通过 std::function 回调通知外部（线程安全设计）
//
//  依赖：仅 Win32 API + C++17 标准库，零 UI 框架依赖
// ============================================================================

// NOMINMAX 必须在 <Windows.h> 之前定义，否则 min/max 宏会污染 std::min/max
// 导致 Qt 头文件编译失败（常见于 MinGW 和部分 MSVC 配置）
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>    // HWND, LRESULT, HDEVNOTIFY, WM_USER, ...

#include <atomic>
#include <functional>
#include <string>
#include <thread>

class UsbDeviceMonitor {
public:
    // ---- 回调类型 ----------------------------------------------------------
    //  driveLetter: 盘符，如 L"D:"
    using DeviceEventCallback = std::function<void(std::wstring driveLetter)>;

    // ---- 构造 / 析构 -------------------------------------------------------
    UsbDeviceMonitor();
    ~UsbDeviceMonitor();

    // 禁止拷贝 & 移动
    UsbDeviceMonitor(const UsbDeviceMonitor&) = delete;
    UsbDeviceMonitor& operator=(const UsbDeviceMonitor&) = delete;
    UsbDeviceMonitor(UsbDeviceMonitor&&) = delete;
    UsbDeviceMonitor& operator=(UsbDeviceMonitor&&) = delete;

    // ---- 设置回调（必须在 start() 之前调用）---------------------------------
    void setOnDeviceArrived(DeviceEventCallback callback);
    void setOnDeviceRemoved(DeviceEventCallback callback);

    // ---- 启动监听（创建后台线程 + 消息窗口）---------------------------------
    //  返回 true 表示启动成功
    bool start();

    // ---- 停止监听（发送退出消息 + join 线程，线程安全）-----------------------
    void stop();

    // ---- 查询状态 ----------------------------------------------------------
    bool isMonitoring() const;

private:
    // ---- 后台线程主函数 ----------------------------------------------------
    void messageLoop();

    // ---- 窗口过程（静态函数，转发到实例）-------------------------------------
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // ---- 处理 WM_DEVICECHANGE 消息 -----------------------------------------
    void handleDeviceChange(WPARAM wParam, LPARAM lParam);

    // ---- 工具：将 DWORD 位掩码转换为盘符字符串 ------------------------------
    //  例如 bit 3 为 1 → L"D:"
    static std::wstring maskToDriveLetter(DWORD unitMask);

    // ---- 退出消息常量 ------------------------------------------------------
    static constexpr UINT WM_MONITOR_EXIT = WM_USER + 100;

    // ---- 窗口相关 ----------------------------------------------------------
    std::wstring m_windowClassName;    // 每实例唯一，避免类名冲突
    HWND         m_hWnd = nullptr;     // 消息窗口句柄
    HDEVNOTIFY   m_hDevNotify = nullptr; // 设备通知句柄

    // ---- 线程 --------------------------------------------------------------
    std::thread m_thread;
    std::atomic<bool> m_running{ false };

    // ---- 回调函数对象 ------------------------------------------------------
    DeviceEventCallback m_onArrived;
    DeviceEventCallback m_onRemoved;

    // ---- 实例计数器（用于生成唯一窗口类名）----------------------------------
    static std::atomic<int> s_instanceCounter;
};
