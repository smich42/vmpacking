#include <vmp_treeinstance.h>

#include <cassert>

namespace vmp
{

size_t TreeInstance::addInner(const size_t parent, const std::unordered_set<int> &pages)
{
    const size_t newNode = nodes.size();
    nodes.emplace_back(parent, pages, nullptr);
    nodes[parent].children.push_back(newNode);
    return newNode;
}

size_t TreeInstance::addLeaf(const size_t parent, const std::shared_ptr<const Guest> &guest,
                             const std::unordered_set<int> &pages)
{
    const size_t newNode = nodes.size();
    nodes.emplace_back(parent, pages, guest);
    nodes[parent].children.push_back(newNode);
    return newNode;
}

const std::vector<size_t> &TreeInstance::getNodeChildren(const size_t node) const
{
    return nodes[node].children;
}

size_t TreeInstance::getNodeParent(const size_t node) const
{
    return nodes[node].parent;
}

const std::unordered_set<int> &TreeInstance::getNodePages(const size_t node) const
{
    return nodes[node].pages;
}

const std::shared_ptr<const Guest> &TreeInstance::getNodeGuest(const size_t node) const
{
    return nodes[node].guest;
}

bool TreeInstance::nodeIsLeaf(const size_t node) const
{
    return nodes[node].children.empty();
}

size_t TreeInstance::getNodeCount() const
{
    return nodes.size();
}

}  // namespace vmp