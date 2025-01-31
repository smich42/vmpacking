#ifndef VMP_CLUSTERTREEINSTANCELOADER_H
#define VMP_CLUSTERTREEINSTANCELOADER_H

#include <vmp_clustertreeinstance.h>

#include <vector>

namespace vmp
{

class ClusterTreeInstanceLoader
{
  public:
    explicit ClusterTreeInstanceLoader(std::string directory);

    [[nodiscard]]
    std::vector<ClusterTreeInstance>
    load(int maxInstances = -1, const std::string &capacityFieldName = "capacity",
         const std::string &nodesFieldName = "nodes",
         const std::string &nodeIdFieldName = "node_id",
         const std::string &nodeParentsFieldName = "node_parents",
         const std::string &pagesFieldName = "node_pages",
         const std::string &guestPagesFieldName = "guest_pages",
         const std::string &clusterChildrenFieldName = "cluster_children") const;

    ~ClusterTreeInstanceLoader() = default;

  private:
    const std::string directory;
};

};  // namespace vmp

#endif  // VMP_TREEINSTANCELOADER_H
