# macpp

A simple tool for MAC address lookup. It uses the database provided by [MAC Address Lookup](https://maclookup.app/downloads/csv-database) as its data source.

## Searching

| Option     | Description                            |
|:-----------|:---------------------------------------|
| `--addr`   | Search by MAC address                  |
| `--name`   | Search by vendor name                  |

## Updating cache

When search is requested for the first time, the application's cache is created. It can be updated (or created) using the options below.

| Option     | Description                            |
|:-----------|:---------------------------------------|
| `--file`   | Use a local CSV file during operation  |
| `--update` | Update cache                           |

## License

This software is available under MIT License.
