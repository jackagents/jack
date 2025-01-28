FetchContent_Declare(nlohmann
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG ${SHA1_nlohmann}
)
FetchContent_MakeAvailable(nlohmann)
