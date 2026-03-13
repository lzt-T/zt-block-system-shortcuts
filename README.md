# zt-block-system-shortcuts

Cross-platform keyboard shortcut blocking library for Node.js. Block system shortcuts like Win key, Alt+Tab, Ctrl, F1-F12, and more on Windows and macOS.

## Features

- Block Windows key (Win) / Command key (macOS)
- Block Alt+Tab application switching
- Block Alt key / Option key (macOS)
- Block Ctrl key / Control key (macOS)
- Block F1-F12 function keys
- Block F3 (Mission Control on macOS)
- Block Fn key combinations
- Prebuilt binaries for Windows x64 and macOS arm64
- Automatic binary download on npm install

## Installation

```bash
npm install zt-block-system-shortcuts
# or
pnpm add zt-block-system-shortcuts
```

## Usage

```javascript
const { disableAll, enableAll, KeyManager } = require('zt-block-system-shortcuts');

// Using exported functions
disableAll();  // Block all system shortcuts
// ... your locked-down application logic ...
enableAll();   // Re-enable all shortcuts

// Using KeyManager class
const km = new KeyManager();
km.disableAll();
// ... your application logic ...
km.enableAll();
```

## API

### `disableAll(): boolean`

Disables all system shortcuts. Returns `true` on success.

### `enableAll(): boolean`

Re-enables all system shortcuts. Returns `true` on success.

### `class KeyManager`

A class wrapper for managing keyboard state.

- `disableAll(): boolean` - Disable all shortcuts
- `enableAll(): boolean` - Enable all shortcuts

## Platform Support

| Platform | Architecture | Status |
|----------|-------------|--------|
| Windows  | x64         | ✅      |
| Windows  | arm64       | ✅      |
| macOS    | x64         | ✅      |
| macOS    | arm64       | ✅      |

## How It Works

### Windows

Uses `SetWindowsHookEx` with `WH_KEYBOARD_LL` to install a low-level keyboard hook that intercepts key events before they reach other applications.

### macOS

Uses `CGEventTapCreate` to create an event tap that monitors and filters keyboard events at the session level.

**Note:** On macOS, the application needs Accessibility permissions to intercept keyboard events. Users will be prompted to grant these permissions.

## Use Cases

- Exam/testing software that needs to prevent users from switching applications
- Kiosk applications
- Full-screen presentation software
- Gaming applications that need exclusive keyboard access

## Building from Source

```bash
# Install dependencies
pnpm install --ignore-scripts

# Compile native addon
pnpm run compile

# Or build prebuilt binaries
pnpm run prebuild
```

## Requirements

- Node.js >= 14.0.0
- Windows: Visual Studio Build Tools
- macOS: Xcode Command Line Tools

## License

MIT
