option(TRACY_ENABLE "Enable profiling with Tracy" ${JACK_WITH_TRACY})
option(TRACY_ONLY_LOCALHOST "Enable discovery of Tracy clients exclusively via localhost" ${JACK_WITH_TRACY})

FetchContent_Declare(Tracy
    GIT_REPOSITORY https://github.com/jackagents/tracy
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG ${SHA1_Tracy}
)
FetchContent_MakeAvailable(Tracy)
