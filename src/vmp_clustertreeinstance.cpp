#include <vmp_clustertreeinstance.h>

#include <cassert>

namespace vmp
{

ClusterTreeInstance::ClusterTreeInstance(const size_t capacity) : capacity(capacity)
{
    this->clusters = { Cluster(ROOT_CLUSTER, {}) };
}

bool ClusterTreeInstance::checkNodesAreInCluster(const std::vector<size_t> &nodes,
                                                 const size_t cluster) const
{
    return std::ranges::all_of(
        nodes, [&](const size_t node) { return this->nodes[node].cluster == cluster; });
}

size_t ClusterTreeInstance::addInner(const size_t cluster, const std::vector<size_t> &parents,
                                     const std::set<int> &pages)
{
    assert(checkNodesAreInCluster(parents, clusters[cluster].parent));

    const size_t newNode = nodes.size();
    nodes.emplace_back(parents, pages, nullptr, cluster);
    clusters[cluster].nodes.push_back(newNode);

    for (const size_t parent : parents) {
        nodes[parent].children.push_back(newNode);
    }
    return newNode;
}

size_t ClusterTreeInstance::addLeaf(const std::vector<size_t> &parents,
                                    const std::shared_ptr<const Guest> &guest,
                                    const std::set<int> &pages)
{
    assert(!parents.empty());

    const size_t parentCluster = nodes[parents[0]].cluster;
    assert(checkNodesAreInCluster(parents, parentCluster));

    const size_t newNode = nodes.size();
    const size_t newCluster = createCluster(parentCluster);

    nodes.emplace_back(parents, pages, guest, newCluster);
    clusters[newCluster].nodes.push_back(newNode);
    this->leaves.push_back(newNode);

    for (const size_t parent : parents) {
        nodes[parent].children.push_back(newNode);
    }
    return newNode;
}

size_t ClusterTreeInstance::createCluster(const size_t parent)
{
    const size_t newCluster = clusters.size();

    clusters.emplace_back(parent, std::vector<size_t>());
    clusters[parent].children.push_back(newCluster);

    return newCluster;
}

size_t ClusterTreeInstance::rootCluster()
{
    return ROOT_CLUSTER;
}

const std::vector<size_t> &ClusterTreeInstance::clusterNodes(const size_t cluster) const
{
    return clusters[cluster].nodes;
}

const std::vector<size_t> &ClusterTreeInstance::clusterChildren(const size_t cluster) const
{
    return clusters[cluster].children;
}

size_t ClusterTreeInstance::clusterParent(const size_t cluster) const
{
    return clusters[cluster].parent;
}

const std::vector<size_t> &ClusterTreeInstance::nodeParents(const size_t node) const
{
    return nodes[node].parents;
}

const std::vector<size_t> &ClusterTreeInstance::nodeChildren(const size_t node) const
{
    return nodes[node].children;
}

const std::vector<size_t> &ClusterTreeInstance::leafNodes() const
{
    return this->leaves;
}

size_t ClusterTreeInstance::clusterCount() const
{
    return this->clusters.size();
}

const std::set<int> &ClusterTreeInstance::nodePages(const size_t node) const
{
    return nodes[node].pages;
}

const std::shared_ptr<const Guest> &ClusterTreeInstance::nodeGuest(const size_t node) const
{
    return nodes[node].guest;
}

bool ClusterTreeInstance::nodeIsLeaf(const size_t node) const
{
    return nodes[node].children.empty();
}

bool ClusterTreeInstance::clusterIsLeaf(const size_t cluster) const
{
    return clusters[cluster].children.empty();
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