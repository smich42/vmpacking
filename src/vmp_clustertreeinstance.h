#ifndef VMP_CLUSTERTREEINSTANCE_H
#define VMP_CLUSTERTREEINSTANCE_H

#include <vector>
#include <vmp_guest.h>

namespace vmp
{

class ClusterTreeInstance
{
  public:
    size_t addInner(size_t cluster, const std::vector<size_t> &parents,
                    const std::vector<int> &pages);
    size_t addLeaf(size_t cluster, const std::vector<size_t> &parents,
                   const std::shared_ptr<const Guest> &guest);

    size_t createCluster(size_t parent);
    [[nodiscard]] static size_t rootCluster();

    [[nodiscard]] const std::vector<size_t> &nodesIn(size_t cluster) const;
    [[nodiscard]] const std::vector<size_t> &parentsOf(size_t node) const;
    [[nodiscard]] const std::vector<int> &pagesOf(size_t node) const;
    [[nodiscard]] const std::shared_ptr<const Guest> &
    guestOf(size_t node) const;
    [[nodiscard]] bool isLeaf(size_t node) const;
    [[nodiscard]] size_t nodeCount() const;
    [[nodiscard]] size_t nodeCountOf(size_t cluster) const;

  private:
    struct Node
    {
        // Parents must be in the same cluster
        std::vector<size_t> parents;

        // If it's an inner node, the pages shared by all descendants
        // If it's a leaf, the pages unique to the node
        std::vector<int> pages;

        std::shared_ptr<const Guest> guest;
        size_t cluster;

        Node(const std::vector<size_t> &parents, const std::vector<int> &pages,
             const std::shared_ptr<const Guest> &guest, const size_t cluster)
            : parents(parents), pages(pages), guest(guest), cluster(cluster)
        {
        }
    };

    struct Cluster
    {
        size_t parent;
        std::vector<size_t> nodes;

        Cluster(const size_t parent, const std::vector<size_t> &nodes)
            : parent(parent), nodes(nodes)
        {
        }
    };

    void assertParentsInExpectedCluster(const std::vector<size_t> &parents,
                                        size_t cluster) const;

    std::vector<Node> nodes;
    std::vector<Cluster> clusters;
};

}  // namespace vmp
#endif