/**
 * @file win_impl.cpp
 * @brief Windows 平台键盘拦截实现
 * 
 * 本文件实现了 Windows 平台的键盘拦截功能。
 * 核心原理：使用 SetWindowsHookEx 安装 WH_KEYBOARD_LL（低级键盘钩子），
 * 在键盘消息传递给目标应用程序之前进行拦截和过滤。
 * 
 * 技术要点：
 * - WH_KEYBOARD_LL 是系统级钩子，可以拦截所有进程的键盘输入
 * - 钩子回调必须在有消息循环的线程中运行
 * - 返回 1 表示拦截消息，调用 CallNextHookEx 表示放行
 */

#include "../common/platform.h"
#include <windows.h>

/*
 * Windows 特定的全局变量
 */

/** @brief 键盘钩子句柄 */
HHOOK g_hHook = NULL;

/** @brief 钩子线程句柄 */
HANDLE g_hThread = NULL;

/** @brief 钩子线程 ID，用于发送消息 */
DWORD g_dwThreadId = 0;

/** @brief 当前模块实例句柄，安装全局钩子时需要 */
HINSTANCE g_hModule = NULL;

/**
 * @brief 低级键盘钩子回调函数
 * 
 * 当任何键盘事件发生时，系统会调用此函数。
 * 函数检查当前按键是否需要被拦截，如果需要则返回 1 阻止消息传递，
 * 否则调用 CallNextHookEx 将消息传递给下一个钩子或目标应用程序。
 * 
 * @param nCode 钩子代码，HC_ACTION 表示有有效的键盘消息
 * @param wParam 消息类型（WM_KEYDOWN, WM_KEYUP 等）
 * @param lParam 指向 KBDLLHOOKSTRUCT 结构的指针，包含按键详细信息
 * @return LRESULT 返回 1 表示拦截消息，否则调用 CallNextHookEx
 */
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // HC_ACTION 表示这是一个有效的键盘动作
    if (nCode == HC_ACTION) {
        // 获取按键信息结构
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        
        // 拦截 Win 键（左 Win 键: VK_LWIN, 右 Win 键: VK_RWIN）
        if (g_isWinKeyDisabled && (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN)) {
            return 1;  // 返回 1 阻止消息传递
        }
        
        // 拦截 Alt+Tab 组合键
        // 检查 Tab 键是否按下，同时 Alt 键处于按下状态（0x8000 表示高位为1）
        if (g_isAltTabDisabled && p->vkCode == VK_TAB && (GetKeyState(VK_MENU) & 0x8000)) {
            return 1;
        }
        
        // 拦截 Alt 键（左 Alt: VK_LMENU, 右 Alt: VK_RMENU）
        if (g_isAltKeyDisabled && (p->vkCode == VK_LMENU || p->vkCode == VK_RMENU)) {
            return 1;
        }
        
        // 拦截 F11 键（通常用于全屏切换）
        if (g_isF11KeyDisabled && p->vkCode == VK_F11) {
            return 1;
        }
        
        // 拦截 Ctrl 键（左 Ctrl: VK_LCONTROL, 右 Ctrl: VK_RCONTROL）
        if (g_isCtrlKeyDisabled && (p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL)) {
            return 1;
        }
        
        // 拦截 F3 键
        if (g_isF3KeyDisabled && p->vkCode == VK_F3) {
            return 1;
        }
        
        // 拦截所有功能键 F1-F12
        if (g_isFunctionKeysDisabled && p->vkCode >= VK_F1 && p->vkCode <= VK_F12) {
            return 1;
        }
        
        // 拦截 Fn 键及其组合
        if (g_isFnKeyDisabled) {
            // Fn 键通常由硬件直接处理，但某些笔记本会发送特殊扫描码
            // 0x73 和 0xE0 是常见的 Fn 键扫描码
            if (p->scanCode == 0x73 || p->scanCode == 0xE0) {
                return 1;
            }
            
            // 检测 Fn+F1 到 Fn+F12 组合键
            if (p->vkCode >= VK_F1 && p->vkCode <= VK_F12) {
                // 检查扩展键标志（0x01）或上下文代码标志（0x10）
                // 这些标志通常表示 Fn 组合键
                if (p->flags & (0x01 | 0x10)) {
                    return 1;
                }
                // 扫描码大于 0x80 通常表示特殊功能键
                if (p->scanCode > 0x80) {
                    return 1;
                }
            }
        }
    }
    
    // 将消息传递给下一个钩子
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

