# Steam Log Collector

A cross-platform tool for collecting and analyzing Steam game logs. Available as both a command-line interface (CLI) and a graphical user interface (GUI) application.

## Description

Steam Log Collector helps users locate, view, and collect log files from their Steam games. The tool automatically detects Steam installation directories, scans for installed games, and allows users to browse and collect log files for troubleshooting and debugging purposes.

### Features

- **Automatic Steam Detection**: Finds your Steam installation directory on Windows, macOS, and Linux
- **Game Library Scanner**: Lists all installed Steam games with their App IDs
- **Log File Discovery**: Searches for log files within game directories
- **File Preview**: View log file contents before copying
- **Batch Collection**: Copy all logs from a game to a single directory
- **Dual Interface**: Choose between CLI for automation or GUI for ease of use
- **Cross-Platform**: Works on Windows, macOS, and Linux

## Libraries and Dependencies

### Core Dependencies

- **CMake** (>= 4.0.3): Build system generator
- **C++17 Compiler**: Required for modern C++ features
- **OpenGL**: Graphics rendering (for GUI only)

### Third-Party Libraries

The following libraries are automatically fetched and built by CMake via FetchContent:

- **[GLFW 3.4](https://github.com/glfw/glfw)** (GUI only): Window and input handling
- **[Dear ImGui v1.92.5](https://github.com/ocornut/imgui)** (GUI only): Immediate mode GUI framework
  - Includes OpenGL3 and GLFW backends

### Platform-Specific Dependencies

#### Windows

- **advapi32.lib**: Windows Registry access for Steam directory detection
- Visual Studio 2022 (recommended) or MinGW/Clang with C++17 support

#### macOS

- Xcode Command Line Tools or compatible C++ compiler
- OpenGL framework (included with macOS)

#### Linux

- GCC or Clang with C++17 support
- OpenGL development libraries
- X11 development libraries (for GLFW)

## Building the Project

### Prerequisites

Ensure you have the following installed:

- CMake 4.0.3 or higher
- A C++17-compatible compiler
- Git (for fetching dependencies)

### Platform-Specific Prerequisites

#### Windows

Install one of the following:

- **Visual Studio 2022** with "Desktop development with C++" workload
- **MinGW-w64** with GCC 7.0 or higher
- **Clang** for Windows

#### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install CMake (using Homebrew)
brew install cmake
```

#### Linux (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

#### Linux (Fedora/RHEL)

```bash
sudo dnf install gcc-c++ cmake git
sudo dnf install mesa-libGL-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel
```

#### Linux (Arch)

```bash
sudo pacman -S base-devel cmake git
sudo pacman -S mesa libx11 libxrandr libxinerama libxcursor libxi
```

### Build Instructions

#### Windows (Visual Studio)

```powershell
# Clone the repository
git clone <repository-url>
cd Steam-Log-Collector

# Create build directory
mkdir build
cd build

# Generate Visual Studio solution
cmake ..

# Build the project
cmake --build . --config Release

# Executables will be in: build\Release\
# Run the GUI
.\Release\steam-log-collector-gui.exe

# Or run the CLI
.\Release\steam-log-collector-cli.exe <game_name>
```

#### Windows (MinGW/Clang)

```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build . --config Release
```

#### macOS

```bash
# Clone the repository
git clone <repository-url>
cd Steam-Log-Collector

# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake ..

# Build the project
make -j$(sysctl -n hw.ncpu)

# Run the GUI
./steam-log-collector-gui

# Or run the CLI
./steam-log-collector-cli <game_name>
```

#### Linux

```bash
# Clone the repository
git clone <repository-url>
cd Steam-Log-Collector

# Create build directory
mkdir build
cd build

# Generate Makefiles
cmake ..

# Build the project
make -j$(nproc)

# Run the GUI
./steam-log-collector-gui

# Or run the CLI
./steam-log-collector-cli <game_name>
```

### Build Options

#### Building CLI Only (No GUI)

To build only the command-line interface without GUI dependencies:

```bash
cmake -DBUILD_GUI=OFF ..
make
```

This is useful for servers or systems without graphical display capabilities.

## Usage

### GUI Application

Simply launch the executable:

- **Windows**: `steam-log-collector-gui.exe`
- **macOS/Linux**: `./steam-log-collector-gui`

The GUI provides an intuitive interface to:

1. Automatically detect or manually select your Steam directory
2. Browse your installed games
3. View available log files
4. Preview log contents
5. Copy logs to a destination folder

### CLI Application

#### List all installed games:

```bash
steam-log-collector-cli --list
```

#### Collect logs for a specific game:

```bash
steam-log-collector-cli "Game Name"
```

#### Specify a custom Steam directory:

```bash
steam-log-collector-cli "Game Name" "/path/to/steam"
```

#### Examples:

```bash
# Windows
steam-log-collector-cli.exe "Counter-Strike 2" "C:\Program Files (x86)\Steam"

# macOS
./steam-log-collector-cli "Dota 2" "/Users/username/Library/Application Support/Steam"

# Linux
./steam-log-collector-cli "Team Fortress 2" "~/.steam/steam"
```

## Project Structure

```
Steam-Log-Collector/
├── CMakeLists.txt          # CMake build configuration
├── LICENSE                 # MIT License
├── README.md               # This file
├── include/                # Header files
│   ├── colors.hpp          # Color definitions for GUI
│   ├── fonts.hpp           # Font configuration
│   ├── logger.hpp          # Logging utilities
│   ├── steam-utils.hpp     # Steam directory and game utilities
│   ├── theme.hpp           # GUI theme settings
│   └── ui_widgets.hpp      # Custom ImGui widgets
├── src/                    # Source files
│   ├── logger.cpp          # Logger implementation
│   ├── main.cpp            # CLI entry point
│   ├── main_gui.cpp        # GUI entry point
│   └── steam-utils.cpp     # Steam utilities implementation
└── build/                  # Build output directory (generated)
```

## Troubleshooting

### Steam Directory Not Found

If the tool cannot automatically detect your Steam installation:

- **Windows**: Default is `C:\Program Files (x86)\Steam`
- **macOS**: Default is `~/Library/Application Support/Steam`
- **Linux**: Check `~/.steam/steam` or `~/.local/share/Steam`

Use the manual directory selection option in the GUI or provide the path via CLI.

### Build Errors

#### CMake Version Error

Ensure CMake 4.0.3 or higher is installed:

```bash
cmake --version
```

#### Missing OpenGL (Linux)

Install OpenGL development packages:

```bash
sudo apt install libgl1-mesa-dev  # Debian/Ubuntu
sudo dnf install mesa-libGL-devel  # Fedora
```

#### Missing X11 Libraries (Linux)

```bash
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Author

Dwayne Mack - 2025

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Acknowledgments

- [Dear ImGui](https://github.com/ocornut/imgui) for the excellent immediate mode GUI framework
- [GLFW](https://github.com/glfw/glfw) for cross-platform window and input handling
- The Steam community for inspiration and support
