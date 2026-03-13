# zt-block-system-shortcuts

跨平台键盘快捷键拦截库，支持 Windows 和 macOS，基于原生 C++ 实现，可阻止 Win 键、Alt+Tab、Ctrl、F1-F12 等系统快捷键。

[![npm version](https://img.shields.io/npm/v/zt-block-system-shortcuts.svg)](https://www.npmjs.com/package/zt-block-system-shortcuts)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 功能特性

- 跨平台支持：Windows 和 macOS
- 拦截 Win 键（Windows）/ Command 键（macOS）
- 拦截 Alt+Tab 应用切换
- 拦截 Alt 键（Windows）/ Option 键（macOS）
- 拦截 Ctrl 键（Windows）/ Control 键（macOS）
- 拦截 F1-F12 功能键
- 拦截 F3 键（macOS 上触发 Mission Control）
- 拦截 Fn 键及其组合
- 原生 C++ 实现，性能优越
- 预构建二进制文件，自动从 GitHub Releases 下载，无需本地编译环境
- 兼容所有主流包管理器（npm / yarn / pnpm）
- 支持 Node.js 14+，兼容所有版本（基于 Node-API）

## 安装

```bash
npm install zt-block-system-shortcuts
# 或
yarn add zt-block-system-shortcuts
# 或
pnpm add zt-block-system-shortcuts
```

> **pnpm 用户注意**：pnpm v10 默认阻止包运行安装脚本。推荐在项目的 `package.json` 中添加以下配置，让 pnpm 在安装时自动下载预编译文件：
>
> ```json
> {
>   "pnpm": {
>     "onlyBuiltDependencies": ["zt-block-system-shortcuts"]
>   }
> }
> ```
>
> 若未添加此配置，本库也会在首次 `require()` 时自动从 GitHub Releases 下载预编译文件。

## 使用方法

### CommonJS

```javascript
const { disableAll, enableAll, KeyManager } = require('zt-block-system-shortcuts');

// 方式一：使用导出的函数
disableAll();  // 禁用所有系统快捷键
// ... 你的应用逻辑（用户无法切换应用或触发系统功能）...
enableAll();   // 恢复所有系统快捷键

// 方式二：使用 KeyManager 类
const km = new KeyManager();
km.disableAll();
// ... 你的应用逻辑 ...
km.enableAll();
```

### TypeScript / ESM

```typescript
import { disableAll, enableAll, KeyManager } from 'zt-block-system-shortcuts';

// 禁用所有系统快捷键
disableAll();

// 应用逻辑...

// 恢复所有系统快捷键
enableAll();
```

## API

### `disableAll(): boolean`

禁用所有系统快捷键。

**返回值：** `boolean` - 操作成功返回 `true`

**拦截的按键：**
- Win 键（Windows）/ Command 键（macOS）
- Alt+Tab 组合键
- Alt 键（Windows）/ Option 键（macOS）
- Ctrl 键（Windows）/ Control 键（macOS）
- F1-F12 功能键
- F3 键（Mission Control）
- Fn 键及其组合
- Command+Q（macOS，防止退出应用）

### `enableAll(): boolean`

恢复所有系统快捷键。

**返回值：** `boolean` - 操作成功返回 `true`

### `class KeyManager`

用于管理键盘状态的类封装。

**方法：**
- `disableAll(): boolean` - 禁用所有快捷键
- `enableAll(): boolean` - 恢复所有快捷键

**类型定义：**

```typescript
declare function disableAll(): boolean;
declare function enableAll(): boolean;

declare class KeyManager {
  constructor();
  disableAll(): boolean;
  enableAll(): boolean;
}
```

## 平台支持

| 系统 | 架构 | 状态 |
|------|------|------|
| Windows | x64 | ✅ |
| Windows | ARM64 | ✅ |
| macOS | x64 | ✅ |
| macOS | ARM64（Apple Silicon）| ✅ |

**Node.js 版本：** >= 14.0.0

## 实现原理

### Windows

使用 `SetWindowsHookEx` 安装 `WH_KEYBOARD_LL`（低级键盘钩子），在键盘消息传递给目标应用程序之前进行拦截。钩子运行在独立线程中，通过消息循环处理键盘事件。

### macOS

使用 `CGEventTapCreate` 创建事件拦截器（Event Tap），在会话级别监控和过滤键盘事件。事件处理运行在独立的 CFRunLoop 线程中。

**注意：** macOS 上需要授予辅助功能权限才能拦截键盘事件。首次运行时系统会提示用户授权。

## 预构建二进制文件

本库使用基于 [Node-API](https://nodejs.org/api/n-api.html) 的预构建二进制文件，一个文件兼容所有 Node.js 版本及 Electron，无需为不同运行时单独构建。

**处理优先级：**

1. 安装时运行 `prebuild-install` 从 GitHub Releases 下载（npm / yarn / pnpm + onlyBuiltDependencies）
2. 若安装脚本未执行，首次 `require()` 时自动从 GitHub Releases 下载
3. 下载失败则抛出明确的错误提示

## 在 Electron 中使用

本库基于 Node-API（NAPI），预编译文件天然兼容所有 Electron 版本，**只能在主进程中使用**，渲染进程可通过 IPC 调用主进程。

### pnpm + electron-builder 配置

使用 pnpm 的 Electron 项目，在 `package.json` 中添加：

```json
{
  "pnpm": {
    "onlyBuiltDependencies": ["zt-block-system-shortcuts"]
  }
}
```

这样 pnpm 安装时会执行 install 脚本，提前下载好预编译文件。

## 从源码构建

如需从源码编译，需提前安装编译工具：

- **Windows**：[Visual Studio 2019+](https://visualstudio.microsoft.com/)（含 C++ 工作负载）和 Python 3.x
- **macOS**：Xcode Command Line Tools（`xcode-select --install`）

```bash
git clone https://github.com/lzt-T/zt-block-system-shortcuts.git
cd zt-block-system-shortcuts
pnpm install --ignore-scripts
pnpm run compile
```

## 维护者发布流程

### 1. 更新版本号

```bash
# 修改 package.json 中的 version 字段，例如改为 1.0.1
```

### 2. 提交并推送 Tag

```bash
git add .
git commit -m "chore: release v1.0.1"
git tag v1.0.1
git push origin master --tags
```

### 3. 自动构建

推送 Tag 后，GitHub Actions 会自动：
- 在 Windows x64 和 macOS ARM64 上构建预编译文件
- 创建 GitHub Release 并上传 `.tar.gz` 文件

| 环境 | 产物 |
|------|------|
| Windows x64 | `zt-block-system-shortcuts-v{version}-napi-v6-win32-x64.tar.gz` |
| macOS ARM64 | `zt-block-system-shortcuts-v{version}-napi-v6-darwin-arm64.tar.gz` |

### 4. 发布到 npm

```bash
pnpm publish
```

## 常见问题

**Q：安装后运行报错 `Could not locate the bindings file`？**

A：预编译文件下载失败。检查网络是否能访问 GitHub，或参考"从源码构建"安装编译工具后重新安装。

**Q：pnpm 安装时提示 `Ignored build scripts`？**

A：这是 pnpm v10 的安全机制。在项目 `package.json` 中添加 `onlyBuiltDependencies` 配置（见上方"pnpm 用户注意"），重新安装后即可在安装阶段完成下载。

**Q：macOS 上无法拦截键盘事件？**

A：需要在"系统偏好设置 > 安全性与隐私 > 隐私 > 辅助功能"中授权你的应用（或终端）。

**Q：Windows 上拦截不生效？**

A：确保应用有足够权限。某些特殊系统快捷键（如 Ctrl+Alt+Delete）无法被用户程序拦截，这是 Windows 的安全设计。

**Q：在 Electron 项目中 `electron-builder install-app-deps` 报错？**

A：本库发布包中不含 `binding.gyp`，`@electron/rebuild` 不会将其识别为需要重编译的模块。确保在 `package.json` 中添加 `onlyBuiltDependencies` 配置。

## License

[MIT](./LICENSE) © lzt-T
