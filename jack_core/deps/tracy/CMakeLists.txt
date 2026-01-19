FetchContent_Declare(Tracy
    GIT_REPOSITORY https://github.com/jackagents/tracy
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG ${SHA1_Tracy}
)
FetchContent_MakeAvailable(Tracy)
