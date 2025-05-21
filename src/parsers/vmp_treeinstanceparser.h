#ifndef VMP_TREEINSTANCELOADER_H
#define VMP_TREEINSTANCELOADER_H

#include <vmp_treeinstance.h>

#include <json.hpp>
#include <set>
#include <vector>

namespace vmp
{

class TreeInstanceParser
{
  public:
    explicit TreeInstanceParser(std::string directory, std::string capacityName = "capacity",
                                std::string guestPagesName = "guest_pages",
                                std::string pagesName = "pages",
                                std::string childrenName = "children");

    [[nodiscard]]
    std::vector<TreeInstance> load(int maxInstances = -1);

    ~TreeInstanceParser() = default;

  private:
    const std::string directory;

    const std::string capacityName;
    const std::string guestPagesName;
    const std::string pagesName;
    const std::string childrenName;

    std::set<std::filesystem::path> paths;
    std::unordered_map<std::filesystem::path, int> processedInstances;

    [[nodiscard]] std::shared_ptr<Guest> parseGuest(const nlohmann::json &nodeJson) const;
    void parseChildren(TreeInstance &instance, size_t parent, const nlohmann::json &nodeJson) const;
};

}  // namespace vmp

#endif  // VMP_TREEINSTANCELOADER_H
