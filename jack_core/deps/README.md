# deps folder

## concurrentqueue
git remote add -f concurrentqueue_origin git@gitlab.aosgrp.net:aos/3rd/concurrentqueue.git
git subtree add --prefix src/jack/jack_core/deps/concurrentqueue concurrentqueue_origin feature/mw/fix_includes --squash

## cpptrace
git subtree add --prefix src/jack/jack_core/deps/cpptrace git@gitlab.aosgrp.net:aos/3rd/cpptrace.git v0.2.1 --squash

## date
git subtree add --prefix src/jack/jack_core/deps/date git@gitlab.aosgrp.net:aos/3rd/date.git v3.0.1 --squash

## fmt
git subtree add --prefix src/jack/jack_core/deps/fmt git@gitlab.aosgrp.net:aos/3rd/fmt.git maint/aos/2023_10 --squash

## googletests
git subtree add --prefix src/jack/jack_core/deps/googletest git@gitlab.aosgrp.net:aos/3rd/googletest.git feature/mw/upstream --squash

## rti
rti needs to be installed as a dependency

## Updating a dependency

## Adding a new dependency

