# OpenSPM - Open Source Package Manager

OpenSPM is a lightweight, cross-platform package manager designed for easy distribution and installation of binary and source packages. It provides a simple YAML-based repository format and supports dependency management, platform-specific packages, and tag-based compatibility.

## Features

- **Simple Repository Format**: YAML-based package metadata that's easy to create and maintain
- **HTTP/HTTPS Support**: Fetch packages from any web server
- **Tag-Based Compatibility**: Platform and compiler tags ensure packages match your system
- **Dependency Management**: Automatic resolution of package dependencies
- **Repository Chaining**: Repositories can depend on other repositories
- **Cross-Platform**: Supports Linux, macOS, and Windows
- **Archive-Based Storage**: Efficient compressed storage for package metadata

## Installation

### Prerequisites

OpenSPM requires the following dependencies:

- CMake 3.10 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- libarchive (system package)
- libzstd (system package)
- pkg-config

On Ubuntu/Debian:
```bash
sudo apt-get install cmake g++ libarchive-dev libzstd-dev pkg-config
```

On Fedora/RHEL:
```bash
sudo dnf install cmake gcc-c++ libarchive-devel libzstd-devel pkgconfig
```

On macOS (using Homebrew):
```bash
brew install cmake libarchive zstd pkg-config
```

### Building from Source

1. Clone the repository:
```bash
git clone https://github.com/ghillie575/Openspm.git
cd Openspm
```

2. Create a build directory and compile:
```bash
mkdir build
cd build
cmake ..
make
```

3. Install (requires administrator/root privileges):
```bash
sudo make install
```

Alternatively, you can run the binary directly from the build directory:
```bash
./openspm --version
```

## Usage

### Initial Configuration

Before using OpenSPM, run the configuration wizard (requires administrator/root privileges):

```bash
sudo openspm configure
```

This will guide you through setting up:
- Data directory (default: `/etc/openspm/`)
- Target installation directory (default: `/usr/local/`)
- Color output preferences

### Managing Repositories

#### Adding a Repository

Add a package repository by providing its URL:

```bash
sudo openspm add-repo https://example.com/repository
```

Short form:
```bash
sudo openspm ar https://example.com/repository
```

#### Listing Repositories

View all configured repositories:

```bash
sudo openspm list-repos
```

Short form:
```bash
sudo openspm lr
```

#### Removing a Repository

Remove a repository:

```bash
sudo openspm rm-repo https://example.com/repository
```

Short form:
```bash
sudo openspm rr https://example.com/repository
```

#### Updating Repository Metadata

Sync metadata from all configured repositories:

```bash
sudo openspm update-repos
```

Short form:
```bash
sudo openspm ur
```

### Managing Packages

#### Listing Available Packages

List all packages compatible with your system:

```bash
sudo openspm list-packages
```

Short form:
```bash
sudo openspm lp
```

#### Updating Package Index

Update the package index from all repositories:

```bash
sudo openspm update
```

Short form:
```bash
sudo openspm up
```

### Command-Line Options

#### Global Flags

- `--data-dir <dir>`: Override the data directory
- `--target-dir <dir>`: Override the installation target directory
- `--tags <tags>`: Override system tags (e.g., "gcc;bin;linux-x86_64")
- `--logfile <file>`: Specify log file location
- `--no-color` or `-nc`: Disable colored output
- `--debug`: Enable verbose debug logging

Example:
```bash
sudo openspm --debug --data-dir /tmp/openspm list-packages
```

### Getting Help

Display help information:

```bash
openspm help
```

Or:
```bash
openspm --help
```

## Creating Your Own Repository

### Repository Structure

A repository consists of two main YAML files hosted on a web server:

1. **repository.yaml** - Repository metadata
2. **pkg-list.yaml** - Package index

### Step 1: Create repository.yaml

Create a file named `repository.yaml` with your repository information:

```yaml
name: My Package Repository
description: A collection of useful packages
mantainer: your-name
```

**Note:** Despite the typo in the field name, use `mantainer` (not "maintainer") for compatibility.

### Step 2: Create pkg-list.yaml

Create a file named `pkg-list.yaml` listing your packages:

```yaml
# Optional: Depend on other repositories
depend:
  - https://another-repo.example.com/repository

# Package list
packages:
  - name: "my-package"
    version: "1.0.0"
    description: "Description of my package"
    maintainer: "your-name"
    dependencies: []
    tags: "bin;linux-x86_64"
    url: "https://your-server.com/packages/my-package-1.0.tar.gz"

  - name: "another-package"
    version: "2.1.0"
    description: "Another useful package"
    maintainer: "your-name"
    dependencies:
      - "my-package"
    tags: "bin;linux-x86_64;gcc"
    url: "https://your-server.com/packages/another-package-2.1.tar.gz"
```

