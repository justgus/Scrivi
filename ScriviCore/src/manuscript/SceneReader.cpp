#include "manuscript/SceneReader.hpp"

#include "util/PathUtils.hpp"

namespace scrivi::manuscript {

SceneReader::SceneReader(CoreServices& services)
    : services_(services) {}

Result<Utf8Text> SceneReader::readContent(const AbsolutePath& projectRoot,
                                          const RelativePath& contentPath) const
{
    auto absPath = util::join(projectRoot, contentPath);
    return services_.fileSystem->readTextFile(absPath);
}

} // namespace scrivi::manuscript
