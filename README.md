# macpp

CLI tool for MAC address lookup. It uses the database provided by [MAC Address Lookup](https://maclookup.app/downloads/csv-database) as its data source.

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

**Building from source:**

* [Linux](doc/build_linux.md)
* [Windows](doc/build_windows.md)

## License

This software is available under MIT License.
