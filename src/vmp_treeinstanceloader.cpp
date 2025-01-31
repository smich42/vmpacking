#include <vmp_treeinstanceloader.h>

#include <cassert>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

namespace vmp
{

TreeInstanceLoader::TreeInstanceLoader(std::string directory) : directory(std::move(directory)) {}

std::vector<TreeInstance> TreeInstanceLoader::load(int maxInstances,
                                                   const std::string &capacityFieldName,
                                                   const std::string &guestPagesFieldName,
                                                   const std::string &pagesFieldName,
                                                   const std::string &childrenFieldName) const
{
    namespace fs = std::filesystem;

    std::vector<TreeInstance> instances;
    instances.reserve(maxInstances);

    std::function parseGuest = [&](const json &nodeJson) -> std::shared_ptr<const Guest> {
        if (!nodeJson.contains(guestPagesFieldName)) {
            return nullptr;
        }
        return std::make_shared<Guest>(std::unordered_set<int>(
            nodeJson[guestPagesFieldName].begin(), nodeJson[guestPagesFieldName].end()));
    };

    std::function<void(TreeInstance &, size_t, const json &)> addChildren =
        [&](auto &tree, const size_t parent, const json &nodeJson) {
            for (const auto &childJson : nodeJson[childrenFieldName]) {
                const std::unordered_set<int> childPages =
                    childJson[pagesFieldName].get<std::unordered_set<int>>();

                const size_t child = childJson.contains(guestPagesFieldName)
                                         ? tree.addLeaf(parent, parseGuest(childJson), childPages)
                                         : tree.addInner(parent, childPages);

                if (childJson.contains(childrenFieldName)) {
                    addChildren(tree, child, childJson);
                }
            }
        };

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

            std::optional<TreeInstance> tree;

            if (rootGuest == nullptr) {
                tree.emplace(capacity, rootPages);
            }
            else {
                tree.emplace(capacity, rootPages, rootGuest);
            }

            if (rootNodeJson.contains(childrenFieldName)) {
                addChildren(*tree, TreeInstance::getRootNode(), rootNodeJson);
            }

            instances.push_back(std::move(*tree));
            if (instances.size() == maxInstances) {
                return instances;
            }
        }
    }

    return instances;
}

}  // namespace vmp