/**
 * @brief 钩子线程入口函数
 * 
 * 此函数在独立线程中运行，负责：
 * 1. 安装低级键盘钩子
 * 2. 运行消息循环（钩子需要消息循环才能正常工作）
 * 3. 线程结束时卸载钩子
 * 
 * @param lpParameter 线程参数（未使用）
 * @return DWORD 返回 0 表示成功，1 表示失败
 */
DWORD WINAPI HookThreadProc(LPVOID lpParameter) {
    // 安装低级键盘钩子
    // 参数说明：
    // - WH_KEYBOARD_LL: 低级键盘钩子类型
    // - LowLevelKeyboardProc: 回调函数
    // - g_hModule: 包含回调函数的模块句柄
    // - 0: 线程 ID，0 表示监控所有线程
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, g_hModule, 0);
    if (g_hHook == NULL) return 1;

    // 运行消息循环
    // 低级键盘钩子需要消息循环来接收和处理钩子事件
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 线程结束前卸载钩子
    if (g_hHook) {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
    }
    return 0;
}

/**
 * @brief 确保钩子线程正在运行
 * 
 * 如果钩子线程尚未启动，则创建新线程并等待钩子安装完成。
 * 
 * @param env N-API 环境对象，用于在失败时抛出 JavaScript 异常
 */
void EnsureHookThreadRunning(Napi::Env env) {
    if (g_hThread == NULL) {
        // 创建钩子线程
        g_hThread = CreateThread(NULL, 0, HookThreadProc, NULL, 0, &g_dwThreadId);
        if (g_hThread == NULL) {
            Napi::TypeError::New(env, "Failed to create hook thread").ThrowAsJavaScriptException();
            return;
        }
        
        // 等待 100ms 让钩子线程完成初始化
        Sleep(100);
        
        // 检查钩子是否安装成功
        if (g_hHook == NULL) {
            Napi::TypeError::New(env, "Failed to install keyboard hook").ThrowAsJavaScriptException();
        }
    }
}

/**
 * @brief 在所有按键都启用时停止钩子线程
 * 
 * 当所有拦截标志都为 false 时，停止钩子线程以释放系统资源。
 * 使用 PostThreadMessage 发送 WM_QUIT 消息来优雅地结束消息循环。
 */
void StopHookThreadIfNeeded() {
    // 检查是否所有拦截标志都为 false
    if (g_hThread != NULL && !g_isWinKeyDisabled && !g_isAltTabDisabled && 
        !g_isAltKeyDisabled && !g_isF11KeyDisabled && !g_isCtrlKeyDisabled && 
        !g_isF3KeyDisabled && !g_isFnKeyDisabled && !g_isFunctionKeysDisabled) {
        
        // 向钩子线程发送退出消息
        PostThreadMessage(g_dwThreadId, WM_QUIT, 0, 0);
        
        // 等待线程结束（最多等待 1 秒）
        WaitForSingleObject(g_hThread, 1000);
        
        // 关闭线程句柄并清理状态
        CloseHandle(g_hThread);
        g_hThread = NULL;
        g_dwThreadId = 0;
        g_hHook = NULL;
    }
}

/**
 * @brief Windows 平台初始化
 * 
 * 获取当前模块的实例句柄，这是安装全局钩子所必需的。
 * 使用 VirtualQuery 查询 PlatformInit 函数所在内存区域的信息，
 * 从而获取模块的基地址（即 HINSTANCE）。
 * 
 * @param env N-API 环境对象
 * @param exports 模块导出对象
 */
void PlatformInit(Napi::Env env, Napi::Object exports) {
    MEMORY_BASIC_INFORMATION mbi;
    // 查询当前函数地址所在的内存区域信息
    VirtualQuery((LPCVOID)PlatformInit, &mbi, sizeof(mbi));
    // AllocationBase 就是模块的加载基地址
    g_hModule = (HINSTANCE)mbi.AllocationBase;
}
