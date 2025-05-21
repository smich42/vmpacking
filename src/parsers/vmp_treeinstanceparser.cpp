#include <vmp_treeinstanceparser.h>

#include <cassert>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

namespace vmp
{

TreeInstanceParser::TreeInstanceParser(std::string directory, std::string capacityName,
                                       std::string guestPagesName, std::string pagesName,
                                       std::string childrenName)
    : directory(std::move(directory)),
      capacityName(std::move(capacityName)),
      guestPagesName(std::move(guestPagesName)),
      pagesName(std::move(pagesName)),
      childrenName(std::move(childrenName))
{
}

std::shared_ptr<Guest> TreeInstanceParser::parseGuest(const json &nodeJson) const
{
    if (!nodeJson.contains(guestPagesName)) {
        return nullptr;
    }
    return std::make_shared<Guest>(
        std::unordered_set<int>(nodeJson[guestPagesName].begin(), nodeJson[guestPagesName].end()));
}

void TreeInstanceParser::parseChildren(TreeInstance &instance, const size_t parent,
                                       const json &nodeJson) const
{
    for (const auto &childJson : nodeJson[childrenName]) {
        const std::unordered_set<int> childPages =
            childJson[pagesName].get<std::unordered_set<int>>();

        const size_t child = childJson.contains(guestPagesName)
                                 ? instance.addLeaf(parent, parseGuest(childJson), childPages)
                                 : instance.addInner(parent, childPages);

        if (childJson.contains(childrenName)) {
            parseChildren(instance, child, childJson);
        }
    }
};

std::vector<TreeInstance> TreeInstanceParser::load(const int maxInstances)
{
    namespace fs = std::filesystem;

    std::vector<TreeInstance> instances;
    instances.reserve(maxInstances);

    for (const auto &directoryEntry : fs::directory_iterator(directory)) {
        if (directoryEntry.path().extension() == ".json") {
            paths.emplace(directoryEntry);
        }
    }

    while (!paths.empty()) {
        const auto path = *paths.begin();
        paths.erase(path);

        std::ifstream file(path);
        assert(file.is_open());

        if (!processedInstances.contains(path)) {
            processedInstances[path] = 0;
        }

        const auto rootNodesJson = json::parse(file);

        for (int i = processedInstances[path]; i < rootNodesJson.size(); ++i) {
            const auto &rootNodeJson = rootNodesJson[i];
            assert(rootNodeJson.contains(capacityName));

            const size_t capacity = rootNodeJson[capacityName].get<size_t>();
            const std::unordered_set<int> rootPages =
                rootNodeJson[pagesName].get<std::unordered_set<int>>();
            const auto rootGuest = parseGuest(rootNodeJson);

            TreeInstance instance = rootGuest == nullptr
                                        ? TreeInstance(capacity, rootPages)
                                        : TreeInstance(capacity, rootPages, rootGuest);

            if (rootNodeJson.contains(childrenName)) {
                parseChildren(instance, TreeInstance::getRootNode(), rootNodeJson);
            }

            instances.push_back(std::move(instance));
            ++processedInstances[path];

            if (instances.size() == maxInstances) {
                return instances;
            }
        }
    }

    return instances;
}

}  // namespace vmp