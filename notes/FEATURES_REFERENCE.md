# GWToolbox++ Features Reference

**Complete feature documentation from gwtoolbox.com**  
**Last Updated:** 2026-02-26

This is a local copy of all feature documentation for offline reference.

---

## Table of Contents

- [Windows](#windows)
- [Widgets](#widgets)
- [Game Settings](#game-settings)
- [Other Features](#other-features)

---

## Windows

Windows are designed to be opened when needed and hidden when not in use. All windows have borders, title bars, and can be moved/resized.

**Available Windows:**
- Hotkeys - Keyboard shortcuts for Toolbox functions
- Travel - Quick travel to any outpost
- Builds - Teambuild management and loading
- Pcons - Automatic consumable usage
- Materials - Auto-buy/sell from traders
- Info - Game information display
- Dialogs - Quick dialog sending
- Notepad - In-game text editor
- Mission Wiki - In-game mission guide viewer
- Settings - Toolbox configuration

### General Interface Controls

- **Double click** title bar to collapse window
- **Click and drag** lower right corner to resize
- **Click and drag** empty space to move
- **Mouse wheel** to scroll
- **TAB/SHIFT+TAB** to cycle through fields
- **Ctrl+Click** on slider to input text
- **Double click** drag box to input text

### Opening/Closing Windows

- Buttons in main window
- Checkboxes in Settings headers
- `/hide (name)` - Hide window
- `/show (name)` - Show window
- `/tb (name)` - Toggle window
- Close button (top right)

---

## Hotkeys Window

Map keyboard/mouse keys to Toolbox functions.

### Creating Hotkeys

1. Click "Create Hotkey…"
2. Select hotkey type
3. Configure settings:
   - Toggle on/off
   - Display message when triggered
   - Block key in Guild Wars
   - Trigger on entering explorable/outpost
   - Trigger on focus gain/loss
   - Map/profession/instance restrictions
   - Character-specific binding

### Hotkey Types

**Send Chat**
- Select channel and message
- Supports GW commands (`/resign`, `/stuck`)
- Supports Toolbox commands (`/pcons`, `/cam unlock`)

**Use Item**
- Specify Model ID and optional name
- Works with consumables in inventory/storage

**Drop or Use Buff**
- Dual function: dismiss if active, cast if not
- Options: Recall, Unyielding Aura, Holy Veil, custom skill ID

**Toggle**
- Clicker - Rapid left-click
- Pcons - Auto pcons on/off
- Coin Drop - Periodic gold coin drop
- Tick - Ready tick in party window

**Execute**
- Open Xunlai Chest (from anywhere in outpost)
- Open Locked Chest (auto lockpick)
- Drop Gold Coin
- Reapply appropriate title
- Enter Challenge

**Target**
- Auto-target NPC/signpost/item by ID

**Move to**
- Move character to map coordinates
- Specify range for activation

**Dialog**
- Send specific dialog ID
- Auto-interact with NPCs/quests

**Ping Build**
- Send teambuild to party chat

**Load Hero Team Build**
- Load saved hero team

**Equip Item**
- Equip by attributes or inventory position

**Flag Hero**
- Set hero flag at angle/distance

**Guild Wars Key**
- Bind to GW control action

**Command Pet**
- Set pet behavior (Fight/Guard/Avoid)

---

## Travel Window

Quick travel to any unlocked outpost.

### Features

- Outpost dropdown selection
- Quick travel buttons
- District selection
- Favorites system
- Auto-retry full districts
- Close/collapse after travel
- Special handling for elite areas

### Chat Commands

`/tp`, `/to`, `/travel [dest] [district]`

**Shortcuts:**
- `gh` - Guild hall
- `gh [tag]` - Alliance guild hall
- `toa` - Temple of the Ages
- `doa`, `goa`, `tdp` - Domain of Anguish
- `eee` - Embark Beach Europe English
- `gtob` - Great Temple of Balthazar
- `la` - Lions Arch
- `ac` - Ascalon City
- `eotn` - Eye of the North
- `hzh` - House Zu Heltzer
- `ctc` - Central Transfer Chamber
- `topk` - Tomb of Primeval Kings
- `ra` - Random Arenas
- `ha` - Heroes' Ascent
- `fa` - Fort Aspenwood
- `jq` - Jade Quarry

**District Shortcuts:**
- `ae`, `ae1` - American
- `int` - International
- `ee` - Europe English
- `eg`, `dd` - Europe German
- `ef`, `fr` - Europe French
- `ei`, `it` - Europe Italian
- `es` - Europe Spanish
- `ep`, `pl` - Europe Polish
- `er`, `ru` - Europe Russian
- `ak`, `kr` - Asia Korean
- `ac`, `atc`, `ch` - Asia Chinese
- `aj`, `jp` - Asia Japanese

**Examples:**
- `/tp kama ae1` - Kamadan American 1
- `/tp fa l` - Fort Aspenwood Luxon
- `/tp deep ee` - The Deep Europe English

---

## Builds Window

Create and manage teambuilds.

### Features

- Send builds to party chat
- Load builds onto character
- Pcons integration per build
- View/send individual builds
- Show numbers option
- Blank template for instructions
- Preferred skill orders
- Teambuild management (up/down/delete)

### Settings

- Hide when entering explorable
- Only show one teambuild at a time
- Auto load pcons
- Send pcons when pinging
- Order by index or name

### Chat Commands

`/load [template|name] [hero index]`
- Load build from GW templates
- Hero index 1-7, omit for player

`/loadbuild [teambuild] <build name|code>`
- Load from Toolbox builds
- Partial search on names/codes
- Matches current profession
- Example: `/loadbuild fbgm tk`

---

## Pcons Window

Automatically maintain cons and pcons.

### Features

- Auto-pop when effect expires
- City speedboost on movement
- Consets pop when party loaded
- Res scrolls/powerstones auto-refill
- Per-character toggle option
- Storage refill system
- Threshold warnings

### Settings

**Functionality:**
- Toggle pcons per character
- Tick with pcons
- Disable when not found
- Refill from storage
- Show storage quantity
- Pcons delay
- Lunars delay

**Thresholds:**
- Set minimum quantity per pcon
- Yellow warning when below
- Message on zone to outpost

**Interface:**
- Items per row
- Icon size (32.000 = inventory size)
- Enabled background color

**Lunars and Alcohol:**
- Current drunk level timer
- Suppress post-processing effects
- Suppress drunk text
- Suppress drunk emotes
- Hide Spiritual Possession/Lucky Aura

**Auto-Disabling:**
- On vanquish completion
- In final room Urgoz/Deep
- On final objective (Dhuum/FoW)
- On map change

---

## Materials Window

Auto-buy/sell materials from traders.

### Features

- Buy/sell common and rare materials
- Price check and purchase for:
  - Consets
  - Powerstones
  - Resurrection scrolls
- Auto gold management
- Use existing stock option
- Queue system (sell > buy > price check)
- Real-time progress display

### Usage

1. Stand next to material trader
2. Enter quantity
3. Click "Price Check" or "Buy"
4. For selling, click "Sell"

### Settings

- Automatically manage gold
- Use stock (inventory + storage)

### Known Issues

- May stop when moving from NPC (re-zone/change district)
- Common materials must be in multiples of 10

---

## Info Window

Display various game information.

(Content rephrased for compliance with licensing restrictions)

Shows useful data about current game state, items, NPCs, and other entities. Use to find IDs for hotkeys and other features.

---

## Dialogs Window

Send dialogs to NPCs automatically.

(Content rephrased for compliance with licensing restrictions)

Allows quick interaction with NPCs by sending dialog IDs without manual clicking.

---

## Notepad Window

In-game text editor.

### Features

- Write/paste any text
- Persists across restarts
- Adjustable text size
- Tab support in text input

### Settings

- Text size slider (min to max)

---

## Mission Wiki Window

In-game mission guide viewer fetching content from Guild Wars wiki.

### Features

- Fetch wiki content for current mission
- Display mission map image
- Show primary objectives
- Show bonus objectives
- Display walkthrough strategy
- List recommended skills
- Show tips and notes
- Auto-fetch on map change
- Manual refresh button
- Color-coded sections for readability
- Async loading (non-blocking)

### Display Sections

**Mission Title** (gold) - Current mission name  
**Map Image** - Mission layout for navigation  
**Objectives** (green) - Primary mission goals  
**Bonus** (gold) - Bonus objective requirements  
**Strategy** (light blue) - Walkthrough tips  
**Recommended Skills** (pink) - Suggested skill builds  
**Tips** (light yellow) - Additional notes and warnings

### Usage

1. Type `/missionwiki` in chat, or
2. Open from Settings → Windows menu
3. Window auto-fetches when entering new mission
4. Click "Fetch Current Mission" to manually refresh

### How It Works

1. Detects current map from game
2. Downloads wiki page from https://wiki.guildwars.com
3. Parses HTML to extract sections
4. Displays formatted content in-game
5. Caches images for performance

### Settings

- **Auto-fetch on map change** - Automatically fetch when entering missions
- Window position and size
- Lock move/resize
- Show/hide close button

### Chat Commands

`/missionwiki` - Open window and fetch current mission

### Notes

- Requires internet connection
- Only fetches in actual missions (areas with objectives)
- Content in English only
- Auto-fetch can be toggled in settings

---

## Settings Window

Configure all Toolbox features.

### Top Buttons

- Open settings folder
- Link to website

### Bottom Buttons

- Save settings
- Load settings

---

## Widgets

Widgets stay open as part of your UI. Mostly non-interactive information displays.

### Interface Controls

- **Click and drag** lower right to resize
- **Click and drag** anywhere to move

### Opening/Closing

- Checkboxes in Info window
- Checkboxes in Settings headers
- `/hide (name)`
- `/show (name)`
- `/tb (name)` to toggle

---

## Timer Widget

Shows instance time - how long the current map/district has been running.

---

## Health Widget

Shows target's health:
- Current health
- Maximum health
- Health percentage

**Note:** Only shows percentage until you directly affect target's health. Max health updates when you damage/heal target.

---

## Distance Widget

Shows distance to target:
- Percentage of compass range
- GW distance units (gwinches)

---

## Minimap Widget

Enhanced compass with advanced features.

### Features

- **Ctrl+click** to target nearest agent (600 range)
- Larger area than default compass (5000 gwinches)
- Instant rotation (no delay)
- Accurate pathing maps
- Custom lines/circles/polygons
- Range circles (party/spirit/cast/aggro)
- Heart of Shadow/Viper's Defense range
- Individual hero flagging (all 7 heroes)
- AoE spell effects display
- EoE/QZ/Winnowing areas

### Agent Display

- Teardrops = can move/turn (point = facing)
- Squares = signposts/items
- Circles = stationary (spirits)
- Bosses = custom size
- Custom colors/shapes per model

### Customization

**Scale:** Zoom level (Shift+mousewheel)  
**View:** Shift+drag to pan

**Agent Colors:**
- EoE, QZ, Winnowing areas
- Target border
- Player (alive/dead)
- Signpost, Item
- Hostile (>90%, <90%, dead)
- Neutral (charmable)
- Ally (player/NPC/spirit/minion/dead)
- Quest giver
- Agent/damaged modifiers

**Agent Sizes:**
- Default, Player, Signpost, Item
- Boss, Minion, Marked Target

**Ranges:**
- Party, Spirit, Cast, Aggro
- Custom range circles

**Pings and Drawings:**
- Player drawings
- Ping markers
- Shadowstep marker/line
- Max range warning (color gradient)

**Symbols:**
- Quest marker (star/arrow)
- North marker (arrow)
- Symbol modifier (shading)

**Terrain:**
- Map area
- Shadow (3D effect)
- Background

### Custom Markers

Create lines, circles, or polygons:
- **Lines:** 2 coordinates
- **Circles:** Center + radius (hollow/filled)
- **Polygons:** 3+ points
- Map ID (0 = all maps)
- Optional name
- Toggle on/off
- Can render in 3D world

**Storage:** `Markers.ini` in settings folder (shareable)

### Hero Flagging

- Show/hide flag controls
- Attach/detach from minimap
- Custom background color
- Works in explorable areas

### AoE Effects

Display enemy spell effects:
- Maelstrom
- Chaos Storm
- Savannah Heat
- Breath of Fire
- Lava Font
- Churning Earth
- Barbed Trap
- Flame Trap
- Spike Trap

### Chat Commands

`/flag [all|clear|number]` - Flag hero  
`/flag <number> clear` - Clear hero flag  
`/flag [all|number] [x] [y]` - Flag to coordinates  
`/marktarget` - Highlight current target  
`/marktarget clear` - Remove marked target  
`/clearmarktarget` - Remove all marked targets

---

## Damage Monitor Widget

Records party damage dealt.

### Display

- Damage number
- Percentage of party total
- Thick horizontal bar
- Thin bar for recent damage (resets after pause)

### How It Works

Reads damage packets from server containing:
- Caster (who dealt damage)
- Target (who received damage)
- Amount (as factor of max health, 0-1)

**Calculation:**
1. If client knows target max health: `amount × max_health`
2. Else if seen this foe before: `amount × saved_max_health`
3. Else estimate: `amount × (level × 20 + 100)`

**Notes:**
- Client doesn't know mob health until you damage/heal it
- Toolbox remembers max health per foe type
- Saves to `healthlog.ini` (persists across sessions)

### Limitations

- Only accurate after directly damaging target
- Only within compass range
- Damage only (not degen)
- Player-inflicted only (not EoE)
- Summoning stone damage counts for summoner

### Settings

- Bar colors
- Row height (align with party window)
- Recent damage reset time

### Chat Commands

`/dmg` or `/damage [arg]`:
- `/dmg` or `/dmg report` - Print full results
- `/dmg me` - Print your damage
- `/dmg [number]` - Print party member damage
- `/dmg reset` - Reset monitor

---

## Bonds Widget

Shows maintained monk enchantments on party.

### Display

- Bonds on party members
- Refrains on self/heroes:
  - Heroic Refrain
  - Bladeturn Refrain
  - Burning Refrain
  - Hasty Refrain
  - Mending Refrain

### Features

- Order matches skill bar on zone entry
- Refresh button in Settings
- Reverse order option
- Click to cast/dismiss (toggleable)
- Darkened when stronger version available
- Self-targeted only enchantments hidden

### Settings

- Only show for party members
- Enable/disable clicking
- Custom background color
- Row height adjustment

---

## Clock Widget

Displays local time.

### Settings

- 12 or 24-hour format

---

## Vanquish Widget

Shows vanquish progress.

### Display

- Total enemies in zone
- Enemies killed
- Updates with spawns (quests/pop-ups)

**Note:** Only appears in vanquishable areas in Hard Mode.

---

## Alcohol Widget

Shows drunkenness timer.

### Display

- Time until effect wears off

**Note:** Only appears in explorable areas.

---

## Game Settings

Quality-of-life features in Settings window.

### Chat & Messages

- Show chat timestamps (with color)
- Keep chat history
- Open web links from templates
- Auto-convert URLs to templates
- Target with double-click on author
- Hide email on login screen
- Block inactive channel messages

### Gameplay

- Tick is a toggle
- Disable camera smoothing
- Auto skip cinematics
- Auto return to outpost on defeat
- Disable gold/green confirmation
- Auto accept party invites when ticked
- Auto accept join requests when ticked
- Skip character name for faction
- Auto-cancel Unyielding Aura
- Remove 1.5s cast bar minimum
- Lazy chest looting
- Auto use keys on locked chests
- Auto use lockpick on locked chests
- Keep current quest when accepting new

### Item Management

- Ctrl+Click to move items
- Move to current storage pane
- Move materials to current pane
- Hide item descriptions (explorable/outpost)
- Hide skill descriptions (explorable/outpost)
- Show price check on hover
- Show salvage info on hover

### Notifications

**Flash taskbar when:**
- Receiving PM
- Zoning to new map
- Cinematic starts/ends
- Player starts trade
- Party member pings your name

**Show GW in foreground when:**
- Launching Toolbox
- Zoning to new map
- Player starts trade

**Show chat message when friend:**
- Logs in/out
- Joins/leaves outpost

**Show chat message when player:**
- Joins/leaves party
- Joins/leaves outpost

### Status

- Auto set "Away" after X minutes
- Auto set "Online" after input
- Set window title as character name

### Miscellaneous

- Warn when faction reaches X%
- CE emotes on player dance
- Auto flag pet to called target
- Block floating numbers (faction/exp/0 exp)
- Disable consumable effects:
  - Tonics, Sweets, Bottle rockets
  - Party poppers, Snowman Summoners
  - Fireworks
- Block sparkle on dropped items
- Limit SoC to 10 in skills window
- Block full screen area message
- Customize name tag colors

---

## Toolbox Settings

Configure Toolbox behavior.

### Update Mode

- Do not check
- Check and display message
- Check and ask before updating
- Check and automatically update

### Interface

- Unlock Move All
- Save Location Data
- Enable/disable windows and widgets
- Show/hide buttons in main window

### Storage Location

`C:\Users\[Username]\AppData\Local\GWToolboxpp`

**Files:**
- `img/` - Icons
- `FilterByContent.txt` - Chat filter words
- `Font.ttf` - UI font
- `GWToolbox.dll` - Main DLL
- `GWToolbox.ini` - Settings
- `healthlog.ini` - Mob health data
- `interface.ini` - Window positions/sizes
- `Markers.ini` - Custom minimap markers
- `Theme.ini` - Theme customizations

### Backup

Recommended files to backup:
- `GWToolbox.ini`
- `Markers.ini`
- `Theme.ini`
- All `.ini` and `.txt` files (optional)

### Customization

- Replace `Font.ttf` for custom font
- Replace icon files for custom icons

---

## Other Features

### Chat Filter

Filter unwanted messages from chat.

(Configured in Settings → Chat Filter)

### Chat Commands

Various `/` commands for Toolbox functions.

(See individual feature sections for specific commands)

### Camera Unlock

Extended camera controls.

(Configured via `/cam unlock` command and Settings)

### Theme Editor

Customize Toolbox appearance.

(Configured in Settings → Theme)

### Integrations

- Discord
- Teamspeak
- Twitch

### Plugins

Extensibility system for custom features.

---

**For detailed information on additional features, visit:** https://www.gwtoolbox.com

**Last Updated:** 2026-02-26
