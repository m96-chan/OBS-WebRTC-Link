# WebRTC Link UI Components

This directory contains Qt-based user interface components for the OBS WebRTC Link plugin.

## Components

### Settings Dialog (`settings-dialog.hpp` / `settings-dialog.cpp`)

A comprehensive settings dialog for configuring WebRTC connection parameters.

**Features:**
- Connection mode selection (SFU/P2P)
- Server URL input with validation
- Authentication token field (password-protected)
- Video codec selection (H.264, VP8, VP9, AV1)
- Video bitrate configuration (100-50000 kbps)
- Audio codec selection (Opus, AAC)
- Audio bitrate configuration (32-512 kbps)
- Input validation with user-friendly error messages
- Multi-language support (en-US, ja-JP)

**Integration:**
The settings dialog is integrated with OBS properties through the "Advanced Settings..." button in the source properties panel.

## Requirements

### Build Requirements
- **Qt 5.15+ or Qt 6.x** - Qt GUI framework
  - Core, Gui, and Widgets modules required
  - Automatically detected during CMake configuration

### Testing
- Settings dialog has comprehensive unit tests in `tests/unit/settings_dialog_test.cpp`
- Tests cover:
  - UI component creation and initialization
  - Getter/setter functionality for all settings
  - Input validation (URL format, bitrate ranges)
  - Default values
  - User interaction workflows

## Building

The UI components are automatically built when Qt is detected:

```bash
# Configure with Qt support
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build will include UI components if Qt is found
cmake --build build
```

If Qt is not available, the plugin will build without UI components, falling back to OBS's default properties system.

## Development

### Adding New UI Components

1. Create header and implementation files in `src/ui/`
2. Add source files to `CMakeLists.txt` under the `if(QT_FOUND)` section
3. Include appropriate Qt headers
4. Use `ENABLE_QT_UI` preprocessor macro to conditionally compile UI code
5. Write comprehensive unit tests

### Localization

UI strings should be added to both:
- `data/locale/en-US.ini` (English)
- `data/locale/ja-JP.ini` (Japanese)

Use the OBS localization system:
```cpp
obs_module_text("SettingsDialog.Title")
```

Or Qt's translation system:
```cpp
tr("Server URL:")
```

### Integration with OBS Properties

To integrate a dialog with OBS properties:

1. Add button to properties:
```cpp
obs_properties_add_button(props, "button_id",
                         obs_module_text("Button Label"),
                         callback_function);
```

2. Implement callback:
```cpp
static bool callback_function(obs_properties_t *props,
                             obs_property_t *property,
                             void *data) {
    // Create and show dialog
    MyDialog dialog(nullptr);

    // Load settings from obs_data_t
    obs_data_t *settings = obs_source_get_settings(source);
    dialog.loadSettings(settings);

    // Show and save if accepted
    if (dialog.exec() == QDialog::Accepted) {
        dialog.saveSettings(settings);
        obs_source_update(source, settings);
    }

    obs_data_release(settings);
    return true;
}
```

## Testing

Run UI tests (requires Qt):

```bash
# Build tests
cmake --build build --target settings_dialog_test

# Run tests
./build/tests/unit/settings_dialog_test
```

## Architecture

```
src/ui/
├── settings-dialog.hpp        # Settings dialog header
├── settings-dialog.cpp        # Settings dialog implementation
└── README.md                  # This file

Integration points:
├── src/source/obs-webrtc-source.cpp  # Source plugin integration
└── src/output/obs-webrtc-output.cpp  # Output plugin integration (future)
```

## Future Enhancements

Potential additions for Issue #14 and beyond:

- [ ] Qt Designer `.ui` files for visual design
- [ ] Connection status indicator
- [ ] Advanced network settings (STUN/TURN servers)
- [ ] Stream quality presets
- [ ] Connection diagnostics panel
- [ ] Log viewer
- [ ] Real-time bandwidth monitoring

## Notes

- UI components are conditionally compiled with `ENABLE_QT_UI` macro
- Plugin works without Qt by using OBS's native properties
- All UI code should be thread-safe when accessing shared data
- Settings are persisted through OBS's `obs_data_t` system
- Dialog validation prevents invalid configurations from being saved
