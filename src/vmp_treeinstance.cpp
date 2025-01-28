#include <vmp_treeinstance.h>

namespace vmp
{

TreeInstance::TreeInstance(const size_t capacity, const std::unordered_set<int> &rootPages,
                           const std::shared_ptr<const Guest> &rootGuest)
    : capacity(capacity)
{
    nodes[ROOT_NODE] = Node(ROOT_NODE, rootPages, rootGuest);
}

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
    leaves.push_back(newNode);

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
    return nodes[node].guest != nullptr;
}

size_t TreeInstance::getNodeCount() const
{
    return nodes.size();
}

size_t TreeInstance::getCapacity() const
{
    return capacity;
}

std::vector<std::shared_ptr<const Guest>> TreeInstance::getGuests() const
{
    std::vector<std::shared_ptr<const Guest>> guests;
    guests.reserve(leaves.size());

    for (const auto &leaf : leaves) {
        guests.push_back(nodes[leaf].guest);
    }
    return guests;
}

const std::vector<size_t> &TreeInstance::getLeaves() const
{
    return leaves;
}

size_t TreeInstance::getRootNode()
{
    return ROOT_NODE;
}

}  // namespace vmp