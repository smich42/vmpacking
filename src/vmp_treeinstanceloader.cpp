#include <vmp_treeinstanceloader.h>

#include <cassert>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

namespace vmp
{

TreeInstanceLoader::TreeInstanceLoader(std::string directory, std::string capacityFieldName,
                                       std::string guestPagesFieldName, std::string pagesFieldName,
                                       std::string childrenFieldName)
    : directory(std::move(directory)),
      capacityFieldName(std::move(capacityFieldName)),
      guestPagesFieldName(std::move(guestPagesFieldName)),
      pagesFieldName(std::move(pagesFieldName)),
      childrenFieldName(std::move(childrenFieldName))
{
}

std::shared_ptr<Guest> TreeInstanceLoader::parseGuest(const json &nodeJson) const
{
    if (!nodeJson.contains(guestPagesFieldName)) {
        return nullptr;
    }
    return std::make_shared<Guest>(std::unordered_set<int>(nodeJson[guestPagesFieldName].begin(),
                                                           nodeJson[guestPagesFieldName].end()));
}

void TreeInstanceLoader::parseChildren(TreeInstance &instance, const size_t parent,
                                       const json &nodeJson) const
{
    for (const auto &childJson : nodeJson[childrenFieldName]) {
        const std::unordered_set<int> childPages =
            childJson[pagesFieldName].get<std::unordered_set<int>>();

        const size_t child = childJson.contains(guestPagesFieldName)
                                 ? instance.addLeaf(parent, parseGuest(childJson), childPages)
                                 : instance.addInner(parent, childPages);

        if (childJson.contains(childrenFieldName)) {
            parseChildren(instance, child, childJson);
        }
    }
};

std::vector<TreeInstance> TreeInstanceLoader::load(const int maxInstances) const
{
    namespace fs = std::filesystem;

    std::vector<TreeInstance> instances;
    instances.reserve(maxInstances);

    for (const auto &directoryEntry : fs::directory_iterator(directory)) {
        if (directoryEntry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(directoryEntry.path());
        assert(file.is_open());

        for (const auto &rootNodeJson : json::parse(file)) {
            assert(rootNodeJson.contains(capacityFieldName));

            const size_t capacity = rootNodeJson[capacityFieldName].get<size_t>();
            const std::unordered_set<int> rootPages =
                rootNodeJson[pagesFieldName].get<std::unordered_set<int>>();
            const auto rootGuest = parseGuest(rootNodeJson);

            std::optional<TreeInstance> instance;

            if (rootGuest == nullptr) {
                instance.emplace(capacity, rootPages);
            }
            else {
                instance.emplace(capacity, rootPages, rootGuest);
            }

            if (rootNodeJson.contains(childrenFieldName)) {
                parseChildren(*instance, TreeInstance::getRootNode(), rootNodeJson);
            }

            instances.push_back(std::move(*instance));
            if (instances.size() == maxInstances) {
                return instances;
            }
        }
    }

    return instances;
}

}  // namespace vmp