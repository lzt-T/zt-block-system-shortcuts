/**
 * @file addon.cpp
 * @brief Node.js 原生插件入口文件
 * 
 * 本文件实现了 N-API 绑定，将 C++ 函数导出为 JavaScript 可调用的接口。
 * 提供 disableAll() 和 enableAll() 两个核心 API。
 */

#include "platform.h"

/*
 * 全局状态变量定义
 * 默认情况下所有按键都处于拦截状态（true）
 */

/** @brief Win/Command 键拦截状态，默认拦截 */
bool g_isWinKeyDisabled = true;

/** @brief Alt+Tab 拦截状态，默认拦截 */
bool g_isAltTabDisabled = true;

/** @brief Alt/Option 键拦截状态，默认拦截 */
bool g_isAltKeyDisabled = true;

/** @brief F11 键拦截状态，默认拦截 */
bool g_isF11KeyDisabled = true;

/** @brief Ctrl/Control 键拦截状态，默认拦截 */
bool g_isCtrlKeyDisabled = true;

/** @brief F3 键拦截状态，默认拦截 */
bool g_isF3KeyDisabled = true;

/** @brief Fn 键拦截状态，默认拦截 */
bool g_isFnKeyDisabled = true;

/** @brief F1-F12 功能键拦截状态，默认拦截 */
bool g_isFunctionKeysDisabled = true;

/**
 * @brief 禁用所有系统快捷键
 * 
 * 将所有拦截标志设置为 true，并确保钩子线程正在运行。
 * 调用后，所有被管理的系统快捷键将被拦截，无法触发系统功能。
 * 
 * @param info N-API 回调信息对象
 * @return Napi::Value 返回 true 表示操作成功
 */
Napi::Value disableAll(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // 设置所有拦截标志为 true
    g_isWinKeyDisabled = true;
    g_isAltTabDisabled = true;
    g_isAltKeyDisabled = true;
    g_isF3KeyDisabled = true;
    g_isCtrlKeyDisabled = true;
    g_isF11KeyDisabled = true;
    g_isFnKeyDisabled = true;
    g_isFunctionKeysDisabled = true;
    
    // 确保钩子线程正在运行
    EnsureHookThreadRunning(env);
    
    return Napi::Boolean::New(env, true);
}

/**
 * @brief 启用所有系统快捷键
 * 
 * 将所有拦截标志设置为 false，并在条件满足时停止钩子线程。
 * 调用后，所有系统快捷键恢复正常功能。
 * 
 * @param info N-API 回调信息对象
 * @return Napi::Value 返回 true 表示操作成功
 */
Napi::Value enableAll(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // 设置所有拦截标志为 false
    g_isWinKeyDisabled = false;
    g_isAltTabDisabled = false;
    g_isAltKeyDisabled = false;
    g_isF3KeyDisabled = false;
    g_isCtrlKeyDisabled = false;
    g_isF11KeyDisabled = false;
    g_isFnKeyDisabled = false;
    g_isFunctionKeysDisabled = false;
    
    // 尝试停止钩子线程以释放资源
    StopHookThreadIfNeeded();
    
    return Napi::Boolean::New(env, true);
}

/**
 * @brief 模块初始化函数
 * 
 * 在模块加载时由 Node.js 调用，负责：
 * 1. 执行平台特定初始化
 * 2. 将 C++ 函数导出为 JavaScript 可调用的方法
 * 
 * @param env N-API 环境对象
 * @param exports 模块导出对象
 * @return Napi::Object 返回填充后的导出对象
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // 执行平台特定初始化
    PlatformInit(env, exports);
    
    // 导出 disableAll 和 enableAll 方法
    exports.Set("disableAll", Napi::Function::New(env, disableAll));
    exports.Set("enableAll", Napi::Function::New(env, enableAll));
    
    return exports;
}

// 注册 Node.js 原生模块
// 模块名称: zt_block_system_shortcuts
// 初始化函数: Init
NODE_API_MODULE(zt_block_system_shortcuts, Init)
