# macpp

macpp is a tool that can lookup hardware manufacturer information by MAC address of a device or by vendor name.

Once the application's database is created, the Internet connection is not required to operate it. In fact, you can create and update the database from a local, pre-downloaded file, so you can work completely offline.

**Data source:** [maclookup.app](https://maclookup.app/downloads/csv-database).

## Subcommands

| Subcommand       | Description                            |
|:-----------------|:---------------------------------------|
| `addr`           | Search by MAC address                  |
| `export`         | Export all records from the database   |
| `name`           | Search by vendor name                  |
| `update`         | Update / initialize vendor database    |

Make sure to run `update` after installation to create vendor database.

## Optional arguments

| Option              | Description                                                                    |
|:--------------------|:-------------------------------------------------------------------------------|
| `-f` `--file`       | Use a local CSV file for `update`                                              |
| `-h` `--help`       | Display brief usage information.                                               |
| `-o` `--out-format` | Set display format for the results of `addr`, `export` and `name` subcommands. |
| `-v` `--version`    | Display version information.                                                   |

Available display formats:

* `csv` - comma separated values
* `json` - JSON (list of dictionaries)
* `regular` - default, human-readable format
* `xml` - Cisco PI vendorMacs.xml

## Examples

### Searching by address

```bash
# Search by prefix
macpp addr 000000

# Search by full MAC address (separators are optional)
macpp addr C0:FB:F9:01:23:45

# Display results in CSV format
macpp -o csv addr 00:00:00
```

### Searching by name

```bash
macpp name xerox

# If vendor name contains space or special characters,
# wrap it in quotes
macpp name "xerox corporation"

# Display results in JSON format
macpp --out-format json name xerox
```

### Exporting records

```bash
# Export records to a JSON file.
macpp -o json export > vendors.json
```

### Updating vendor database

```bash
# Perform online update
macpp update

# Use a local CSV file for update
macpp update --file local-file.csv
```

## Installation

Download the package of your choice from the [Releases](https://github.com/Zedran/macpp/releases) page.

| Package                  | Tested on              | Note                                             |
|:-------------------------|:----------------------:|:-------------------------------------------------|
| **DEB**                  | Debian 12<br>Debian 13 | Should work on all Debian derivatives.           |
| **RPM**                  | Fedora 41<br>Fedora 42 | Unsigned.                                        |
| **Windows<br>installer** | Windows 10             | Unsigned. May trigger Windows Defender warnings. |

### Linux

#### Debian-based

```bash
apt install /path/to/macpp-<version>.deb
```

#### Fedora

```bash
dnf install /path/to/macpp-<version>.rpm
```

#### Other

For other distributions, you will have to [build macpp from source](doc/build_linux.md).

### Windows

Run the installer. Make sure to select the option to add the installation directory to PATH.

### Building from source

* [Linux](doc/build_linux.md)
* [Windows](doc/build_windows.md)
* macOS - not supported

## License

This software is available under MIT License.
