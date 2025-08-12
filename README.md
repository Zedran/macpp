# macpp

macpp is a tool that can lookup hardware manufacturer information by MAC address of a device or by vendor name.

Once the application's database is created, the Internet connection is not required to operate it. In fact, you can create and update the database from a local, pre-downloaded file, so you can work completely offline.

**Data source:** [maclookup.app](https://maclookup.app/downloads/csv-database).

## Subcommands

| Subcommand       | Description                            |
|:-----------------|:---------------------------------------|
| `addr`           | Search by MAC address                  |
| `name`           | Search by vendor name                  |
| `update`         | Update / initialize cache              |

## Optional arguments

| Option           | Description                            |
|:-----------------|:---------------------------------------|
| `-f` `--file`    | Use a local CSV file for `update`      |
| `-h` `--help`    | Display brief usage information.       |
| `-v` `--version` | Display version information.           |

## Installation

### Release

| Package                  | Tested on              | Note                                             |
|:-------------------------|:----------------------:|:-------------------------------------------------|
| **DEB**                  | Debian 12<br>Debian 13 | Should work on all Debian derivatives.           |
| **RPM**                  | Fedora 41<br>Fedora 42 | Unsigned.                                        |
| **Windows<br>installer** | Windows 10             | Unsigned. May trigger Windows Defender warnings. |

### Building from source

* [Linux](doc/build_linux.md)
* [Windows](doc/build_windows.md)

## License

This software is available under MIT License.
