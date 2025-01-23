# deps folder

Everything in the folder is sourced in using git subtree as below:
(Please update these if you pull an update)

## concurrentqueue
git remote add -f concurrentqueue_origin git@gitlab.aosgrp.net:aos/3rd/concurrentqueue.git
git subtree add --prefix src/jack/jack_core/deps/concurrentqueue concurrentqueue_origin feature/mw/fix_includes --squash

## cpptoml
git subtree add --prefix src/jack/jack_core/deps/cpptoml git@gitlab.aosgrp.net:aos/3rd/cpptoml.git v0.1.1 --squash

## cpptrace
git subtree add --prefix src/jack/jack_core/deps/cpptrace git@gitlab.aosgrp.net:aos/3rd/cpptrace.git v0.2.1 --squash

## date
git subtree add --prefix src/jack/jack_core/deps/date git@gitlab.aosgrp.net:aos/3rd/date.git v3.0.1 --squash

## cyclonedds
removed for now
also the patches should be pushed over to internal repo and tagged

## flatbuffers
not using anymore?

## fmt
git subtree add --prefix src/jack/jack_core/deps/fmt git@gitlab.aosgrp.net:aos/3rd/fmt.git maint/aos/2023_10 --squash

## googletests
git subtree add --prefix src/jack/jack_core/deps/googletest git@gitlab.aosgrp.net:aos/3rd/googletest.git feature/mw/upstream --squash

## iceoryx
removed for now
required for cyclonedds?

## libuv
not used? uwebsockets?
WITH_LIBUV=1 ??
yeah I don't think we even use this

## rti
rti needs to be installed as a dependency

## tracy
git subtree add --prefix src/jack/jack_core/deps/tracy git@gitlab.aosgrp.net:aos/3rd/tracy.git v0.9.1 --squash

## uwebsockets
git subtree add --prefix src/jack/jack_core/deps/uwebsockets git@gitlab.aosgrp.net:aos/3rd/uwebsockets_20_35_0.git feature/mw/fix_includes --squash

TODO apply other setting from the original CMakeLists.txt

## zlib
probably broken the windows build 
we don't vendor this for linux - maybe we should

## Updating a dependency
git subtree pull etc.

## Adding a new dependency

example:

called from the base of aewcf
use --squash
use a tag if possible

```
git subtree add --prefix src/jack/jack_core/deps/cpptoml git@gitlab.aosgrp.net:aos/3rd/cpptoml.git v0.1.1 --squash
```

update this file with the tag/branch used