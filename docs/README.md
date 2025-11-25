# OBS-WebRTC-Link API Documentation

This directory contains the generated API documentation for OBS-WebRTC-Link.

## Prerequisites

To generate the documentation, you need to have the following tools installed:

- **Doxygen** (version 1.8.0 or later)
- **Graphviz** (optional, for generating diagrams)

### Installing Doxygen

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get install doxygen graphviz
```

#### macOS
```bash
brew install doxygen graphviz
```

#### Windows
Download and install from [Doxygen Downloads](https://www.doxygen.nl/download.html) and [Graphviz Downloads](https://graphviz.org/download/).

Alternatively, using Chocolatey:
```powershell
choco install doxygen.install graphviz
```

## Generating Documentation

### Using CMake

1. Configure the project with documentation enabled:
```bash
cmake -B build -DBUILD_DOCUMENTATION=ON
```

2. Build the documentation:
```bash
cmake --build build --target docs
```

The generated HTML documentation will be in `docs/html/`. Open `docs/html/index.html` in your browser to view it.

### Using Doxygen Directly

Alternatively, you can generate the documentation directly with Doxygen:

```bash
doxygen Doxyfile
```

This will create the documentation in the `docs/html/` directory.

## Documentation Structure

The generated documentation includes:

- **Class List**: All classes in the project with their descriptions
- **File List**: All source files with their contents
- **Namespace List**: All namespaces used in the project
- **Class Hierarchy**: Inheritance diagrams (if Graphviz is installed)
- **Call Graphs**: Function call relationships (if Graphviz is installed)
- **Include Graphs**: Header file dependencies (if Graphviz is installed)

## Key Components Documented

### Core Components (`src/core/`)

All core WebRTC components are fully documented:

- **PeerConnection**: WebRTC peer connection wrapper using libdatachannel
- **SignalingClient**: Generic WebSocket-based signaling for SDP and ICE exchange
- **WHIPClient**: WHIP (WebRTC-HTTP Ingestion Protocol) client for publishing streams
- **WHEPClient**: WHEP (WebRTC-HTTP Egress Protocol) client for receiving streams
- **P2PConnection**: Direct peer-to-peer connection management
- **ReconnectionManager**: Automatic reconnection with exponential backoff
- **AudioOnlyConfig**: Audio-only mode configuration and quality presets

### Integration Layer (`src/output/`, `src/source/`)

- **WebRTCOutput**: OBS output implementation for sending streams
- **WebRTCSource**: OBS source implementation for receiving streams

### UI Layer (`src/ui/`)

- **SettingsDialog**: Qt-based settings dialogs (optional)

## Customizing Documentation

You can customize the documentation generation by editing the `Doxyfile` in the project root directory.

Key configuration options:

- `PROJECT_NAME`: Name of the project
- `PROJECT_NUMBER`: Version number
- `OUTPUT_DIRECTORY`: Where to generate the documentation (default: `docs`)
- `EXTRACT_ALL`: Whether to document undocumented code (default: NO)
- `HAVE_DOT`: Enable Graphviz diagram generation (default: YES)

Refer to the [Doxygen Manual](https://www.doxygen.nl/manual/) for more configuration options.

## Publishing Documentation

### GitHub Pages (Optional)

To publish the documentation on GitHub Pages:

1. Generate the documentation
2. Copy the contents of `docs/html/` to a `gh-pages` branch
3. Enable GitHub Pages in the repository settings

Or use the automated workflow in `.github/workflows/docs.yml` (if configured).

## Troubleshooting

### "Doxygen not found"

Make sure Doxygen is installed and available in your PATH:
```bash
doxygen --version
```

### "Graphviz not found" warnings

If you see warnings about Graphviz, you can either:
- Install Graphviz to enable diagram generation
- Disable diagram generation in `Doxyfile` by setting `HAVE_DOT = NO`

### Empty or incomplete documentation

Make sure all source files have proper Doxygen comments. The core classes already have comprehensive documentation in their header files.

## Documentation Standards

This project uses Javadoc-style comments for Doxygen documentation:

```cpp
/**
 * @brief Brief description of the class/function
 *
 * Detailed description explaining the purpose and usage.
 *
 * @param paramName Description of the parameter
 * @return Description of the return value
 * @throws std::runtime_error Description of when this exception is thrown
 */
```

For examples, see the header files in `src/core/`.

## License

The documentation is generated from the OBS-WebRTC-Link source code, which is licensed under the GPLv2 License. See the [LICENSE](../LICENSE) file for details.
