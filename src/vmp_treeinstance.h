#ifndef VMP_TREEINSTANCE_H
#define VMP_TREEINSTANCE_H

#include <vmp_guest.h>

#include <queue>
#include <unordered_set>
#include <vector>

namespace vmp
{

class TreeInstance
{
    friend class TreeInstanceParser;

  public:
    size_t addInner(size_t parent, const std::unordered_set<int> &pages);
    size_t addLeaf(size_t parent, const std::shared_ptr<const Guest> &guest,
                   const std::unordered_set<int> &pages);

    [[nodiscard]] const std::vector<size_t> &getNodeChildren(size_t node) const;
    [[nodiscard]] size_t getNodeParent(size_t node) const;
    [[nodiscard]] const std::unordered_set<int> &getNodePages(size_t node) const;
    [[nodiscard]] std::shared_ptr<const Guest> getNodeGuest(size_t node) const;
    [[nodiscard]] const std::unordered_set<std::shared_ptr<const Guest>> &
    getSubtreeGuests(size_t root) const;
    [[nodiscard]] bool nodeIsLeaf(size_t node) const;
    [[nodiscard]] size_t getNodeCount() const;
    [[nodiscard]] size_t getCapacity() const;
    [[nodiscard]] const std::unordered_set<std::shared_ptr<const Guest>> &getGuests() const;
    [[nodiscard]] const std::vector<size_t> &getLeaves() const;
    void removeSubtree(size_t root);

    static size_t getRootNode();

    TreeInstance(size_t capacity, const std::unordered_set<int> &rootPages);
    TreeInstance(size_t capacity, const std::unordered_set<int> &rootPages,
                 const std::shared_ptr<const Guest> &rootGuest);

  private:
    struct Node
    {
        std::size_t parent;
        std::vector<size_t> children;

        // If it's an inner node, the pages shared by all descendants
        // If it's a leaf, the pages unique to the node
        std::unordered_set<int> pages;

        // This is a useful and reasonably expensive cache to keep at each node
        std::unordered_set<std::shared_ptr<const Guest>> guests;

        Node(const size_t parent, const std::unordered_set<int> &pages,
             const std::unordered_set<std::shared_ptr<const Guest>> &guests)
            : parent(parent), pages(pages), guests(guests)
        {
        }

        Node() : parent(0) {};
    };

    std::vector<std::optional<Node>> nodes;
    std::vector<size_t> leaves;

    const size_t capacity;
    static constexpr size_t ROOT_NODE = 0;
};

}  // namespace vmp

#endif  // VMP_TREEINSTANCE_H
