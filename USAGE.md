# OpenSPM Command-Line Usage Guide

This document provides detailed usage information for the OpenSPM command-line executable.

## Synopsis

```
openspm [global-flags] <command> [command-args] [command-flags]
```

## Global Flags

These flags can be used with any command and must appear before the command name:

| Flag | Description |
|------|-------------|
| `--data-dir <dir>` | Override the data directory (default: `/etc/openspm/`) |
| `--target-dir <dir>` | Override the installation target directory (default: `/usr/local/`) |
| `--tags <tags>` | Override system tags (semicolon-separated, e.g., `"gcc;bin;linux-x86_64"`) |
| `--logfile <file>` | Specify log file location (default: `/var/log/openspm/openspm.log`) |
| `--no-color`, `-nc` | Disable colored output |
| `--debug` | Enable verbose debug logging |

### Examples

```bash
# Use custom data directory with debug logging
sudo openspm --debug --data-dir /tmp/openspm list-packages

# Override system tags
sudo openspm --tags "bin;linux-x86_64;gcc-11" list-packages

# Custom log file location without color
openspm --logfile /tmp/openspm.log --no-color help
```

## Commands

### Configuration

#### `configure`
Run the interactive configuration wizard to set up OpenSPM.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm configure
```

The wizard will prompt for:
- Data directory (where OpenSPM stores metadata)
- Target installation directory (where packages are installed)
- Color output preference

### Repository Management

#### `add-repo` (alias: `ar`)
Add a package repository to OpenSPM.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm add-repo <repository-url>
sudo openspm ar <repository-url>
```

**Arguments:**
- `<repository-url>`: Full URL to the repository (e.g., `https://example.com/repository`)

**Example:**
```bash
sudo openspm add-repo https://packages.example.com/openspm-repo
```

#### `list-repos` (alias: `lr`)
Display all configured repositories.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm list-repos
sudo openspm lr
```

**Output:** Lists repository URLs, names, descriptions, and maintainers.

#### `rm-repo` (alias: `rr`)
Remove a repository from OpenSPM.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm rm-repo <repository-url>
sudo openspm rr <repository-url>
```

**Arguments:**
- `<repository-url>`: URL of the repository to remove

**Example:**
```bash
sudo openspm rm-repo https://packages.example.com/openspm-repo
```

#### `update-repos` (alias: `ur`)
Update metadata from all configured repositories.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm update-repos
sudo openspm ur
```

This command fetches the latest repository information (name, description, maintainer) from all configured repositories.

### Package Management

#### `list-packages` (alias: `lp`)
List all packages compatible with your system.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm list-packages
sudo openspm lp
```

**Output:** Lists package names, versions, descriptions, and maintainers for packages whose tags match your system's supported tags.

#### `update` (alias: `up`)
Update the package index from all repositories.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm update
sudo openspm up
```

This command:
1. Fetches package lists from all configured repositories
2. Resolves repository dependencies (repositories that depend on other repositories)
3. Updates the local package database
4. Filters packages to show only those compatible with your system

#### `collect` (alias: `c`)
Collect and install a package along with its dependencies.

**Requires:** Administrator/root privileges

**Usage:**
```bash
sudo openspm collect <package-name>
sudo openspm c <package-name>
```

**Arguments:**
- `<package-name>`: Name of the package to install

**Example:**
```bash
sudo openspm collect my-package
```

This command:
1. Resolves package dependencies recursively
2. Shows the list of packages to be installed
3. Prompts for confirmation
4. Downloads and installs all packages

### Help and Information

#### `help` (alias: `--help`, `-h`)
Display help information and available commands.

**Usage:**
```bash
openspm help
openspm --help
openspm -h
```

#### `--version`
Display the OpenSPM version.

**Usage:**
```bash
openspm --version
```

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | General error or invalid arguments |
| 2+ | Command-specific error |

## Privilege Requirements

Most OpenSPM commands require administrator/root privileges because they:
- Modify system configuration files in `/etc/openspm/`
- Install packages to system directories (e.g., `/usr/local/`)
- Write logs to `/var/log/openspm/`

**Linux/macOS:**
```bash
sudo openspm <command>
```

**Windows:**
Run Command Prompt as Administrator, then:
```cmd
openspm <command>
```

## Configuration Files

### Main Configuration
**Location:** `/etc/openspm/config.yaml`

**Example:**
```yaml
dataDir: /etc/openspm/
targetDir: /usr/local/
colorOutput: true
platform: linux-x86_64
supported_tags: bin;linux-x86_64;gcc;gcc-11;non-bin;
supported: true
unsupported_msg: ""
```

### Data Archive
**Location:** `<dataDir>/data.bin`

This is a compressed tar.gz archive containing:
- `repositories.yaml` - List of configured repositories
- `packages.yaml` - Aggregated package index

## Tag System

OpenSPM uses a tag-based compatibility system. A package is compatible with your system only if all of its tags are in your system's supported tags.

### Common Tags

**Platform tags:**
- `linux-x86_64` - 64-bit Linux
- `macos-x86_64` - 64-bit macOS
- `windows-x86_64` - 64-bit Windows

**Binary/Source:**
- `bin` - Binary (pre-compiled) package
- `non-bin` - Source package (requires compilation)

**Compiler tags:**
- `gcc` - GCC compiler
- `gcc-11` - GCC version 11
- `clang` - Clang compiler

### Tag Matching Examples

System tags: `bin;linux-x86_64;gcc;gcc-11;non-bin;`

| Package Tags | Compatible? | Reason |
|--------------|-------------|---------|
| `bin;linux-x86_64` | ✓ Yes | All package tags are in system tags |
| `bin;linux-x86_64;gcc` | ✓ Yes | All package tags are in system tags |
| `bin;macos-x86_64` | ✗ No | `macos-x86_64` not in system tags |
| `bin;windows-x86_64` | ✗ No | `windows-x86_64` not in system tags |
| `non-bin;gcc` | ✓ Yes | All package tags are in system tags |

## Troubleshooting

### "Permission Denied"
**Solution:** Run with administrator/root privileges using `sudo` (Linux/macOS) or run as Administrator (Windows).

### "No repositories found"
**Solution:** Add at least one repository:
```bash
sudo openspm add-repo https://example.com/repository
```

### Color output issues
**Solution:** Disable colored output:
```bash
openspm --no-color <command>
```

Or configure during setup to disable colors permanently.

### Getting debug information
**Solution:** Enable debug logging:
```bash
sudo openspm --debug <command>
```

Debug logs show detailed information about:
- Configuration loading
- Network requests
- Tag matching
- Archive operations
- Package resolution

## Advanced Usage

### Multiple Repositories with Dependencies
Repositories can depend on other repositories by referencing their `pkg-list.yaml` files. OpenSPM automatically resolves these dependencies.

**Example workflow:**
```bash
# Add main repository
sudo openspm add-repo https://main.example.com/repo

# Update to fetch dependencies
sudo openspm update

# List all packages (from main repo and its dependencies)
sudo openspm list-packages
```

### Custom Tag Configuration
Override system tags to test package compatibility:

```bash
# Test with different platform
sudo openspm --tags "bin;macos-x86_64;gcc" list-packages

# Test with specific compiler version
sudo openspm --tags "bin;linux-x86_64;gcc;gcc-13" list-packages
```

### Testing Configuration Without Installing
Use custom directories for testing:

```bash
# Test with temporary directories
sudo openspm --data-dir /tmp/openspm-test \
             --target-dir /tmp/install-test \
             configure
```

## See Also

- Main README: `README.md`
- Repository creation guide in README
- Example repository in `examples/` directory
- API documentation (generated by Doxygen)
