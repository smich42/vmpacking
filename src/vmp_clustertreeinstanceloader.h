#ifndef VMP_CLUSTERTREEINSTANCELOADER_H
#define VMP_CLUSTERTREEINSTANCELOADER_H

#include <json.hpp>
#include <vmp_clustertreeinstance.h>

#include <vector>

namespace vmp
{

class ClusterTreeInstanceLoader
{
  public:
    explicit ClusterTreeInstanceLoader(std::string directory,
                                       std::string capacityFieldName = "capacity",
                                       std::string nodesFieldName = "nodes",
                                       std::string nodeIdFieldName = "node_id",
                                       std::string nodeParentsFieldName = "node_parents",
                                       std::string pagesFieldName = "node_pages",
                                       std::string guestPagesFieldName = "guest_pages",
                                       std::string clusterChildrenFieldName = "cluster_children");

    [[nodiscard]]
    std::vector<ClusterTreeInstance> load(int maxInstances = -1) const;

    ~ClusterTreeInstanceLoader() = default;

  private:
    const std::string directory;

    const std::string capacityFieldName;
    const std::string nodesFieldName;
    const std::string nodeIdFieldName;
    const std::string nodeParentsFieldName;
    const std::string pagesFieldName;
    const std::string guestPagesFieldName;
    const std::string clusterChildrenFieldName;

    [[nodiscard]] std::shared_ptr<Guest> parseGuest(const nlohmann::json &nodeJson) const;
    void parseClusterSubtree(ClusterTreeInstance &instance, size_t parentCluster,
                             const nlohmann::json &clusterJson,
                             std::unordered_map<size_t, size_t> &jsonToNodeIds) const;
};

};  // namespace vmp

#endif  // VMP_TREEINSTANCELOADER_H
