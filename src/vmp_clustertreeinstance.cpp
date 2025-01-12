#include <cassert>
#include <vmp_clustertreeinstance.h>

namespace vmp
{

void ClusterTreeInstance::assertParentsInExpectedCluster(
    const std::vector<size_t> &parents, const size_t cluster) const
{
    if (parents.empty()) {
        return;
    }

    for (const size_t parent : parents) {
        assert(nodes[parent].cluster == cluster);
    }
}

size_t ClusterTreeInstance::addInner(const size_t cluster,
                                     const std::vector<size_t> &parents,
                                     const std::vector<int> &pages)
{
    assertParentsInExpectedCluster(parents, clusters[cluster].parent);

    const size_t newId = nodes.size();
    nodes.emplace_back(parents, pages, nullptr, cluster);
    clusters[cluster].nodes.push_back(newId);
    return newId;
}

size_t ClusterTreeInstance::addLeaf(const size_t cluster,
                                    const std::vector<size_t> &parents,
                                    const std::shared_ptr<const Guest> &guest)
{
    assertParentsInExpectedCluster(parents, clusters[cluster].parent);

    const size_t newId = nodes.size();
    nodes.emplace_back(parents, std::vector<int>(), guest, cluster);
    clusters[cluster].nodes.push_back(newId);
    return newId;
}

size_t ClusterTreeInstance::createCluster(const size_t parent)
{
    const size_t newId = clusters.size();
    clusters.emplace_back(parent, std::vector<size_t>());
    return newId;
}

size_t ClusterTreeInstance::rootCluster()
{
    return 0;
}

const std::vector<size_t> &
ClusterTreeInstance::nodesIn(const size_t cluster) const
{
    return clusters[cluster].nodes;
}

const std::vector<size_t> &
ClusterTreeInstance::parentsOf(const size_t node) const
{
    return nodes[node].parents;
}

const std::vector<int> &ClusterTreeInstance::pagesOf(const size_t node) const
{
    return nodes[node].pages;
}

const std::shared_ptr<const Guest> &
ClusterTreeInstance::guestOf(const size_t node) const
{
    return nodes[node].guest;
}

bool ClusterTreeInstance::isLeaf(const size_t node) const
{
    return nodes[node].guest != nullptr;
}

size_t ClusterTreeInstance::nodeCount() const
{
    return nodes.size();
}

size_t ClusterTreeInstance::nodeCountOf(const size_t cluster) const
{
    return clusters[cluster].nodes.size();
}

}  // namespace vmp