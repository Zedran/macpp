# Building macpp on Windows

Tested on Windows 10 with MSVC 14.42 (cl 19.42), provided by Visual Studio 2022.

## Prerequisites

* CMake
* Git
* MSVC with support for C++20.
* VCPKG for managing dependencies - you can find instructions on how to set it up [here](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg).

If you are using Visual Studio, you can install all of those components through Visual Studio Installer.

**Environmental variables**

1. Add your CMake installation directory to `PATH`.
2. Add your Git installation directory to `PATH`
3. Add a new variable `VCPKG_ROOT` pointing to your VCPKG installation directory.
4. Add `%VCPKG_ROOT%` entry to `PATH`.

## Steps

Clone the repository:

```ps1
git clone https://github.com/Zedran/macpp.git
```

Open Powershell or Command Prompt inside the project directory and run the following instructions:

```ps1
mkdir build
cd build

cmake ..
cmake --build . --config Release
```
