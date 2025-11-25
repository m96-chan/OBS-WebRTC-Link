# Images Directory

This directory contains images for documentation purposes.

## Directory Structure

- **screenshots/**: UI screenshots of the OBS plugin
- **diagrams/**: Architecture and workflow diagrams
- **examples/**: Example use case images

## Screenshot Requirements

### Screenshots Needed

#### Plugin UI
- `screenshots/obs-source-settings-whep.png` - WebRTC Link Source settings (WHEP mode)
- `screenshots/obs-source-settings-p2p.png` - WebRTC Link Source settings (P2P mode)
- `screenshots/obs-output-settings-whip.png` - WebRTC Output settings (WHIP mode)
- `screenshots/obs-output-settings-p2p.png` - WebRTC Output settings (P2P mode)
- `screenshots/obs-sources-menu.png` - Adding WebRTC Link Source from OBS menu

#### Use Case Examples
- `examples/browser-to-obs.png` - Browser sending video to OBS
- `examples/obs-to-obs-relay.png` - OBS-to-OBS relay setup
- `examples/mobile-camera.png` - Mobile device as wireless camera

## Image Guidelines

### Format
- **Preferred**: PNG (for UI screenshots)
- **Alternative**: WebP (for smaller file sizes)
- Avoid JPEG for UI screenshots (compression artifacts)

### Size
- Maximum width: 1920px
- Recommended width for UI screenshots: 800-1200px
- Compress images to keep file sizes under 500KB

### Optimization Tools
- **pngquant**: Lossy PNG compression
- **optipng**: Lossless PNG optimization
- **ImageOptim** (macOS): All-in-one optimizer
- **squoosh.app**: Web-based image optimizer

### Naming Convention
- Use lowercase with hyphens: `my-screenshot.png`
- Include context in filename: `obs-source-settings-whep.png`
- Avoid spaces and special characters

### Privacy
- Remove or blur any sensitive information (tokens, API keys, URLs)
- Use example/placeholder data where possible

## Adding Screenshots

1. Take screenshot of the relevant OBS plugin UI
2. Optimize the image using tools above
3. Save to appropriate subdirectory
4. Update documentation to reference the image

## Placeholder Images

Until actual screenshots are available, documentation uses placeholders in the format:
```markdown
![Description](docs/images/screenshots/placeholder.png)
*Screenshot will be added in future release*
```
