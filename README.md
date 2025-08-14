# duef
A very basic cross-platform tool for uncompressing `.uecrash` files. This utility tries to make it easier to work with crashes from the packaged game builds.

`.uecrash` file is a zlib compressed file, created by `CrashReportClient` for sending crash data to `DataRouter` url (which can be a basic webhook).

a quick usage would be.
```powershell
explorer (duef -f ./CrashReport.uecrash)
```

duef uncompresses this files and places them on the file system.
On windows this place is in `%LocalAppdata%\duef` while in other systems it is in `~/.duef` directory.

## Usage

### Uncompress
Uncompressing is actually pretty basic you should just give the file with f option.
```powershell
duef -f ./CrashReport.uecrash
```
or for opening the directory instantly on windows
```powershell
explorer (duef -f ./CrashReport.uecrash)
```
If you dont enter a file parameter duef fallsback to the `CrashReport.uecrash`.

Which means if you have a `CrashReport.uecrash` in your current working directory you can basically do
```powershell
explorer (duef)
```

Also you can pipe the output of this command because the directory that is created will be written to the standard output.
If duef receives `i` option, instead of giving back the path of the directory duef will write the absolute path of the created files.
```powershell
code (duef -if ./Crashreport.uecrash)
# i don't know, whatever program that you want to use.
```
---
```powershell
duef -vif ./Crashreport.uecrash
```
Uses verbose printing (-v option). Will print details about file, compressed files and process to the stderr.

### Cleanup
duef doesn't magically understand when you are done with the files and remove them, instead you should run command below periodically (per week would probably be enough or after you are done with each crash) to remove collected crashes.
```powershell
duef --clean
```

## Building

The project supports both CMake and Make for building:

### Using Make (recommended for simplicity)
```bash
make          # Build duef
make clean    # Clean build artifacts
make help     # Show available targets
```

### Using CMake
```bash
mkdir build && cd build
cmake -DZLIB_BUILD_EXAMPLES=OFF ..
make
```
