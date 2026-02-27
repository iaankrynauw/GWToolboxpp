# GWToolbox++ Quick Reference

**Quick lookup for common development tasks**

---

## Project Structure

```
GWToolboxpp/
├── GWToolbox/          # Launcher (exe)
├── GWToolboxdll/       # Main DLL
│   ├── Windows/        # UI with borders
│   ├── Widgets/        # Borderless overlays
│   ├── Modules/        # Background functionality
│   └── Utils/          # Helper functions
├── Dependencies/       # Third-party libs
│   └── GWCA/          # Game API
└── notes/             # Development docs (gitignored)
```

---

## Build Commands

```bash
# First time setup
cmake --preset=vcpkg

# Build
cmake --build build --config RelWithDebInfo

# Debug build
cmake --build build --config Debug

# Open in Visual Studio
cmake --open build
```

---

## Component Types

| Type | Base Class | Location | Has UI | Has Border | Example |
|------|-----------|----------|--------|------------|---------|
| Module | ToolboxModule | Modules/ | ❌ | N/A | ChatFilter |
| Window | ToolboxWindow | Windows/ | ✅ | ✅ | BuildsWindow |
| Widget | ToolboxWidget | Widgets/ | ✅ | ❌ | ClockWidget |

---

## Lifecycle Methods

```cpp
class MyComponent : public ToolboxWindow {
    // 1. Construction (singleton)
    static MyComponent& Instance();
    
    // 2. Setup
    void Initialize() override;
    
    // 3. Every frame
    void Update(float delta) override;
    
    // 4. Render (UI only)
    void Draw(IDirect3DDevice9*) override;
    
    // 5. Settings UI
    void DrawSettingsInternal() override;
    
    // 6. Cleanup (uses GWCA/modules)
    void SignalTerminate() override;
    
    // 7. Cleanup (revert game changes)
    void Terminate() override;
    
    // 8. Destructor (class scope only)
    ~MyComponent() override;
};
```

---

## Registration Checklist

### New Window

1. Create `Windows/MyWindow.h` and `.cpp`
2. Add to `ToolboxSettings.cpp`:
   ```cpp
   #include <Windows/MyWindow.h>
   // In optional_windows array:
   MyWindow::Instance(),
   ```
3. Build (auto-included via CMake glob)

### New Chat Command

1. Declare in `ChatCommands.h`:
   ```cpp
   static void CHAT_CMD_FUNC(CmdMyCommand);
   ```
2. Register in `ChatCommands.cpp`:
   ```cpp
   {L"mycommand", CmdMyCommand},
   ```
3. Implement:
   ```cpp
   void CHAT_CMD_FUNC(ChatCommands::CmdMyCommand) {
       // Handle command
   }
   ```

---

## Common Patterns

### Singleton

```cpp
class MyWindow : public ToolboxWindow {
    MyWindow() = default;
    ~MyWindow() override = default;
public:
    static MyWindow& Instance() {
        static MyWindow instance;
        return instance;
    }
};
```

### Basic Window

```cpp
void MyWindow::Draw(IDirect3DDevice9*) {
    if (!visible) return;
    
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
        ImGui::Text("Content");
    }
    ImGui::End();
}
```

### Settings

```cpp
void MyWindow::DrawSettingsInternal() {
    ToolboxWindow::DrawSettingsInternal();
    ImGui::Checkbox("Option", &my_option);
}

void MyWindow::LoadSettings(ToolboxIni* ini) {
    ToolboxWindow::LoadSettings(ini);
    LOAD_BOOL(my_option);
}

void MyWindow::SaveSettings(ToolboxIni* ini) {
    ToolboxWindow::SaveSettings(ini);
    SAVE_BOOL(my_option);
}
```

### Async Download

```cpp
void MyWindow::FetchData() {
    Resources::Download(url, OnDataReceived, this);
}

void MyWindow::OnDataReceived(bool success, const std::string& data, void* ctx) {
    auto* window = static_cast<MyWindow*>(ctx);
    if (success) {
        Resources::EnqueueWorkerTask([window, data] {
            window->ProcessData(data);
        });
    }
}
```

### GWCA Usage

```cpp
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Constants/Constants.h>

// Get map info
auto map_id = GW::Map::GetMapID();
auto map_info = GW::Map::GetCurrentMapInfo();
if (!map_info) return;

// Get map name
GuiUtils::EncString name;
name.language(GW::Constants::Language::English);
name.reset(map_info->name_id);
std::string map_name = name.string();
```

---

## ImGui Cheat Sheet

### Basic Widgets

```cpp
ImGui::Text("Label: %s", text.c_str());
ImGui::TextColored(ImVec4(1,0,0,1), "Red text");
ImGui::BulletText("Bullet point");
ImGui::Separator();
ImGui::Spacing();

if (ImGui::Button("Click")) { }
ImGui::Checkbox("Enable", &enabled);
ImGui::SliderFloat("Value", &value, 0.0f, 100.0f);
ImGui::InputText("Text", buffer, sizeof(buffer));
```

### Layout