### Step 3: Understanding Tags

Tags determine package compatibility with different systems. Common tags include:

- **Platform tags**: `linux-x86_64`, `macos-x86_64`, `windows-x86_64`
- **Binary/Source**: `bin` (binary), `non-bin` (source)
- **Compiler tags**: `gcc`, `gcc-11`, `gcc-11.2`

A package is compatible only if the target system supports **all** of the package's tags.

Examples:
- `tags: "bin;linux-x86_64"` - Binary package for 64-bit Linux
- `tags: "bin;linux-x86_64;gcc"` - Binary requiring GCC on 64-bit Linux
- `tags: "non-bin"` - Source package requiring compilation

### Step 4: Package Format

Packages should be distributed as `.tar.gz` archives. The archive structure is flexible, but typically includes:

```
my-package-1.0.tar.gz
├── bin/
│   └── my-executable
├── lib/
│   └── libmylib.so
├── include/
│   └── mylib.h
└── install.sh (optional)
```

### Step 5: Host Your Repository

Upload both YAML files to a web server at the same directory level:

```
https://your-server.com/repository/
├── repository.yaml
└── pkg-list.yaml
```

Your repository URL is: `https://your-server.com/repository`

### Step 6: Test Your Repository

Add your repository to OpenSPM:

```bash
sudo openspm add-repo https://your-server.com/repository
sudo openspm update
sudo openspm list-packages
```

### Example Repository

See the `examples/` directory for a complete example:

- `examples/repository.yaml` - Example repository metadata
- `examples/pkg-list.yaml` - Example package index with dependencies
- `examples/package/` - Example package structure

## Configuration

### Configuration File

OpenSPM stores its configuration in `/etc/openspm/config.yaml`:

```yaml
dataDir: /etc/openspm/
targetDir: /usr/local/
colorOutput: true
platform: linux-x86_64
supported_tags: bin;linux-x86_64;gcc;gcc-11;non-bin;
supported: true
unsupported_msg: ""
```

### Data Storage

OpenSPM uses an archive file for storing metadata:
- **Archive location**: `<dataDir>/data.bin`
- **Contents**: 
  - `repositories.yaml` - List of configured repositories
  - `packages.yaml` - Aggregated package index

### Log Files

Logs are written to `/var/log/openspm/openspm.log` by default. Change this with the `--logfile` flag or by modifying the configuration.

## Development

### Project Structure

```
Openspm/
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
├── main.cpp          # Entry point
├── tests/            # Test files
├── examples/         # Example repository and packages
└── CMakeLists.txt    # Build configuration
```

### Building for Development

For development builds with debug symbols:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

This enables debug logging by default (version will be suffixed with `-dev`).

### Running Tests

```bash
cd build
ctest
```

Or run tests with verbose output:

```bash
ctest --verbose
```

### Code Overview

- **archive.hpp/cpp**: Compressed archive management for metadata storage
- **config.hpp/cpp**: Configuration loading, saving, and management
- **logger.hpp/cpp**: Logging system with file and console output
- **openspm_cli.hpp/cpp**: Command-line interface and command processing
- **package_manager.hpp/cpp**: Package fetching, listing, and management
- **repository_manager.hpp/cpp**: Repository operations and metadata
- **utils.hpp/cpp**: Utility functions (URL parsing, tag comparison)
- **main.cpp**: Entry point with argument parsing and privilege checks

## Platform Support

### Linux
Fully supported. Requires root privileges for system-wide installation.

### macOS
Supported. Requires administrator privileges.

### Windows
Partially supported. Requires administrator privileges. Some features may have limited functionality.

## Troubleshooting

### Permission Denied

OpenSPM requires administrator/root privileges for most operations:

```bash
# Linux/macOS
sudo openspm <command>

# Windows (run Command Prompt as Administrator)
openspm <command>
```

### No Repositories Found

If you see "No repositories found", you need to add at least one repository:

```bash
sudo openspm add-repo https://example.com/repository
```

### Color Output Issues

If colors don't display correctly in your terminal, disable them:

```bash
openspm --no-color <command>
```

Or configure during setup:

```bash
sudo openspm configure
```

### Debug Information

Enable debug logging to troubleshoot issues:

```bash
sudo openspm --debug <command>
```

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues on GitHub.

## License

This project is open source. See the repository for license details.

## Author

Created by ghillie575

## Version

Current version: 0.1.0

For the latest version information, run:
```bash
openspm --version
```
