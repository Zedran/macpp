% MACPP(1)
%
% January 2025

# NAME

**macpp** - a simple tool for MAC address lookup.

# SYNOPSIS

**macpp** \[**-a** *mac-address*]  
**macpp** \[**-n** *vendor-name*]  
**macpp** \[**-u**] \[**-f** *path*]

# DESCRIPTION

**macpp** is a tool for identifying hardware manufacturers by MAC prefix. This software uses the database provided by [MAC Address Lookup](https://maclookup.app) as its data source.

**-a**, **\--addr**
: Search by MAC address. Specifying a complete address is not required, but it cannot be shorter than 6 characters. Colon separators are allowed, but not required.

**-f**, **\--file**
: Provide path to a local CSV file for **-u**. It must conform to the format of the file provided by maclookup.app.

**-h**, **\--help**
: Display brief usage information and exit.

**-n**, **\--name**
: Search by vendor name. Case insensitive.

**-u**, **\--update**
: Update vendor database and exit. By itself, it performs the online update, but a path to a local file may be provided with **-f**. The update option is provided for convenience, but it is not recommended to generate unnecessary traffic with frequent updates. Database created during the initial run should be sufficient for most users.

**-v**, **\--version**
: Display version information and exit.

# EXAMPLES

## Searching by MAC address

macpp \--addr 000000  
macpp \--addr C0:FB:F9:01:23:45

## Searching by vendor name

macpp \--name xerox  
macpp \--name "xerox corporation"

## Updating vendor database

macpp \--update  
macpp \--update \--file local-file.csv

# REPORTING BUGS

If you experience any problems with macpp or have a suggestion, feel free to open new issue on the project's issue tracker:

https://github.com/Zedran/macpp/issues

# AUTHOR

Written by Wojciech Głąb.
