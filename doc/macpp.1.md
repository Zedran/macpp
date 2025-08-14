% MACPP(1)
%
% August 2025

# NAME

**macpp** - tool for MAC address lookup.

# SYNOPSIS

**macpp** \[-h | \--help] \[-v | \--version] \[-o | \--out-format FORMAT] \<command> \[args]

# DESCRIPTION

**macpp** is a tool for identifying hardware manufacturers by MAC prefix. This software uses the database provided by [MAC Address Lookup](https://maclookup.app) as its data source.

## SUBCOMMANDS

**addr**
: Search by MAC address. Specifying a complete address is not required, but it cannot be shorter than 6 characters. Colon separators are allowed, but not required.

**name**
: Search by vendor name. Case insensitive.

**update**
: Update vendor database and exit. By itself, it performs the online update, but a path to a local file may be provided with **--file**. The update option is provided for convenience, but it is not recommended to generate unnecessary traffic with frequent updates. Database created initially should be sufficient for most users.

## OPTIONAL ARGUMENTS

**-f**, **\--file**
: Provide path to a local CSV file for the **update** subcommand. It must conform with the format of the file provided by maclookup.app.

**-h**, **\--help**
: Display brief usage information and exit.

**-o**, **\--out-format**
: Set display format for the results of **addr** and **name** subcommands. Available options are: **csv** - comma-separated values, **json** - JSON format (list of dictionaries) and **regular** - default, human-readable format.

**-v**, **\--version**
: Display version information and exit.

# EXAMPLES

## Searching by MAC address

macpp addr 000000  
macpp addr C0:FB:F9:01:23:45  
macpp -o csv addr 00:00:00

## Searching by vendor name

macpp name xerox  
macpp name "xerox corporation"  
macpp \--out-format name xerox

## Updating vendor database

macpp update  
macpp update \--file local-file.csv

# REPORTING BUGS

If you experience any problems with macpp or have a suggestion, feel free to open new issue on the project's issue tracker:

https://github.com/Zedran/macpp/issues

# AUTHOR

Written by Wojciech Głąb.
