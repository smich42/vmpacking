#ifndef VMP_CLUSTERTREEINSTANCE_H
#define VMP_CLUSTERTREEINSTANCE_H

#include <vector>
#include <vmp_guest.h>

namespace vmp
{

class ClusterTreeInstance
{
  public:
    const size_t capacity;

    size_t addInner(size_t cluster, const std::vector<size_t> &parents, const std::set<int> &pages);
    size_t addLeaf(const std::vector<size_t> &parents, const std::shared_ptr<const Guest> &guest,
                   const std::set<int> &pages);

    size_t createCluster(size_t parent);

    [[nodiscard]] const std::vector<size_t> &clusterNodes(size_t cluster) const;
    [[nodiscard]] const std::vector<size_t> &clusterChildren(size_t cluster) const;
    [[nodiscard]] size_t clusterParent(size_t cluster) const;
    [[nodiscard]] bool clusterIsLeaf(size_t cluster) const;

    [[nodiscard]] const std::vector<size_t> &nodeParents(size_t node) const;
    [[nodiscard]] const std::vector<size_t> &nodeChildren(size_t node) const;
    [[nodiscard]] const std::set<int> &nodePages(size_t node) const;
    [[nodiscard]] const std::shared_ptr<const Guest> &nodeGuest(size_t node) const;
    [[nodiscard]] bool nodeIsLeaf(size_t node) const;
    [[nodiscard]] size_t nodeCount() const;
    [[nodiscard]] size_t nodeCountOf(size_t cluster) const;

    [[nodiscard]] const std::vector<size_t> &leafNodes() const;
    [[nodiscard]] size_t clusterCount() const;
    [[nodiscard]] static size_t rootCluster();

    explicit ClusterTreeInstance(size_t capacity);

    static constexpr size_t ROOT_CLUSTER = 0;

  private:
    struct Node
    {
        // Parents must be in the same cluster
        std::vector<size_t> parents;
        std::vector<size_t> children;

        // If it's an inner node, the pages shared by all descendants
        // If it's a leaf, the pages unique to the node
        std::set<int> pages;

        std::shared_ptr<const Guest> guest;
        size_t cluster;

        Node(const std::vector<size_t> &parents, const std::set<int> &pages,
             const std::shared_ptr<const Guest> &guest, const size_t cluster)
            : parents(parents), pages(pages), guest(guest), cluster(cluster)
        {
        }
    };

    struct Cluster
    {
        size_t parent;
        std::vector<size_t> children;

        std::vector<size_t> nodes;

        Cluster(const size_t parent, const std::vector<size_t> &nodes)
            : parent(parent), nodes(nodes)
        {
        }
    };

    [[nodiscard]] bool checkNodesAreInCluster(const std::vector<size_t> &nodes,
                                              size_t cluster) const;

    std::vector<Node> nodes;
    std::vector<size_t> leaves;
    std::vector<Cluster> clusters;
};

}  // namespace vmp
#endif