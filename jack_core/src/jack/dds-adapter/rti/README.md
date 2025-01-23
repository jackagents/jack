# jack-nds-adapter

A sample implementation of an adapter for communicating on the JACK
distribution network protocol utilising the RTI implementation of DDS.

## Build

This repository exposes a CMake target `jack-rti-dds-adapter` that is available
after an `add_subdirectory(dds-adapter)` command in CMake. The target can be
linked to using the `target_link_libraries(<your library> PRIVATE jack-dds-adapter)`
command.

### Build Options

Useful options for building the repository are listed below.

 - `CONNEXTDDS_DIR`

    Set this to the root installation directory of RTI DDS, defaults to
    `/opt/rti_connext_dds-5.3.1`

 - `RTICODEGEN_DIR`

    Set this to the bin directory of RTI DDS, defaults to the bin directory in
    the `CONNEXTDDS_DIR` value.

 - `DDS_ADAPTER_USER_INCLUDE_DIRS (default: "")`

    Set additional include paths intended for making structures available to use
    in the adapter's user config header

 - `DDS_ADAPTER_USER_CONFIG_H: (default: "")`

    Set the path to a header file that will be included at the top of the header
    file. Useful for overriding the macros like the asserts and logs in the
    adapter.

An example of a custom configuration header to override macros is provided below.

```cpp
/// ddsadapterconfig.h
///
/// Customise the behaviour of the DDS-Adapter in this file.
/// For example, to replace the default logging functions with JACK, i.e.
/// Invoke CMake w/ flags
/// cmake -D DDS_ADAPTER_USER_INCLUDE_DIRS "/path/to/jack" -D DDS_ADAPTER_USER_CONFIG_H="path/to/ddsadapterconfig.h" ..

/// Add the compile time options as per below, see the rtiddsadapter.h for all
/// compile time options.

#include <jack>
#define AOS_DDS_ADAPTER_ERROR(fmt) JACK_ERROR(fmt)
#define AOS_DDS_ADAPTER_INFO(fmt) JACK_INFO(fmt)
#define AOS_DDS_ADAPTER_WARNING(fmt) JACK_WARNING(fmt)
```

## Licensing
Applications using the adapter must have a licensed RTI's DDS installation.
In the absence of a license, RTI DDS will fail to start and throw an exception.
The license can be made available to the linking application by setting the
environment variable before launching the application linking the adapter.

- `RTI_LICENSE_FILE`

  Set an absolute path to the license `.dat` file in an environment variable..e

Alternatively set the environment variable,

- `NDDSHOME`

  Set an absolute path to the root installation of DDS where the license is
  placed in a file called `rti_license.dat`

There are further options available if these are not applicable from the
[RTI DDS troubleshooting guide](https://community.rti.com/kb/why-do-i-get-error-rti-data-distribution-service-no-source-license-information)
