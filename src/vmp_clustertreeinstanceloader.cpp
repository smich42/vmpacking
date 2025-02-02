#include <vmp_clustertreeinstanceloader.h>

#include <cassert>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

namespace vmp
{

ClusterTreeInstanceLoader::ClusterTreeInstanceLoader(
    std::string directory, std::string capacityFieldName, std::string nodesFieldName,
    std::string nodeIdFieldName, std::string nodeParentsFieldName, std::string pagesFieldName,
    std::string guestPagesFieldName, std::string clusterChildrenFieldName)
    : directory(std::move(directory)),
      capacityFieldName(std::move(capacityFieldName)),
      nodesFieldName(std::move(nodesFieldName)),
      nodeIdFieldName(std::move(nodeIdFieldName)),
      nodeParentsFieldName(std::move(nodeParentsFieldName)),
      pagesFieldName(std::move(pagesFieldName)),
      guestPagesFieldName(std::move(guestPagesFieldName)),
      clusterChildrenFieldName(std::move(clusterChildrenFieldName))
{
}

std::shared_ptr<Guest> ClusterTreeInstanceLoader::parseGuest(const json &nodeJson) const
{
    if (!nodeJson.contains(guestPagesFieldName)) {
        return nullptr;
    }
    return std::make_shared<Guest>(std::unordered_set<int>(nodeJson[guestPagesFieldName].begin(),
                                                           nodeJson[guestPagesFieldName].end()));
}

void ClusterTreeInstanceLoader::parseClusterSubtree(
    ClusterTreeInstance &instance, const size_t parentCluster, const json &clusterJson,
    std::unordered_map<size_t, size_t> &jsonToNodeIds) const
{
    const size_t cluster = instance.createCluster(parentCluster);

    for (const auto &nodeJson : clusterJson[nodesFieldName]) {
        const std::unordered_set<int> pages =
            nodeJson[pagesFieldName].get<std::unordered_set<int>>();

        size_t jsonNodeId = nodeJson[nodeIdFieldName].get<size_t>();

        std::vector<size_t> parents;
        for (const size_t jsonParentId : nodeJson[nodeParentsFieldName].get<std::vector<int>>()) {
            parents.push_back(jsonToNodeIds[jsonParentId]);
        }

        const size_t node = nodeJson.contains(guestPagesFieldName)
                                ? instance.addLeaf(parents, parseGuest(nodeJson), pages)
                                : instance.addInner(cluster, parents, pages);

        jsonToNodeIds[jsonNodeId] = node;
    }

    for (const auto &clusterChildJson : clusterJson[clusterChildrenFieldName]) {
        parseClusterSubtree(instance, cluster, clusterChildJson, jsonToNodeIds);
    }
}

std::vector<ClusterTreeInstance> ClusterTreeInstanceLoader::load(const int maxInstances) const
{
    namespace fs = std::filesystem;

    std::vector<ClusterTreeInstance> instances;
    instances.reserve(maxInstances);

    for (const auto &directoryEntry : fs::directory_iterator(directory)) {
        if (directoryEntry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(directoryEntry.path());
        assert(file.is_open());

        for (const auto &instanceJson : json::parse(file)) {
            assert(instanceJson.contains(capacityFieldName));
            const size_t capacity = instanceJson[capacityFieldName].get<size_t>();

            std::unordered_map<size_t, size_t> jsonToNodeIds;

            ClusterTreeInstance instance(capacity);
            parseClusterSubtree(instance, ClusterTreeInstance::getRootCluster(), instanceJson,
                                jsonToNodeIds);

            instances.push_back(std::move(instance));
            if (instances.size() == maxInstances) {
                return instances;
            }
        }
    }

    return instances;
}

}  // namespace vmp
