/**
 * @file platform.h
 * @brief 跨平台键盘拦截模块的公共接口定义
 * 
 * 本文件定义了 Windows 和 macOS 平台实现需要遵循的统一接口，
 * 以及用于控制各类按键拦截状态的全局变量声明。
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <napi.h>

/**
 * @brief 确保键盘钩子线程正在运行
 * 
 * 如果钩子线程尚未启动，则创建并启动它。
 * Windows 平台使用 WH_KEYBOARD_LL 低级键盘钩子。
 * macOS 平台使用 CGEventTap 事件拦截器。
 * 
 * @param env Node.js N-API 环境对象，用于抛出 JavaScript 异常
 */
void EnsureHookThreadRunning(Napi::Env env);

/**
 * @brief 在所有按键都启用时停止钩子线程
 * 
 * 检查所有拦截标志，如果全部为 false（即不拦截任何按键），
 * 则停止钩子线程以释放系统资源。
 */
void StopHookThreadIfNeeded();

/**
 * @brief 平台特定的初始化函数
 * 
 * 在模块加载时调用，用于执行平台特定的初始化操作。
 * Windows: 获取当前模块句柄，用于安装全局钩子。
 * macOS: 无需特殊初始化。
 * 
 * @param env Node.js N-API 环境对象
 * @param exports 模块导出对象
 */
void PlatformInit(Napi::Env env, Napi::Object exports);

/*
 * 全局状态变量声明
 * 这些变量控制各类按键的拦截状态，true 表示拦截，false 表示放行
 */

/** @brief Win 键（Windows）/ Command 键（macOS）拦截状态 */
extern bool g_isWinKeyDisabled;

/** @brief Alt+Tab 组合键拦截状态 */
extern bool g_isAltTabDisabled;

/** @brief Alt 键（Windows）/ Option 键（macOS）拦截状态 */
extern bool g_isAltKeyDisabled;

/** @brief F11 键拦截状态 */
extern bool g_isF11KeyDisabled;

/** @brief Ctrl 键（Windows）/ Control 键（macOS）拦截状态 */
extern bool g_isCtrlKeyDisabled;

/** @brief F3 键拦截状态（macOS 上通常触发 Mission Control） */
extern bool g_isF3KeyDisabled;

/** @brief Fn 键及其组合键拦截状态 */
extern bool g_isFnKeyDisabled;

/** @brief 所有功能键 F1-F12 拦截状态 */
extern bool g_isFunctionKeysDisabled;

#endif // PLATFORM_H
