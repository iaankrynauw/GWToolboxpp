# Windows Build & Deploy Process

## Prerequisites
- Windows machine accessible via Tailscale SSH (100.104.178.33)
- Visual Studio 2022 BuildTools installed
- vcpkg package manager
- Git configured with SSH access to GitHub

## Build Directory Setup

**C:\GWToolbox_Build** is now a git repository:
- Remote: git@github.com:iaankrynauw/GWToolboxpp.git
- SSH key configured for authentication
- Can pull/push directly from Windows

## Build Process

### 1. SSH to Windows Host
```bash
ssh krynauw@100.104.178.33
```

### 2. Pull Latest Changes
```powershell
cd C:\GWToolbox_Build
git fetch origin
git checkout <branch-name>
git pull origin <branch-name>
```

**For feature branches:**
```powershell
# Mission Wiki feature
git checkout mission-wiki-feature
git pull origin mission-wiki-feature

# TTS Kokoro streaming
git checkout tts-kokoro-streaming
git pull origin tts-kokoro-streaming
```

### 3. Reconfigure CMake (if new files added)
```powershell
cmake build
```

### 4. Build the DLL
```powershell
cmake --build build --config RelWithDebInfo --target GWToolboxdll
```

### 5. Check for Warnings
```powershell
# Build output should have no warnings
# CI/CD will fail if warnings exist
```

### Known Issues
1. **Regex patterns** - MSVC compiler has issues with complex `ctre` regex patterns
   - Raw string literals with `>` characters cause parse errors
   - Pattern: `R"(<a[^>]+class="image"[^>]*><img[^>]+src="([^"]+Map\.jpg[^"]*)">)"` fails
   - Need to escape or simplify patterns for Windows build
2. **Missing includes** - `#include <Utils/FontLoader.h>` required but not in Linux build
3. **GWCA API differences** - Some context structures may differ between builds

## Build Process

### 1. Copy Files to Windows
```bash
# From Linux machine
scp /path/to/file.cpp krynauw@100.104.178.33:C:/GWToolbox_Build/GWToolboxdll/Windows/
scp /path/to/file.h krynauw@100.104.178.33:C:/GWToolbox_Build/GWToolboxdll/Windows/
```

### 2. SSH to Windows Host
```bash
ssh krynauw@100.104.178.33
```

### 3. Reconfigure CMake (if new files added)
```cmd
cd C:\GWToolbox_Build
cmake build
```

### 4. Build the DLL
```cmd
cmake --build build --config RelWithDebInfo --target GWToolboxdll
```

## Deploy Process

### 1. Close Guild Wars (if running)
```powershell
taskkill /F /IM Gw.exe
```

### 2. Backup Current DLL
```powershell
copy "H:\SteamLibrary\steamapps\common\Guild Wars\plugins\GWToolboxdll.dll" "H:\SteamLibrary\steamapps\common\Guild Wars\plugins\GWToolboxdll_backup.dll"
```

### 3. Deploy New DLL
```powershell
copy "C:\GWToolbox_Build\bin\RelWithDebInfo\GWToolboxdll.dll" "H:\SteamLibrary\steamapps\common\Guild Wars\plugins\GWToolboxdll.dll"
```

### 4. Start Guild Wars
```powershell
start "" "H:\SteamLibrary\steamapps\common\Guild Wars\Gw.exe"
```

### 5. Restore Backup if Needed
```powershell
copy "H:\SteamLibrary\steamapps\common\Guild Wars\plugins\GWToolboxdll_backup.dll" "H:\SteamLibrary\steamapps\common\Guild Wars\plugins\GWToolboxdll.dll"
```

## Quick Workflow

### Testing a Feature Branch
```powershell
# SSH to Windows
ssh krynauw@100.104.178.33

# Pull feature branch
cd C:\GWToolbox_Build
git fetch origin
git checkout mission-wiki-feature
git pull origin mission-wiki-feature

# Build
cmake build
cmake --build build --config RelWithDebInfo --target GWToolboxdll

# Deploy
taskkill /F /IM Gw.exe
copy "C:\GWToolbox_Build\bin\RelWithDebInfo\GWToolboxdll.dll" "H:\SteamLibrary\steamapps\common\Guild Wars\plugins\GWToolboxdll.dll"
start "" "H:\SteamLibrary\steamapps\common\Guild Wars\Gw.exe"
```

### Switching Between Branches
```powershell
cd C:\GWToolbox_Build

# Switch to TTS feature
git checkout tts-kokoro-streaming
git pull origin tts-kokoro-streaming
cmake build
cmake --build build --config RelWithDebInfo --target GWToolboxdll

# Switch to Mission Wiki
git checkout mission-wiki-feature
git pull origin mission-wiki-feature
cmake build
cmake --build build --config RelWithDebInfo --target GWToolboxdll

# Back to master
git checkout master
git pull origin master
cmake build
cmake --build build --config RelWithDebInfo --target GWToolboxdll
```

## Troubleshooting

### Git Issues

**Error: "Permission denied (publickey)"**
- SSH key not added to GitHub
- Solution: Add `~\.ssh\id_ed25519.pub` to https://github.com/settings/ssh/new

**Error: "Your local changes would be overwritten"**
- Uncommitted changes in working directory
- Solution: `git stash` or `git reset --hard origin/<branch>`

**Error: "fatal: not a git repository"**
- Git not initialized
- Solution: `git init && git remote add origin git@github.com:iaankrynauw/GWToolboxpp.git`

### Build Errors

**Error: "unresolved external symbol"**
- CMake didn't pick up new .cpp files
- Solution: Run `cmake build` to reconfigure

**Error: "syntax error" in regex patterns**
- MSVC has stricter parsing than GCC/Clang
- Solution: Simplify regex or use different parsing method

**Error: "cannot deduce template arguments for 'ctll::fixed_string'"**
- Raw string literal syntax issue with MSVC
- Solution: Escape special characters or use runtime regex

**Error: "FontLoader: is not a class or namespace name"**
- Missing include file
- Solution: Add `#include <Utils/FontLoader.h>`

### Deployment Issues

**DLL fails to load**
- Check architecture (must be x86, not x64)
- Verify all dependencies are present
- Check Windows Event Viewer for errors

**Feature doesn't appear**
- Verify registration in ToolboxSettings.cpp
- Check chat command registration in ChatCommands.cpp
- Restart Guild Wars completely

## Alternative: GitHub Actions Build

For production builds, use GitHub Actions instead:

```bash
# Commit and push to trigger build
git push origin master

# Download artifacts from:
# https://github.com/gwdevhub/GWToolboxpp/actions
```

GitHub Actions builds on `windows-latest` with proper environment setup.

## Key Differences: Linux vs Windows Build

| Aspect | Linux (Dev) | Windows (Build) |
|--------|-------------|-----------------|
| Compiler | GCC/Clang | MSVC |
| Regex | More lenient | Stricter parsing |
| Includes | Auto-resolved | Must be explicit |
| CMake | Auto-detects | Needs reconfigure |
| Git | Full repo | No git (manual copy) |

## Recommendations

1. **Test regex patterns** on Windows before committing
2. **Use GitHub Actions** for production builds
3. **Keep Windows build dir** in sync manually
4. **Document MSVC-specific issues** as they arise
5. **Consider cross-compilation** from Linux in future
