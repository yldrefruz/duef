# duef
A very basic tool for uncompressing `.uecrash` files. This utility tries to make it easier to work with crashes from the packaged game builds.

`.uecrash` file is a zlib compressed file, created by `CrashReportClient` for sending crash data to `DataRouter` url (which can be a basic webhook).

a quick usage would be.
```bash
duef -f ./CrashReport.uecrash | explorer
```

duef uncompresses this files and places them on the file system.
On windows this place is in `%LocalAppdata%\duef` while in other systems it is in `~/.duef` directory.

## Usage

### Uncompress

Uncompressing is actually pretty basic you should just give the file with f option.
```bash
duef -f ./CrashReport.uecrash
```
or for opening the directory instantly on windows
```bash
duef -f ./CrashReport.uecrash | explorer
```
If you dont enter a file parameter duef fallsback to the `CrashReport.uecrash`.

Which means if you have a `CrashReport.uecrash` in your current working directory you can basically do
```bash
duef | explorer
```

Also you can pipe the output of this command because the directory that is created will be written to the standard output.
If duef receives `i` option instead of giving back the name of the directory, will write the absolute path of the created files.
```bash
duef -if ./Crashreport.uecrash | code
# i don't know whatever program you want to use.
```

### Cleanup
duef doesn't magically understand when you are done with the files and remove them, instead you should run command below periodically (per week would probably be enough or after you are done with each crash) to remove collected crashes.
```bash
duef --clear
```
