# jack-event-protocol

A description of the events/messages that the JACK engine and participating
entities in the BDI process on the network use. By utilising this event schema,
third party applications can craft messages that are understood by other
applications in the JACK protocol.

JACK currently supports DDS as a remote networking technology to execute the BDI
process remotely, however, this repository exposes an adapter interface that
JACK and third party applications can plug into integrate to allow communication
over any bespoke network.

By fulfilling the interface and attaching JACK to the newly crafted adapter,
integral BDI events will be sent through the adapter which can be used to listen
and participate in the BDI process.

## Build

This repository exposes a CMake target `jack-event-protocol` that is available
after an `add_subdirectory(event-protocol)` command in CMake. The target can be
linked to using the `target_link_libraries(<your library> PRIVATE jack-event-protocol)`
command.

## Release

This repository is packaged as part of jack-redist and the CMake configuration
scripts are configured from the `/cmake` folder to produce a `.cmake` file that
is usable from `find_package`.
