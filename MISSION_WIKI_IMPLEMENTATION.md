# Mission Wiki In-Game Viewer - Implementation Summary

## What Was Added

A new in-game window that fetches and displays Guild Wars wiki content for the current mission, eliminating the need to alt-tab to a browser. Perfect for controller users!

## Files Created

1. **GWToolboxdll/Windows/MissionWikiWindow.h** - Window header
2. **GWToolboxdll/Windows/MissionWikiWindow.cpp** - Window implementation

## Files Modified

1. **GWToolboxdll/Modules/ChatCommands.h** - Added `CmdMissionWiki` declaration
2. **GWToolboxdll/Modules/ChatCommands.cpp** - Registered `/missionwiki` command and implementation
3. **GWToolboxdll/Modules/ToolboxSettings.cpp** - Registered window in optional windows list

## How to Build

The files are already set up in the build system:
- CMakeLists.txt automatically includes `Windows/*.cpp` files
- Window is registered in ToolboxSettings.cpp
- Command is registered in ChatCommands.cpp

Just run your normal build:
```bash
cmake --build build --config RelWithDebInfo
```

## How It Works

1. User types `/missionwiki` in chat
2. Window fetches current map name from game
3. Downloads wiki page from https://wiki.guildwars.com
4. Parses HTML to extract:
   - Map image
   - Primary objectives
   - Bonus objectives
   - Walkthrough strategy
   - Skill recommendations
   - Tips and notes
5. Displays formatted, color-coded text in-game window

## Usage

```
/missionwiki
```

Opens the Mission Wiki window and automatically fetches info for your current map.

## Features

- **No Alt-Tabbing**: Content displayed directly in-game
- **Controller Friendly**: Text-based display with color-coded sections
- **Map Image**: Shows mission layout for navigation
- **Complete Info**: Objectives, bonus, strategy, skills, tips
- **Async Loading**: Doesn't freeze game while fetching
- **HTML Parsing**: Extracts and formats wiki content
- **Reusable**: Click "Fetch Current Mission" button to refresh

## Display Example (Riverside Province)

```
Mission Title (gold)
[Map Image]

Objectives: (green)
• Steal the Scepter of Orr
• Avoid detection...

Bonus: (gold)
• Take out the Watchman at each tower...

Strategy: (light blue)
This mission can be done without fighting...

Recommended Skills: (pink)
• Area of effect spells...
• Anti-caster skills...

Tips: (light yellow)
• You can drop the Scepter...
• White Mantle and undead fight each other...
```

## Technical Details

- Uses existing `RestClient` infrastructure for HTTP requests
- Follows same pattern as `TargetInfoWindow` for wiki fetching
- Uses `ctre` regex library for HTML parsing
- Runs parsing on worker thread to avoid blocking UI
- Color-coded sections using ImGui's native text coloring
- Supports all font sizes from FontLoader

