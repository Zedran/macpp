# Building macpp on Linux

Tested with GCC 14.2.

## Prerequisites

### Build

* CMake
* Git
* GCC with support for C++20 features
* CURL
* sqlite3
* pandoc and gzip for generating the manual

### Testing

* sqlite3 executable
* LCOV for generating coverage data

## Steps

Clone the repository:

```bash
git clone https://github.com/Zedran/macpp.git
```

### Build

Run the following instructions inside the project's root directory:

```bash
mkdir build
cd build

cmake ..
make
```

The following commands can be used to install and uninstall the application:

```bash
make install
make uninstall
```

### Testing

```bash
mkdir build
cd build

cmake -DMAKE_TESTS=ON ..
make
./t
```

### CMake options

| Option         | Description                                                   |
|:---------------|:--------------------------------------------------------------|
| `-DBUNDLE`     | Compile SQLite into the application and link CURL statically* |
| `-DMAKE_TESTS` | Build test binary                                             |
| `-DMAKE_COVER` | Generate coverage report target (`make cover`)                |
| `-DMAKE_MAN`   | Generate a manual for the application                         |

\* Bundled build is slightly faster and uses significantly less memory. CURL still requires its dynamically-linked dependencies.
