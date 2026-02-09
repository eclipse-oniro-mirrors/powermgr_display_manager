# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the OpenHarmony Display Manager module (powermgr_display_manager), a core system service component that manages display power states, screen brightness control, and display on/off functionality. It's part of the powermgr subsystem and provides system-wide display management capabilities including automatic brightness adjustment and display power strategies.

## Build System

The project uses GN (Generate Ninja) build system. Key configuration:

- **Main config file**: `displaymgr.gni` - Contains feature flags and build configurations
- **Component definition**: `bundle.json` - Defines component structure and dependencies
- **SystemAbility**: Registered as SA 3308 in `state_manager/sa_profile/3308.json`

### Build Commands

```bash
# Independent build (hb build system) - recommended for display_manager component
cd ~/openharmony/ai/test && hb build display_manager -i
# for build display_manager testcase
cd ~/openharmony/ai/test && hb build display_manager -t

# Build the display manager service (in OpenHarmony environment)
gn gen out/display --args="is_standard_system=true"
ninja -C out/display //base/powermgr/display_manager/state_manager/service:displaymgrservice

# Build specific component groups
ninja -C out/display //base/powermgr/display_manager:base_group    # Config files
ninja -C out/display //base/powermgr/display_manager:fwk_group     # Framework APIs
ninja -C out/display //base/powermgr/display_manager:service_group # Service implementations

# Build brightness manager
ninja -C out/display //base/powermgr/display_manager/brightness_manager:brightness_manager

# Build tests
ninja -C out/display //base/powermgr/display_manager/state_manager/test:displaymgr_coverage_test
ninja -C out/display //base/powermgr/display_manager/state_manager/test:displaymgr_fuzztest
ninja -C out/display //base/powermgr/display_manager/state_manager/test:systemtest
```

## Architecture

### High-Level Structure

```
/base/powermgr/display_manager/
├── state_manager/        # Display state manager module
│   ├── frameworks/       # Framework layer (APIs for apps)
│   │   ├── napi/         # Node.js API bindings (JavaScript)
│   │   ├── native/       # Native C++ client APIs
│   │   └── ets/taihe/    # Taihe ETS runtime bindings
│   ├── interfaces/       # Internal API definitions
│   │   └── inner_api/    # C++ internal APIs and IPC interfaces
│   ├── service/          # Service implementation
│   │   ├── etc/          # Display configuration files
│   │   ├── native/       # Native service (DisplayPowerMgrService)
│   │   └── zidl/         # ZIDL interface definitions for IPC
│   ├── utils/            # Utility modules
│   ├── test/             # Test suites
│   │   ├── fuzztest/     # Fuzz testing
│   │   └── systemtest/   # System tests
│   └── sa_profile/       # SystemAbility profile
├── brightness_manager/   # Brightness management module
└── displaymgr.gni        # Build configuration
```

### Key Components

**DisplayPowerMgrService** (`state_manager/service/native/src/display_power_mgr_service.cpp`): Main service singleton (SA 3308)
- Manages display power states (ON, OFF, DIM)
- Handles screen brightness adjustment
- Controls display on/off transitions
- Manages gradual brightness animation
- Implements display power strategies

**ScreenController** (`state_manager/service/native/src/screen_controller.cpp`): Display controller
- Controls screen on/off state
- Manages display brightness
- Handles display state transitions
- Coordinates with display HAL

**GradualAnimator** (`state_manager/service/native/src/gradual_animator.cpp`): Brightness animation
- Smooth brightness transitions
- Gradual dimming effects
- Animation timing control

**DisplayAutoBrightness** (`state_manager/service/native/src/display_auto_brightness.cpp`): Auto brightness
- Ambient light sensor integration
- Automatic brightness adjustment
- Brightness mapping curves

### IPC Communication

Uses ZIDL interfaces defined in `state_manager/service/zidl/`:
- `IDisplayPowerMgr.idl`: Main display manager interface
- `DisplayPowerMgrIdlTypes.idl`: Data types for display management
- Callback registration for brightness/power state changes

### Feature Flags (displaymgr.gni)

Key feature flags controlling behavior:
- `display_manager_feature_brightnessext`: Enable brightness extension features
- `display_manager_feature_poweroff_strategy`: Enable screen power-off strategy

### Conditional Compilation

Feature flags enable preprocessor defines:
- `ENABLE_SCREEN_POWER_OFF_STRATEGY`: When power-off strategy enabled
- `HAS_HIVIEWDFX_HISYSEVENT_PART`: When HiSysEvent available
- `HAS_DFX_HIVIEW_PART`: When HiView available
- `ENABLE_SENSOR_PART`: When sensor module available (for auto brightness)

## Testing

### Test Structure

- **Unit Tests**: Component-level testing in `brightness_manager/test/unittest/`
- **Fuzz Tests**: Security-focused testing with fuzz inputs (`state_manager/test/fuzztest/`)
- **System Tests**: End-to-end scenario testing (`state_manager/test/systemtest/`)

### Testing Private Methods

When testing private/protected methods, use the preprocessor directive approach to make them accessible:

```cpp
// At the top of the test file, before including the header
#define private public
#include "brightness_service.h"
#undef private
```

This technique allows unit tests to access private members for comprehensive testing while maintaining encapsulation in production code.

### Running Tests

```bash
# Run all tests
cd ~/openharmony/ai/test && hb build display_manager -t

# Run specific unit test
ninja -C out/display //base/powermgr/display_manager/brightness_manager/test/unittest:brightness_service_test

# Run specific fuzz test
ninja -C out/display //base/powermgr/display_manager/state_manager/test/fuzztest/setbrightness_fuzzer
```

### Unit Test Coverage Goals

Target test code coverage of **85%** for all production code. This includes:
- Normal operation paths
- Boundary conditions (min/max values, edge cases)
- Error handling paths
- State transitions
- Private/internal methods when critical to functionality

## HiLog Logging

Domain: `0xD002982` (DISPLAY_POWER_MANAGER)
Log Tag: `DisplayPowerSvc`

## Dependencies

Key system dependencies:
- **System Framework**: SAMGR, IPC, HiLog, Eventhandler
- **Display**: Window Manager, Graphic 2D, Skia
- **Power Manager**: Power manager client, power setting, power sysparam
- **Sensors**: Sensor interface (for auto brightness, when available)
- **Settings**: Data share, settings provider
- **Common Event**: HiSysEvent, HiCollie
- **Image Framework**: Image native for display icons

## Entry Points

1. **Main Service**: `DisplayPowerMgrService` (SystemAbility 3308) - `state_manager/service/native/src/display_power_mgr_service.cpp`
2. **Client APIs**: `DisplayPowerMgrClient` - `state_manager/interfaces/inner_api/native/include/display_power_mgr_client.h`
3. **Callbacks**:
   - `IDisplayPowerCallback` - Display power state changes
   - `IDisplayBrightnessCallback` - Brightness changes
   - `IDisplayBrightnessListener` - Brightness update notifications

## Configuration Files

- **display.para**: Display parameter configuration
- **display.para.dac**: Display parameter DAC configuration
- **3308.json**: SystemAbility profile registration

## Additional Notes

- Display Manager depends on Power Manager for some core functionality
- Supports multiple display IDs for multi-display scenarios
- Gradual brightness animation uses screen composer for smooth transitions
- Auto brightness uses ambient light sensor when available