```cpp
ImGui::SameLine();  // Next widget on same line
ImGui::Indent();    // Indent following widgets
ImGui::Unindent();  // Restore indent

ImGui::BeginChild("region", ImVec2(200, 300));
// Scrollable region
ImGui::EndChild();

ImGui::Columns(2);  // Split into columns
ImGui::NextColumn();
```

### Fonts & Colors

```cpp
ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::header2));
ImGui::Text("Large text");
ImGui::PopFont();

ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,0,1));
ImGui::Text("Yellow");
ImGui::PopStyleColor();
```

---

## GWCA Quick Reference

### Managers

```cpp
#include <GWCA/Managers/MapMgr.h>
GW::Map::GetMapID()
GW::Map::GetInstanceType()
GW::Map::GetCurrentMapInfo()

#include <GWCA/Managers/AgentMgr.h>
GW::Agents::GetPlayer()
GW::Agents::GetTarget()
GW::Agents::GetAgentByID(id)

#include <GWCA/Managers/PartyMgr.h>
GW::PartyMgr::GetPartyInfo()
GW::PartyMgr::GetIsPartyInHardMode()

#include <GWCA/Managers/ChatMgr.h>
GW::Chat::SendChat(message)
GW::Chat::WriteChat(channel, message)
```

### Constants

```cpp
#include <GWCA/Constants/Constants.h>

GW::Constants::MapID::Riverside_Province
GW::Constants::InstanceType::Explorable
GW::Constants::Language::English
```

---

## Debugging

### Attach Debugger

1. Launch GWToolbox normally
2. Visual Studio → Debug → Attach to Process
3. Select `Gw.exe`
4. Set breakpoints
5. Trigger code in-game

### Logging

```cpp
#include <Logger.h>

Log::Info("Message: %s", text.c_str());
Log::Warning("Warning: %d", value);
Log::Error("Error occurred");
```

### Common Issues

| Problem | Solution |
|---------|----------|
| Component not appearing | Check ToolboxSettings.cpp registration |
| Crash on map change | Null-check GWCA pointers |
| UI freezing | Move heavy work to worker thread |
| Settings not saving | Implement LoadSettings/SaveSettings |
| Window not drawing | Check `visible` flag in Draw() |

---

## File Templates

### Window Header

```cpp
#pragma once
#include <ToolboxWindow.h>

class MyWindow : public ToolboxWindow {
    MyWindow() = default;
    ~MyWindow() override = default;
    
public:
    static MyWindow& Instance();
    
    const char* Name() const override { return "My Window"; }
    const char* Icon() const override { return ICON_FA_WINDOW; }
    
    void Initialize() override;
    void Draw(IDirect3DDevice9*) override;
    void DrawSettingsInternal() override;
    
private:
    bool my_setting = false;
};
```

### Window Implementation

```cpp
#include "stdafx.h"
#include "MyWindow.h"

MyWindow& MyWindow::Instance() {
    static MyWindow instance;
    return instance;
}

void MyWindow::Initialize() {
    ToolboxWindow::Initialize();
}

void MyWindow::Draw(IDirect3DDevice9*) {
    if (!visible) return;
    
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
        ImGui::Text("Hello World");
    }
    ImGui::End();
}

void MyWindow::DrawSettingsInternal() {
    ToolboxWindow::DrawSettingsInternal();
    ImGui::Checkbox("My Setting", &my_setting);
}
```

---

## Resources

- **Architecture:** See `notes/CODEBASE_ARCHITECTURE.md`
- **Example:** See `notes/MISSION_WIKI_IMPLEMENTATION.md`
- **Website:** https://www.gwtoolbox.com
- **GitHub:** https://github.com/gwdevhub/GWToolboxpp
- **Discord:** https://discord.gg/pGS5pFn

---

## Color Codes (ImVec4)

```cpp
// Format: ImVec4(R, G, B, A) - values 0.0 to 1.0

ImVec4(1.0f, 0.0f, 0.0f, 1.0f)  // Red
ImVec4(0.0f, 1.0f, 0.0f, 1.0f)  // Green
ImVec4(0.0f, 0.0f, 1.0f, 1.0f)  // Blue
ImVec4(1.0f, 1.0f, 0.0f, 1.0f)  // Yellow
ImVec4(1.0f, 0.0f, 1.0f, 1.0f)  // Magenta
ImVec4(0.0f, 1.0f, 1.0f, 1.0f)  // Cyan
ImVec4(1.0f, 0.8f, 0.2f, 1.0f)  // Gold
ImVec4(0.5f, 0.5f, 0.5f, 1.0f)  // Gray
```

---

## Font Awesome Icons

```cpp
#include <Defines.h>  // Contains ICON_FA_* constants

ICON_FA_WINDOW          // Window
ICON_FA_COG             // Settings gear
ICON_FA_BOOK_OPEN       // Open book
ICON_FA_CLIPBOARD       // Clipboard
ICON_FA_STAR            // Star
ICON_FA_MAP             // Map
ICON_FA_USERS           // Users/party
ICON_FA_SHIELD          // Shield
ICON_FA_SWORD           // Sword
```

---

**Last Updated:** 2026-02-26
