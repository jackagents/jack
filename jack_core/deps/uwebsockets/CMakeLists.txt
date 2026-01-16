FetchContent_Declare(uWebSockets
    GIT_REPOSITORY https://github.com/jackagents/uWebSockets
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG ${SHA1_uwebsockets}
)
FetchContent_MakeAvailable(uWebSockets)
