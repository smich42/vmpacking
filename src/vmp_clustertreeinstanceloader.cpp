#include <vmp_clustertreeinstanceloader.h>

#include <cassert>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

namespace vmp
{

ClusterTreeInstanceLoader::ClusterTreeInstanceLoader(std::string directory)
    : directory(std::move(directory))
{
}

std::vector<ClusterTreeInstance> ClusterTreeInstanceLoader::load(
    int maxInstances, const std::string &capacityFieldName, const std::string &nodesFieldName,
    const std::string &nodeIdFieldName, const std::string &nodeParentsFieldName,
    const std::string &pagesFieldName, const std::string &guestPagesFieldName,
    const std::string &clusterChildrenFieldName) const
{
    namespace fs = std::filesystem;

    std::vector<ClusterTreeInstance> instances;
    instances.reserve(maxInstances);

    auto parseGuest = [&](const json &nodeJson) -> std::shared_ptr<const Guest> {
        if (!nodeJson.contains(guestPagesFieldName)) {
            return nullptr;
        }
        return std::make_shared<Guest>(std::unordered_set<int>(
            nodeJson[guestPagesFieldName].begin(), nodeJson[guestPagesFieldName].end()));
    };

    std::function<void(ClusterTreeInstance &, size_t, const json &,
                       std::unordered_map<size_t, size_t> &)>
        parseCluster = [&](auto &tree, const size_t parentCluster, const json &clusterJson,
                           auto &jsonToNodeIds) {
            const size_t cluster = tree.createCluster(parentCluster);

            for (const auto &nodeJson : clusterJson[nodesFieldName]) {
                const std::unordered_set<int> pages =
                    nodeJson[pagesFieldName].get<std::unordered_set<int>>();

                size_t jsonNodeId = nodeJson[nodeIdFieldName].get<size_t>();

                std::vector<size_t> parents;
                for (const size_t jsonParentId :
                     nodeJson[nodeParentsFieldName].get<std::vector<int>>()) {
                    parents.push_back(jsonToNodeIds[jsonParentId]);
                }

                const size_t node = nodeJson.contains(guestPagesFieldName)
                                        ? tree.addLeaf(parents, parseGuest(nodeJson), pages)
                                        : tree.addInner(cluster, parents, pages);

                jsonToNodeIds[jsonNodeId] = node;
            }

            for (const auto &clusterChildJson : clusterJson[clusterChildrenFieldName]) {
                parseCluster(tree, cluster, clusterChildJson, jsonToNodeIds);
            }
        };

    for (const auto &directoryEntry : fs::directory_iterator(directory)) {
        if (directoryEntry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(directoryEntry.path());
        assert(file.is_open());

        for (const auto &instanceJson : json::parse(file)) {
            assert(instanceJson.contains(capacityFieldName));
            size_t capacity = instanceJson[capacityFieldName].get<size_t>();

            std::unordered_map<size_t, size_t> jsonToNodeIds;

            ClusterTreeInstance tree(capacity);
            parseCluster(tree, ClusterTreeInstance::getRootCluster(), instanceJson, jsonToNodeIds);

            instances.push_back(std::move(tree));
            if (instances.size() == maxInstances) {
                return instances;
            }
        }
    }

    return instances;
}

}  // namespace vmp
