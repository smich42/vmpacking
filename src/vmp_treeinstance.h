#ifndef VMP_TREEINSTANCE_H
#define VMP_TREEINSTANCE_H

#include <vmp_guest.h>

#include <unordered_set>
#include <vector>

namespace vmp
{

class TreeInstance
{
  public:
    size_t addInner(size_t parent, const std::unordered_set<int> &pages);
    size_t addLeaf(size_t parent, const std::shared_ptr<const Guest> &guest,
                   const std::unordered_set<int> &pages);

    [[nodiscard]] const std::vector<size_t> &getNodeChildren(size_t node) const;
    [[nodiscard]] size_t getNodeParent(size_t node) const;
    [[nodiscard]] const std::unordered_set<int> &getNodePages(size_t node) const;
    [[nodiscard]] const std::shared_ptr<const Guest> &getNodeGuest(size_t node) const;
    [[nodiscard]] bool nodeIsLeaf(size_t node) const;
    [[nodiscard]] size_t getNodeCount() const;

    explicit TreeInstance() = default;

  private:
    struct Node
    {
        std::size_t parent;
        std::vector<size_t> children;

        // If it's an inner node, the pages shared by all descendants
        // If it's a leaf, the pages unique to the node
        std::unordered_set<int> pages;

        std::shared_ptr<const Guest> guest;

        Node(size_t parent, const std::unordered_set<int> &pages,
             const std::shared_ptr<const Guest> &guest)
            : parent(parent), pages(pages), guest(guest)
        {
        }
    };

    std::vector<Node> nodes;
};

}  // namespace vmp

#endif  // VMP_TREEINSTANCE_H
