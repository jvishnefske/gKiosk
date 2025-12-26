# gKiosk Design Document

## Overview

gKiosk is a fullscreen kiosk browser built on GTK3 and WebKit2GTK for Linux systems.

## MVP Functional Requirements

- [ ] FR-001: Application launches a fullscreen window
- [ ] FR-002: WebKit2 web view loads and renders web content
- [ ] FR-003: Default URL (example.com) loads when KIOSK_URL is not set
- [ ] FR-004: Custom URL loads when KIOSK_URL environment variable is set
- [ ] FR-005: JavaScript execution is enabled
- [ ] FR-006: Java plugin is disabled for security
- [ ] FR-007: Application exits cleanly when window is closed
- [ ] FR-008: Build system produces executable from source

## Architecture

### Components

1. **Main Window** (`main.cc`)
   - GTK Application lifecycle management
   - Window creation and fullscreen mode
   - Signal handling for activation

2. **Web View Configuration**
   - WebKit2 settings initialization
   - Security hardening (Java disabled)
   - Developer tools access

3. **Task Executor** (`taskHelper.cpp`)
   - Boost.Asio-based coroutine scheduler
   - Interval-based task execution
   - Clean shutdown support

### Design Decisions

- **GTK3/WebKit2GTK**: Industry-standard Linux web rendering
- **Environment variable configuration**: Simple, container-friendly
- **Fullscreen by default**: Kiosk use case optimization
- **Shared pointer for GtkApplication**: RAII resource management

## Build System

- CMake for configuration
- Ninja for fast incremental builds
- Make wrapper for common operations

## Testing Strategy

- Unit tests for TaskExecutor scheduling logic
- Integration tests for URL loading behavior
- Coverage targets via gcov/lcov
