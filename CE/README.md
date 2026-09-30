# Days Gone 900p Patch - GoldHEN Plugin (SPRX)

A PS4 GoldHEN plugin to force Days Gone (CUSA15027, EUR, v1.81) to run at 900p with reduced shadow/fog quality for better performance on PS4 9.00 + GoldHEN 2.4b18.10.

## Features

- **900p Resolution**: Forces 75% resolution scale (1600x900 equivalent from 1080p)
- **Reduced Shadow Quality**: Lowers shadow quality from Epic/High to Medium
- **Reduced View Distance**: Lowers LOD/fog distance for performance
- **Reduced Post-Processing**: Lowers bloom, DOF, fog quality
- **Runtime Pattern Scanning**: Finds UE4 functions dynamically (works across versions)
- **Automatic Re-application**: Re-applies settings if game resets them

## Requirements

- PS4 on 9.00 firmware
- GoldHEN 2.4b18.10 (or compatible)
- Days Gone CUSA15027 (EUR) v1.81

## Installation

1. Build the SPRX (see Building below) or download from Releases
2. Copy `days_gone_900p.sprx` to your PS4:
   ```
   /data/GoldHEN/plugins/days_gone_900p.sprx
   ```
3. Enable plugins in GoldHEN settings
4. Restart GoldHEN or reboot PS4
5. Launch Days Gone

## Building

### Using GitHub Actions (Recommended)
1. Fork this repository
2. Push to main branch
3. Download `days_gone_900p.sprx` from Actions artifacts

### Local Build (Linux/macOS with Docker)
```bash
docker run --rm -v $(pwd):/src orbisdev/ps4-sdk:latest bash -c "
  cd /src &&
  cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=/opt/ps4/toolchain.cmake &&
  cmake --build build
"
```

### Local Build (Native OrbisDev)
```bash
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=/opt/ps4/toolchain.cmake
cmake --build build
```

Output: `build/days_gone_900p.sprx`

## How It Works

The plugin uses **runtime pattern scanning** to locate UE4's `UGameUserSettings` functions in memory:

1. Scans for known string references (`SetResolutionScaleNormalized`, `SetShadowQuality`, etc.)
2. Finds RIP-relative references to those strings in the code section
3. Walks backwards to find function prologues (`push rbp; mov rbp, rsp`)
4. Calls the functions directly with our desired values

This approach is **version-resilient** - it doesn't rely on hardcoded offsets that change between game versions.

## Target Settings

| Setting | Value | Description |
|---------|-------|-------------|
| Screen Percentage | 75% | 900p from 1080p native |
| Shadow Quality | 1 (Medium) | Reduced from 3 (Epic) |
| View Distance | 1 (Medium) | Reduces fog/LOD distance |
| Post Process | 1 (Medium) | Reduces bloom, DOF, fog |

## Verification

In-game, check:
- Resolution: Should render at ~1600x900 internally (upscaled to 1080p output)
- Shadows: Less detailed, shorter distance
- Fog: Thinner, shorter draw distance
- Performance: Improved frame rate in heavy scenes

## Troubleshooting

**Plugin doesn't load:**
- Check GoldHEN plugin loader logs: `/data/GoldHEN/logs/`
- Verify SPRX is in correct folder
- Ensure GoldHEN plugins are enabled

**Settings don't apply:**
- Game may reset settings on resolution change
- Plugin attempts re-application on each tick
- Check kernel logs for `[days_gone_900p]` messages

**Crash on launch:**
- Pattern scan may have found wrong function
- Try rebuilding with updated patterns
- Report crash dump for analysis

## Technical Details

### Functions Hooked/Used
- `UGameUserSettings::SetResolutionScaleNormalized(float)`
- `UGameUserSettings::SetResolutionScaleValue(float)`
- `UGameUserSettings::SetScreenResolution(int, int)`
- `UGameUserSettings::SetShadowQuality(int)`
- `UGameUserSettings::SetViewDistanceQuality(int)`
- `UGameUserSettings::SetPostProcessingQuality(int)`
- `UGameUserSettings::ApplyResolutionSettings()`
- `UGameUserSettings::GetGameUserSettings()`

### Pattern Scan Regions
- Code: `0x0` - `0x5724830` (from ELF program headers)
- Strings: `0x4a80000` - `0x4a90000` (rodata section)

## License

MIT License - Use at your own risk. This modifies game memory at runtime.

## Credits

- OrbisDev team for PS4 SDK
- GoldHEN team for plugin loader
- UE4 reverse engineering community