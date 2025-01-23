#include <string_view>

namespace aos::jack
{
std::string_view version();        ///< X.X.X
unsigned int versionMajor();
unsigned int versionMinor();
unsigned int versionPatch();
unsigned int versionTweak();
std::string_view versionGitHash();
bool versionIsGitDirty();
std::string_view versionDetail();  ///< X.X.X-<hash>-<git dirty>
} // namespace aos::jack
