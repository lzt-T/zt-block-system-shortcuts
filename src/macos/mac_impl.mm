/**
 * @file mac_impl.mm
 * @brief macOS 平台键盘拦截实现
 * 
 * 本文件实现了 macOS 平台的键盘拦截功能。
 * 核心原理：使用 CGEventTapCreate 创建事件拦截器（Event Tap），
 * 在键盘事件传递给目标应用程序之前进行拦截和过滤。
 * 
 * 技术要点：
 * - CGEventTap 是 macOS 底层的事件拦截机制
 * - 需要辅助功能权限才能拦截其他应用程序的键盘事件
 * - 返回 NULL 表示消费（拦截）事件，返回原事件表示放行
 * - 事件处理必须在有 CFRunLoop 的线程中运行
 * 
 * 注意：使用此功能需要用户在系统偏好设置中授予辅助功能权限
 */

#include "../common/platform.h"
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>  // 包含虚拟键码定义（kVK_*）
#include <iostream>
#include <pthread.h>

/*
 * macOS 特定的全局变量
 */

/** @brief 事件拦截器（Event Tap）句柄 */
CFMachPortRef g_eventTap = NULL;

/** @brief RunLoop 事件源 */
CFRunLoopSourceRef g_runLoopSource = NULL;

/** @brief RunLoop 句柄 */
CFRunLoopRef g_runLoop = NULL;

/** @brief 事件处理线程 */
pthread_t g_thread;

/**
 * @brief 事件拦截回调函数
 * 
 * 当键盘事件发生时，系统会调用此函数。
 * 函数检查当前按键是否需要被拦截：
 * - 返回 NULL: 消费事件（拦截），事件不会传递给其他应用
 * - 返回 event: 放行事件，事件继续传递
 * 
 * @param proxy 事件代理（未使用）
 * @param type 事件类型
 * @param event 事件对象
 * @param refcon 用户数据（未使用）
 * @return CGEventRef 返回 NULL 表示拦截，返回 event 表示放行
 */
CGEventRef EventTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    // 只处理键盘相关事件
    if (type == kCGEventKeyDown || type == kCGEventKeyUp || type == kCGEventFlagsChanged) {
        // 获取修饰键状态和按键码
        CGEventFlags flags = CGEventGetFlags(event);
        CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

        // 拦截 Option + keyCode 103 组合（特殊功能键）
        if ((flags & kCGEventFlagMaskAlternate) && keyCode == 103) {
            return NULL;
        }

        // 拦截 Command 键（macOS 的 Win/Super 键等价物）
        // keyCode 55: 左 Command, 54: 右 Command, 103: 某些键盘的特殊 Command
        if (g_isWinKeyDisabled && (flags & kCGEventFlagMaskCommand) && 
            (keyCode == 55 || keyCode == 54 || keyCode == 103)) {
            return NULL;
        }

        // 拦截 Command+Tab（应用切换）
        // kVK_Tab 是 Tab 键的虚拟键码
        if (g_isAltTabDisabled && (flags & kCGEventFlagMaskCommand) && keyCode == kVK_Tab) {
            return NULL;
        }

        // 拦截 Option 键（macOS 的 Alt 键等价物）
        if (g_isAltKeyDisabled && (flags & kCGEventFlagMaskAlternate)) {
            return NULL;
        }

        // 拦截 F11 键（通常用于显示桌面）
        if (g_isF11KeyDisabled && keyCode == kVK_F11) {
            return NULL;
        }

        // 拦截 Control 键
        if (g_isCtrlKeyDisabled && (flags & kCGEventFlagMaskControl)) {
            return NULL;
        }

        // 拦截 Command+Q（退出应用）
        // 这是额外的安全措施，防止用户意外退出应用
        if ((flags & kCGEventFlagMaskCommand) && keyCode == kVK_ANSI_Q) {
            return NULL;
        }

        // 拦截 F3 键及 Mission Control 相关键码
        if (g_isF3KeyDisabled) {
            if (keyCode == kVK_F3) {
                return NULL;
            }
            // 这些键码是 Mission Control 可能使用的特殊功能键
            // 不同 macOS 版本和键盘可能使用不同的键码
            if (keyCode == 160 || keyCode == 131 || keyCode == 179 || keyCode == 130) {
                return NULL;
            }
        }

        // 拦截所有功能键 F1-F12
        if (g_isFunctionKeysDisabled && 
            (keyCode == kVK_F1 || keyCode == kVK_F2 || keyCode == kVK_F3 || 
             keyCode == kVK_F4 || keyCode == kVK_F5 || keyCode == kVK_F6 || 
             keyCode == kVK_F7 || keyCode == kVK_F8 || keyCode == kVK_F9 || 
             keyCode == kVK_F10 || keyCode == kVK_F11 || keyCode == kVK_F12)) {
            return NULL;
        }

        // 拦截 Fn 键及其组合
        if (g_isFnKeyDisabled) {
            // macOS 上 Fn 键的键码通常是 63
            if (keyCode == 63) {
                return NULL;
            }
            
            // 检测 Fn+功能键组合
            // 当 Fn 键被按下时，flags 中会设置特定标志位（0x800000）
            if (keyCode == kVK_F1 || keyCode == kVK_F2 || keyCode == kVK_F3 || 
                keyCode == kVK_F4 || keyCode == kVK_F5 || keyCode == kVK_F6 ||
                keyCode == kVK_F7 || keyCode == kVK_F8 || keyCode == kVK_F9 ||
                keyCode == kVK_F10 || keyCode == kVK_F11 || keyCode == kVK_F12) {
                // 0x800000 是 Fn 修饰键的标志位
                if (flags & 0x800000) {
                    return NULL;
                }
            }
            
            // 拦截 Fn 键触发的特殊功能键
            // 这些键码对应各种系统功能（亮度、音量、播放控制等）
            if (keyCode == 145 || keyCode == 160 || keyCode == 144 || 
                keyCode == 131 || keyCode == 96 || keyCode == 97 || 
                keyCode == 177 || keyCode == 176 || keyCode == 178) {
                return NULL;
            }
        }
    }
    
    // 放行事件
    return event;
}

