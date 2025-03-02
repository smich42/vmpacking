#ifndef VMP_TREEINSTANCELOADER_H
#define VMP_TREEINSTANCELOADER_H

#include <vmp_treeinstance.h>

#include <json.hpp>
#include <set>
#include <vector>

namespace vmp
{

class TreeInstanceLoader
{
  public:
    explicit TreeInstanceLoader(std::string directory, std::string capacityFieldName = "capacity",
                                std::string guestPagesFieldName = "guest_pages",
                                std::string pagesFieldName = "pages",
                                std::string childrenFieldName = "children");

    [[nodiscard]]
    std::vector<TreeInstance> load(int maxInstances = -1);

    ~TreeInstanceLoader() = default;

  private:
    const std::string directory;

    const std::string capacityFieldName;
    const std::string guestPagesFieldName;
    const std::string pagesFieldName;
    const std::string childrenFieldName;

    std::set<std::filesystem::path> paths;
    std::unordered_map<std::filesystem::path, int> processedInstances;

    [[nodiscard]] std::shared_ptr<Guest> parseGuest(const nlohmann::json &nodeJson) const;
    void parseChildren(TreeInstance &instance, size_t parent, const nlohmann::json &nodeJson) const;
};

}  // namespace vmp

#endif  // VMP_TREEINSTANCELOADER_H
