# Firewall Issues - Quick Fix Guide

## Problem
PlatformIO was trying to access blocked domains causing build failures:
- `api.registry.nm1.platformio.org`
- `collector.platformio.org`

## Solution
Use the new offline validation script:

```bash
cd firmware/
./offline_validate.sh
```

This script:
✅ Works completely offline
✅ Validates all code (GPIO conflicts, syntax, etc.)
✅ Completes in under 30 seconds
✅ No network access required

## For Full Builds
If you need actual ESP32 compilation (requires network on first run):

1. Configure PlatformIO for offline mode:
   ```bash
   pio settings set enable_telemetry No
   pio settings set check_platformio_interval 0
   ```

2. Use pinned platform version:
   ```bash
   pio run -e esp32-s3-devkitc-1
   ```

## Documentation
See `docs/firewall_fix_guide.md` for complete details.