/**
 * @brief 事件处理线程入口函数
 * 
 * 此函数在独立线程中运行，负责：
 * 1. 创建 CGEventTap 事件拦截器
 * 2. 将事件源添加到 RunLoop
 * 3. 运行 RunLoop 处理事件
 * 4. 线程结束时清理资源
 * 
 * @param arg 线程参数（未使用）
 * @return void* 返回 NULL
 */
void *RunLoopThread(void *arg) {
    // 获取当前线程的 RunLoop
    g_runLoop = CFRunLoopGetCurrent();

    // 创建事件拦截器
    // 参数说明：
    // - kCGSessionEventTap: 在用户会话级别拦截事件
    // - kCGHeadInsertEventTap: 在事件队列头部插入（优先处理）
    // - kCGEventTapOptionDefault: 默认选项（可以修改和拦截事件）
    // - kCGEventMaskForAllEvents: 监听所有类型的事件
    // - EventTapCallback: 回调函数
    // - NULL: 用户数据
    g_eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 
                                   kCGEventTapOptionDefault, kCGEventMaskForAllEvents, 
                                   EventTapCallback, NULL);
    if (!g_eventTap) {
        std::cerr << "Failed to create event tap" << std::endl;
        return NULL;
    }

    // 创建 RunLoop 事件源并添加到 RunLoop
    g_runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, g_eventTap, 0);
    CFRunLoopAddSource(g_runLoop, g_runLoopSource, kCFRunLoopCommonModes);
    
    // 启用事件拦截
    CGEventTapEnable(g_eventTap, true);

    // 运行 RunLoop（阻塞直到 CFRunLoopStop 被调用）
    CFRunLoopRun();

    // 清理资源
    CGEventTapEnable(g_eventTap, false);
    if (g_runLoopSource) CFRelease(g_runLoopSource);
    if (g_eventTap) CFRelease(g_eventTap);
    g_runLoop = NULL;
    g_eventTap = NULL;
    g_runLoopSource = NULL;

    return NULL;
}

/**
 * @brief 确保事件处理线程正在运行
 * 
 * 如果线程尚未启动，则创建新线程并等待事件拦截器初始化完成。
 * 
 * @param env N-API 环境对象，用于在失败时抛出 JavaScript 异常
 */
void EnsureHookThreadRunning(Napi::Env env) {
    if (!g_runLoop) {
        // 创建事件处理线程
        if (pthread_create(&g_thread, NULL, RunLoopThread, NULL) != 0) {
            Napi::TypeError::New(env, "Failed to create thread").ThrowAsJavaScriptException();
            return;
        }
        
        // 等待 1 秒让线程完成初始化
        sleep(1);
        
        // 检查事件拦截器是否创建成功
        if (!g_eventTap) {
            Napi::TypeError::New(env, "Failed to install keyboard hook").ThrowAsJavaScriptException();
        }
    }
}

/**
 * @brief 在所有按键都启用时停止事件处理线程
 * 
 * 当所有拦截标志都为 false 时，停止 RunLoop 并等待线程结束。
 */
void StopHookThreadIfNeeded() {
    // 检查是否所有拦截标志都为 false
    if (g_runLoop && !g_isWinKeyDisabled && !g_isAltTabDisabled && 
        !g_isAltKeyDisabled && !g_isF11KeyDisabled && !g_isCtrlKeyDisabled && 
        !g_isF3KeyDisabled && !g_isFnKeyDisabled && !g_isFunctionKeysDisabled) {
        
        // 停止 RunLoop
        CFRunLoopStop(g_runLoop);
        
        // 等待线程结束
        pthread_join(g_thread, NULL);
    }
}

/**
 * @brief macOS 平台初始化
 * 
 * macOS 平台不需要特殊的初始化操作。
 * 事件拦截器在 EnsureHookThreadRunning 中按需创建。
 * 
 * @param env N-API 环境对象
 * @param exports 模块导出对象
 */
void PlatformInit(Napi::Env env, Napi::Object exports) {
    // macOS 平台无需特殊初始化
}
