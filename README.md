# gKiosk

A lightweight GTK3/WebKit2 kiosk browser for Linux that displays a fullscreen web page, ideal for digital signage, point-of-sale displays, and dedicated web terminals.

## Quick Start

```bash
# Build
make

# Run (displays example.com by default)
./cmake-build/gKiosk

# Run with custom URL
KIOSK_URL=https://your-site.com ./cmake-build/gKiosk
```

## Features

- Fullscreen WebKit2 browser window
- Configurable target URL via `KIOSK_URL` environment variable
- JavaScript enabled, Java disabled by default
- Developer tools support
- Minimal resource footprint

## Dependencies

- GTK+ 3.0
- WebKit2GTK 4.0
- CMake 3.20+
- Ninja build system

## Building

```bash
make clean   # Remove build artifacts
make         # Configure and build
```

## Configuration

| Environment Variable | Default | Description |
|---------------------|---------|-------------|
| `KIOSK_URL` | `https://example.com/` | URL to display in kiosk mode |

## License

See LICENSE file for details.
