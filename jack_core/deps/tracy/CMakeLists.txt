option(TRACY_ENABLE "Enable profiling with Tracy" ${JACK_WITH_TRACY})
option(TRACY_ONLY_LOCALHOST "Enable discovery of Tracy clients exclusively via localhost" ${JACK_WITH_TRACY})

# Tracy package is already found in deps.cmake via find_package(Tracy CONFIG REQUIRED)
# This directory is kept for additional Tracy configuration if needed
