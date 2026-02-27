# GWToolbox++ Codebase Architecture

**Last Updated:** 2026-02-26  
**Project:** GWToolbox++ - Tools for Guild Wars Players  
**Repository:** https://github.com/gwdevhub/GWToolboxpp

## Table of Contents

1. [Project Overview](#project-overview)
2. [Directory Structure](#directory-structure)
3. [Core Architecture](#core-architecture)
4. [Component Types](#component-types)
5. [Build System](#build-system)
6. [Key Dependencies](#key-dependencies)
7. [Development Workflow](#development-workflow)

---

## Project Overview

GWToolbox++ is a collection of tools and enhancements for Guild Wars (2005). It consists of:

- **GWToolbox.exe** - Launcher/injector that selects a Guild Wars client and injects the DLL
- **GWToolboxdll.dll** - Main toolbox functionality injected into the game process
- **Plugins** - Optional extensions that can be loaded at runtime

### Technology Stack

- **Language:** C++23
- **Compiler:** MSVC v143 Platform Toolset (Visual Studio 2022 17.10+)
- **Build System:** CMake 3.29+
- **Package Manager:** vcpkg
- **UI Framework:** Dear ImGui with DirectX 9
- **Game API:** GWCA (Guild Wars Client API)

---

## Directory Structure

```
GWToolboxpp/
├── GWToolbox/              # Launcher/injector executable
│   ├── main.cpp            # Entry point
│   ├── Inject.cpp          # DLL injection logic
│   ├── Download.cpp        # Update/download functionality
│   └── Settings.cpp        # Launcher settings
│
├── GWToolboxdll/           # Main DLL (injected into game)
│   ├── Windows/            # UI Windows (with borders/headers)
│   ├── Widgets/            # UI Widgets (borderless overlays)
│   ├── Modules/            # Non-UI functionality modules
│   ├── Utils/              # Utility functions and helpers
│   ├── Constants/          # Game constants and data
│   ├── GWToolbox.cpp       # Main DLL entry point
│   └── CMakeLists.txt      # Build configuration
│
├── Dependencies/           # Third-party libraries
│   ├── GWCA/               # Guild Wars Client API
│   ├── easywsclient/       # WebSocket client
│   ├── wintoast/           # Windows notifications
│   ├── nativefiledialog/   # File dialogs
│   └── steamworks_sdk/     # Steam integration
│
├── RestClient/             # HTTP client library (curl wrapper)
├── Core/                   # Core utilities (threading, paths, etc.)
├── plugins/                # Plugin examples and base classes
├── resources/              # Fonts, icons, config files
├── docs/                   # User documentation (Jekyll site)
├── cmake/                  # CMake helper scripts
└── vcpkg-overlays/         # Custom vcpkg ports
```

---

## Core Architecture

### Inheritance Hierarchy

```
ToolboxModule (base class)
    ├── ToolboxUIElement (adds UI capabilities)
    │   ├── ToolboxWindow (windows with borders/headers)
    │   └── ToolboxWidget (borderless overlays)
    └── Pure Modules (no UI, settings panel only)
```

### Component Lifecycle

All components follow this lifecycle:

1. **Construction** - Singleton pattern via `Instance()` static method
2. **Initialize()** - Setup, allocate resources, register hooks
3. **Update(float delta)** - Called every frame (delta in seconds)
4. **Draw(IDirect3DDevice9*)** - Render UI (UIElements only)
5. **SignalTerminate()** - Cleanup that requires other modules/GWCA
6. **Terminate()** - Revert game changes, unhook
7. **Destructor** - Final cleanup (class scope only)

**CRITICAL:** Destruction order is `SignalTerminate → Terminate → Destructor`. Handle all GWCA/module interactions in `SignalTerminate`, game state reversion in `Terminate`, and only class-scope cleanup in destructor.

---

## Component Types

### 1. Modules (No UI)

**Location:** `GWToolboxdll/Modules/`  
**Base Class:** `ToolboxModule`  
**Purpose:** Background functionality without visual interface

**Key Characteristics:**
- No `Draw()` method
- Have settings panel via `DrawSettingsInternal()`
- Examples: ChatFilter, GameSettings, Resources, InventoryManager

**Example Structure:**
```cpp
class MyModule : public ToolboxModule {
public:
    static MyModule& Instance() {
        static MyModule instance;
        return instance;
    }
    
    const char* Name() const override { return "My Module"; }
    const char* Icon() const override { return ICON_FA_COG; }
    
    void Initialize() override;
    void Update(float delta) override;
    void DrawSettingsInternal() override;
    void LoadSettings(ToolboxIni* ini) override;
    void SaveSettings(ToolboxIni* ini) override;
};
```

### 2. Windows (UI with Borders)

**Location:** `GWToolboxdll/Windows/`  
**Base Class:** `ToolboxWindow`  
**Purpose:** Full-featured windows with title bar, borders, close button

**Key Characteristics:**
- Has title bar and close button
- Resizable and movable by default
- Can be toggled from main window
- Examples: BuildsWindow, TravelWindow, MaterialsWindow

**Example Structure:**
```cpp
class MyWindow : public ToolboxWindow {
    MyWindow() = default;
    ~MyWindow() override = default;
    
public:
    static MyWindow& Instance() {
        static MyWindow instance;
        return instance;
    }
    
    const char* Name() const override { return "My Window"; }
    const char* Icon() const override { return ICON_FA_WINDOW; }
    
    void Initialize() override;
    void Draw(IDirect3DDevice9* device) override;
    void DrawSettingsInternal() override;
    
private:
    // Window state variables
};
```

### 3. Widgets (Borderless Overlays)

**Location:** `GWToolboxdll/Widgets/`  
**Base Class:** `ToolboxWidget`  
**Purpose:** Lightweight overlays without window chrome

**Key Characteristics:**
- No title bar or borders by default
- `lock_move` and `lock_size` enabled by default
- Minimal visual footprint
- Examples: ClockWidget, HealthWidget, BondsWidget

**Example Structure:**
```cpp
class MyWidget : public ToolboxWidget {
    MyWidget() = default;
    ~MyWidget() override = default;
    
public:
    static MyWidget& Instance() {
        static MyWidget instance;
        return instance;
    }
    
    const char* Name() const override { return "My Widget"; }
    const char* Icon() const override { return ICON_FA_STAR; }
    
    void Initialize() override;
    void Draw(IDirect3DDevice9* device) override;
    void DrawSettingsInternal() override;
};
```

---

## Build System

### CMake Configuration

The project uses CMake with vcpkg integration:

```bash
# Configure (first time)
cmake --preset=vcpkg

# Build
cmake --build build --config RelWithDebInfo

# Open in IDE
cmake --open build
```

### File Discovery

**Automatic Inclusion:** CMakeLists.txt uses `file(GLOB SOURCES ...)` patterns:
- `"Windows/*.cpp"` - All window implementations
- `"Widgets/*.cpp"` - All widget implementations  
- `"Modules/*.cpp"` - All module implementations
- `"Utils/*.cpp"` - All utility files

**No manual registration needed** - Just create `.h` and `.cpp` files in the appropriate directory.

### Registration Points

New components must be registered in **one place**:

**File:** `GWToolboxdll/Modules/ToolboxSettings.cpp`

```cpp
// Around line 200-225
static const std::vector<OptionalModule> optional_windows = {
    // ... existing windows ...
    MyNewWindow::Instance(),  // Add here
};
```

For modules without UI:
```cpp
static const std::vector<OptionalModule> optional_modules = {
    // ... existing modules ...
    MyNewModule::Instance(),  // Add here
};
```

For widgets:
```cpp
static const std::vector<OptionalModule> optional_widgets = {
    // ... existing widgets ...
    MyNewWidget::Instance(),  // Add here
};
```

---

## Key Dependencies

### GWCA (Guild Wars Client API)

**Location:** `Dependencies/GWCA/`  
**Purpose:** Interface with Guild Wars game client

**Key Namespaces:**
- `GW::Map` - Map/instance information
- `GW::Agents` - Agent (NPC/player) data
- `GW::Items` - Inventory and item management
- `GW::Chat` - Chat system interaction
- `GW::PartyMgr` - Party management
- `GW::SkillbarMgr` - Skillbar access

**Common Patterns:**
```cpp
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Constants/Constants.h>

auto map_id = GW::Map::GetMapID();
auto map_info = GW::Map::GetCurrentMapInfo();
```

### ImGui (UI Framework)

**Integration:** Custom DirectX 9 implementation  
**Files:** `imgui_impl_dx9.cpp`, `imgui_impl_win32.cpp`

**Common Patterns:**
```cpp
void MyWindow::Draw(IDirect3DDevice9*) {
    if (!visible) return;
    
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
        ImGui::Text("Hello World");
        if (ImGui::Button("Click Me")) {
            // Handle click
        }
    }
    ImGui::End();
}
```

### Resources Module

**File:** `GWToolboxdll/Modules/Resources.h`  
**Purpose:** Async HTTP downloads, image loading, worker threads

**Common Patterns:**
```cpp
// Download web content
Resources::Download(url, callback, context);

// Load image from URL
auto texture = Resources::GetGuildWarsWikiImage(url, 0, false);

// Run task on worker thread
Resources::EnqueueWorkerTask([this] {
    // Heavy processing here
});
```

### GuiUtils

**File:** `GWToolboxdll/Utils/GuiUtils.h`  
**Purpose:** Common UI helpers and utilities

**Common Functions:**
```cpp
// Get wiki URL for item/skill/location
std::string wiki_url = GuiUtils::WikiUrl(name);

// Encoded string handling
GuiUtils::EncString enc_str;
enc_str.language(GW::Constants::Language::English);
enc_str.reset(string_id);
std::string text = enc_str.string();
```

---

## Development Workflow

### Adding a New Window

1. **Create header file:** `GWToolboxdll/Windows/MyWindow.h`
   ```cpp
   #pragma once
   #include <ToolboxWindow.h>
   
   class MyWindow : public ToolboxWindow {
       MyWindow() = default;
       ~MyWindow() override = default;
   public:
       static MyWindow& Instance();
       const char* Name() const override;
       const char* Icon() const override;
       void Draw(IDirect3DDevice9*) override;
   };
   ```

2. **Create implementation:** `GWToolboxdll/Windows/MyWindow.cpp`
   ```cpp
   #include "stdafx.h"
   #include "MyWindow.h"
   
   MyWindow& MyWindow::Instance() {
       static MyWindow instance;
       return instance;
   }
   
   const char* MyWindow::Name() const { return "My Window"; }
   const char* MyWindow::Icon() const { return ICON_FA_WINDOW; }
   
   void MyWindow::Draw(IDirect3DDevice9*) {
       if (!visible) return;
       if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
           ImGui::Text("Content here");
       }
       ImGui::End();
   }
   ```

3. **Register in ToolboxSettings.cpp:**
   ```cpp
   #include <Windows/MyWindow.h>
   
   // In optional_windows array:
   MyWindow::Instance(),
   ```

4. **Build:**
   ```bash
   cmake --build build --config RelWithDebInfo
   ```

### Adding a Chat Command

**File:** `GWToolboxdll/Modules/ChatCommands.cpp`

1. **Declare in header** (`ChatCommands.h`):
   ```cpp
   static void CHAT_CMD_FUNC(CmdMyCommand);
   ```

2. **Register command** (in `ChatCommands.cpp` constructor):
   ```cpp
   {L"mycommand", CmdMyCommand},
   ```

3. **Implement handler:**
   ```cpp
   void CHAT_CMD_FUNC(ChatCommands::CmdMyCommand) {
       // argc = argument count
       // argv = argument array
       MyWindow::Instance().visible = true;
   }
   ```

### Settings Panel Pattern

```cpp
void MyWindow::DrawSettingsInternal() {
    ToolboxWindow::DrawSettingsInternal();
    
    ImGui::Checkbox("Enable feature", &enable_feature);
    ImGui::SliderFloat("Value", &my_value, 0.0f, 100.0f);
    
    if (ImGui::Button("Reset")) {
        // Reset to defaults
    }
}

void MyWindow::LoadSettings(ToolboxIni* ini) {
    ToolboxWindow::LoadSettings(ini);
    LOAD_BOOL(enable_feature);
    LOAD_FLOAT(my_value);
}

void MyWindow::SaveSettings(ToolboxIni* ini) {
    ToolboxWindow::SaveSettings(ini);
    SAVE_BOOL(enable_feature);
    SAVE_FLOAT(my_value);
}
```

### Async Operations Pattern

```cpp
void MyWindow::FetchData() {
    if (is_fetching) return;
    is_fetching = true;
    
    Resources::Download(url, OnDataFetched, this);
}

void MyWindow::OnDataFetched(bool success, const std::string& response, void* ctx) {
    auto* window = static_cast<MyWindow*>(ctx);
    window->is_fetching = false;
    
    if (success) {
        // Parse on worker thread to avoid blocking UI
        Resources::EnqueueWorkerTask([window, data = response] {
            window->ParseData(data);
        });
    }
}
```

---

## Code Style Guidelines

### Naming Conventions

- **Classes:** PascalCase (`MyWindow`, `ChatCommands`)
- **Functions:** PascalCase (`Initialize()`, `DrawSettings()`)
- **Variables:** snake_case (`is_fetching`, `map_texture`)
- **Constants:** snake_case with CAPS for macros (`max_value`, `ICON_FA_STAR`)
- **Private members:** snake_case, no prefix (`data`, not `m_data`)

### File Organization

- **Header guards:** `#pragma once` (not `#ifndef`)
- **Includes:** `#include "stdafx.h"` first in `.cpp` files
- **Singleton pattern:** Always use `Instance()` static method
- **Constructor/Destructor:** Private or default, in class definition

### ImGui Patterns

```cpp
// Always check visibility
if (!visible) return;

// Set initial size on first use
ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

// Use GetVisiblePtr() for close button
if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
    // Content
}
ImGui::End();
```

### GWCA Safety

```cpp
// Always null-check GWCA pointers
auto* agent = GW::Agents::GetPlayer();
if (!agent) return;

// Check game state before operations
if (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Explorable) {
    return;
}
```

---

## Testing and Debugging

### Visual Studio Debugger

1. Launch GWToolbox normally (injects DLL)
2. In Visual Studio: Debug → Attach to Process
3. Select `Gw.exe` process
4. Set breakpoints in toolbox code
5. Trigger the code path in-game

### Debug Builds

```bash
cmake --build build --config Debug
```

Debug builds include:
- Full symbols for debugging
- Additional logging
- Packet logger window (auto-enabled)
- String decoder window (auto-enabled)

### Logging

```cpp
#include <Logger.h>

Log::Info("Message: %s", text.c_str());
Log::Warning("Warning: %d", value);
Log::Error("Error occurred");
```

---

## Common Pitfalls

1. **Forgetting to register in ToolboxSettings.cpp** - Component won't appear
2. **Not checking `visible` in `Draw()`** - Wastes performance
3. **Heavy processing in `Draw()`** - Use worker threads via `Resources::EnqueueWorkerTask()`
4. **Not null-checking GWCA pointers** - Crashes when game state changes
5. **Using destructor for GWCA cleanup** - Use `SignalTerminate()` instead
6. **Blocking operations in UI thread** - Use async patterns with callbacks

---

## Resources

- **Main Site:** https://www.gwtoolbox.com
- **GitHub:** https://github.com/gwdevhub/GWToolboxpp
- **Discord:** https://discord.gg/pGS5pFn
- **Issue Tracker:** https://github.com/gwdevhub/GWToolboxpp/issues
- **GWCA Documentation:** See `Dependencies/GWCA/include/GWCA/`
- **ImGui Documentation:** https://github.com/ocornut/imgui

---

**Next:** See [MISSION_WIKI_IMPLEMENTATION.md](MISSION_WIKI_IMPLEMENTATION.md) for a complete example of adding a new window.
