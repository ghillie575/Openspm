# OpenSPM - Open Source Package Manager

OpenSPM is a lightweight, cross-platform package manager designed for distributing and managing software packages from remote repositories.

## Features

- 📦 Simple package installation and removal
- 🌐 Support for multiple remote repositories
- 🏷️ Tag-based package compatibility system
- 🔄 Automatic dependency resolution
- 💾 Compressed archive-based package storage
- 🎨 Colorized terminal output

## Requirements

### Build Dependencies

- CMake 3.10.0 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- pkg-config

### System Libraries

- libarchive
- libzstd (Zstandard compression)

### Automatically Fetched Dependencies

The following dependencies are automatically downloaded during build:
- yaml-cpp (YAML parser)
- cpp-httplib (HTTP client library)

## Building

```bash
# Install system dependencies (Ubuntu/Debian)
sudo apt-get install cmake build-essential pkg-config libzstd-dev libarchive-dev

# Configure CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release --parallel

# The executable will be at build/openspm
```

### Build Types

- **Release**: Optimized build for production use
- **Debug**: Development build with debug symbols

## Installation

After building, you can install OpenSPM system-wide:

```bash
sudo cp build/openspm /usr/local/bin/
```

## Usage

### Initial Configuration

Run the interactive configuration wizard:

```bash
openspm configure
```

This will prompt you to set:
- Data directory (default: `/etc/openspm/`)
- Target installation directory (default: `/usr/local/`)
- Color output preference

### Managing Repositories

Add a repository:
```bash
openspm add-repo <repository-url>
```

Update repository information:
```bash
openspm update-repos
```

Update package lists:
```bash
openspm update-packages
```

Update both repositories and packages:
```bash
openspm update
```

### Managing Packages

List available packages:
```bash
openspm list
```

Install a package:
```bash
openspm install <package-name>
```

Remove a package:
```bash
openspm remove <package-name>
```

### Command-line Flags

Global flags that can be used with any command:

- `--data-dir <path>`: Override the data directory
- `--target-dir <path>`: Override the target installation directory
- `--tags <tags>`: Override supported tags (semicolon-separated)
- `--no-color` or `-nc`: Disable colored output

Example:
```bash
openspm install mypackage --data-dir /custom/data --no-color
```

### Version Information

Display version and build date:
```bash
openspm version
```

## Repository Format

Repositories must provide two files:

### repository.yaml
```yaml
name: Repository Name
description: Repository description
mantainer: Maintainer name
```

### pkg-list.yaml
```yaml
depend:
  - https://other-repo.example.com

packages:
  - name: package-name
    version: "1.0.0"
    description: Package description
    maintainer: Maintainer name
    tags: bin;linux-x86_64;
    url: https://example.com/package.tar.gz
```

## Tag System

OpenSPM uses tags to ensure package compatibility. Common tags include:

- Platform tags: `linux-x86_64`, `windows-x86_64`, `macos-x86_64`
- Type tags: `bin` (binary), `non-bin` (source)
- Compiler tags: `gcc`, `gcc-13`, etc.

Packages are only installable if all their tags are supported by the system.

## Project Structure

```
openspm/
├── include/          # Header files
│   ├── archive.hpp
│   ├── config.hpp
│   ├── logger.hpp
│   ├── openspm_cli.hpp
│   ├── package_manager.hpp
│   ├── repository_manager.hpp
│   └── utils.hpp
├── src/              # Implementation files
│   ├── archive.cpp
│   ├── config.cpp
│   ├── logger.cpp
│   ├── openspm_cli.cpp
│   ├── package_manager.cpp
│   ├── repository_manager.cpp
│   └── utils.cpp
├── examples/         # Example repository files
├── main.cpp          # Entry point
└── CMakeLists.txt    # Build configuration
```

## Documentation

Full API documentation can be generated using Doxygen:

```bash
# Configure with Release build type to enable Doxygen
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build documentation
cmake --build build --target doxygen

# Open documentation (Linux)
xdg-open build/docs/html/index.html
```

## Contributing

Contributions are welcome! Please ensure your code:
- Follows the existing code style
- Includes Doxygen comments for public APIs
- Builds successfully with both Debug and Release configurations

## License

This project is open source. Please check the repository for license details.

## Author

Developed by ghillie575
