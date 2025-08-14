# duef - Unreal Engine Crash File Decompressor

Always follow these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

duef is a cross-platform C tool for decompressing Unreal Engine crash files (.uecrash). It extracts zlib-compressed crash data and places extracted files in the filesystem for analysis.

## Working Effectively

### Bootstrap and Build
Run these commands in sequence to build the project:

```bash
# Navigate to project root
cd /home/runner/work/duef/duef

# Create build directory and configure with CMake
mkdir build && cd build
cmake -DZLIB_BUILD_EXAMPLES=OFF ..

# Build the project - NEVER CANCEL, takes ~2-3 seconds. Set timeout to 30+ seconds.
make
```

**CRITICAL BUILD NOTES:**
- **NEVER CANCEL** the build - it completes in 2-3 seconds but always set timeout to 30+ seconds minimum.
- **MUST use `-DZLIB_BUILD_EXAMPLES=OFF`** - without this flag, CMake fails because zlib test files are missing.
- Build produces warnings about format truncation - this is expected and does not prevent successful compilation.
- Creates `duef` executable in the `build/` directory.

### Alternative Build Commands
If the above fails, try these commands step by step:
```bash
# Clean and restart
rm -rf build
mkdir build && cd build
cmake -DZLIB_BUILD_EXAMPLES=OFF .. && make
```

### Testing the Build
Verify the build works correctly:
```bash
cd build
ls -la duef                    # Should show executable file (~82KB)
./duef --clean                 # Should succeed (cleans ~/.duef directory)
./duef -v                      # Should fail with "Error opening input file: CrashFile.uecrash" - this is expected
```

## Code Quality and Validation

### Run Static Analysis
**ALWAYS run clang-tidy before committing changes** - takes ~5 seconds:
```bash
# NEVER CANCEL - takes 5 seconds. Set timeout to 60+ seconds.
clang-tidy duef.c -- -I./zlib-1.3.1 -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS
```

**Known Issues in Code:**
- Memory management bug in `UECrashFile_Destroy` function (use-after-free)
- Multiple security warnings about fprintf usage
- Format truncation warnings in snprintf usage

### Validation Scenarios
After making changes, test these scenarios:
```bash
cd build

# Test command line options
./duef --clean                 # Should clean ~/.duef directory
./duef -v                      # Should show verbose error about missing file
./duef --file nonexistent.uecrash  # Should fail gracefully
./duef -vif nonexistent.uecrash    # Should show verbose output and fail gracefully

# Verify build still works
make clean && make             # Should rebuild successfully
```

## Project Structure

### Key Files
- `duef.c` - Main source file with argument parsing and decompression logic
- `duef.h` - Function declarations and interface definitions  
- `duef_types.h` - Data structures and parsing functions for .uecrash format
- `duef_printing.h` - Verbose and debug printing utilities
- `CMakeLists.txt` - Build configuration (requires CMake 3.29+)
- `zlib-1.3.1/` - Bundled zlib compression library dependency

### Repository Root Structure
```
.
├── README.md              # User documentation and usage examples
├── CMakeLists.txt         # Build configuration
├── duef.c                 # Main source file
├── duef.h                 # Header declarations
├── duef_types.h           # Data structure definitions
├── duef_printing.h        # Printing utilities
├── zlib-1.3.1/           # Bundled zlib library
├── build/                 # Build output directory (created by cmake)
└── .gitignore            # Excludes build/, test/, .vscode/, compile_commands.json
```

## Usage and Functionality

### Command Line Options
- `-v` or `--verbose` - Enable verbose output to stderr
- `-f <file>` or `--file <file>` - Specify .uecrash file to process  
- `-i` - Print individual file paths instead of directory path
- `--clean` - Remove all extracted files from ~/.duef directory

### Default Behavior
- Without `-f` option, looks for `CrashFile.uecrash` in current directory
- Extracts files to `~/.duef/<directory_name>/` on Unix systems
- Outputs directory path to stdout (or individual file paths with `-i`)

### Expected File Format
duef expects .uecrash files with this structure:
- File header: version (3 bytes), directory name, file name, uncompressed size, file count
- Multiple embedded files: file index, file name, file size, file data

## Common Development Tasks

### When Modifying duef.c
1. Build and test basic functionality
2. Run clang-tidy to check for new issues
3. Test all command line options
4. Verify file extraction behavior (even without real .uecrash files)

### When Modifying Build System
1. Test both failing case: `cmake ..` (should fail on zlib examples)
2. Test working case: `cmake -DZLIB_BUILD_EXAMPLES=OFF ..`
3. Verify clean build: `rm -rf build && mkdir build && cd build && cmake -DZLIB_BUILD_EXAMPLES=OFF .. && make`

### Debugging Build Issues
- If CMake configuration fails on zlib examples, ensure `-DZLIB_BUILD_EXAMPLES=OFF` is used
- If compilation fails on `MAX_PATH`, check cross-platform compatibility in duef.c
- If linker fails, verify zlib static library is built first (should be automatic)

## Dependencies and Requirements

### System Requirements
- CMake 3.29 or higher
- GCC or compatible C compiler
- Standard C libraries (stdio, stdlib, string, etc.)

### Bundled Dependencies
- zlib 1.3.1 (included in `zlib-1.3.1/` directory)
- No external package installation required

### Optional Tools
- `clang-tidy` for static analysis (recommended)
- Any text editor (no specific IDE requirements)

## Timing Expectations

- **Full build from scratch**: 2-3 seconds - NEVER CANCEL, set 30+ second timeout
- **Incremental build**: <1 second - NEVER CANCEL, set 30+ second timeout  
- **clang-tidy analysis**: ~5 seconds - NEVER CANCEL, set 60+ second timeout
- **Clean and rebuild**: 2-3 seconds - NEVER CANCEL, set 30+ second timeout

## Critical Reminders

- **ALWAYS use** `-DZLIB_BUILD_EXAMPLES=OFF` with cmake
- **NEVER CANCEL** builds even though they're fast - always set appropriate timeouts
- **ALWAYS run** clang-tidy before committing to catch code quality issues
- **REMEMBER** the code has known memory management bugs that should be fixed
- **TEST** all command line options after making changes, even if you don't have .uecrash files