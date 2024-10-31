#include <vmp_clustertreeinstance.h>
#include <vmp_generalinstance.h>
#include <vmp_solvers.h>
#include <vmp_treeinstance.h>

#include <iostream>

constexpr auto capacity = 4;

// A 4-page capacity makes possible a 2-host packing for these guests:
const auto guest1 = std::make_shared<const vmp::Guest>(std::unordered_set{ 1 });
const auto guest2 = std::make_shared<const vmp::Guest>(std::unordered_set{ 2, 3, 5, 8 });
const auto guest3 = std::make_shared<const vmp::Guest>(std::unordered_set{ 1 });
const auto guest4 = std::make_shared<const vmp::Guest>(std::unordered_set{ 3, 5 });
const auto guest5 = std::make_shared<const vmp::Guest>(std::unordered_set{ 6, 8 });

vmp::GeneralInstance mkGeneral();
vmp::TreeInstance mkTree();

int main()
{
    const auto general = mkGeneral();
    const auto tree = mkTree();
    const auto cluster = nullptr;

    // Using the general solvers
    std::cout << vmp::solveByNextFit(general).getHostCount() << std::endl;
    std::cout << vmp::solveByFirstFit(general).getHostCount() << std::endl;
    std::cout << vmp::solveByEfficiency(general).getHostCount() << std::endl;
    std::cout << vmp::solveByOpportunityAwareEfficiency(general).getHostCount() << std::endl;
    std::cout << vmp::solveByOverloadAndRemove(general).getHostCount() << std::endl;

    // Using the tree solver with an intermediate solver which iterates over an std::unordered_set
    // collection of guests
    using SetGuestIt = std::unordered_set<std::shared_ptr<const vmp::Guest>>::iterator;

    std::cout << vmp::solveByTree<SetGuestIt>(tree, vmp::proceedByFirstFit).getHostCount()
              << std::endl;
    std::cout << vmp::solveByTree<SetGuestIt>(tree, vmp::proceedByOverloadAndRemove).getHostCount()
              << std::endl;

    // Using the general solvers on an instance ordered by tree insertion
    std::cout << vmp::solveByOpportunityAwareEfficiency(tree).getHostCount() << std::endl;
    std::cout << vmp::solveByOverloadAndRemove(tree).getHostCount() << std::endl;
}

vmp::GeneralInstance mkGeneral()
{
    return { capacity, { guest1, guest2, guest3, guest4, guest5 } };
}

vmp::TreeInstance mkTree()
{
    auto tree = vmp::TreeInstance(capacity, {});

    // N.B. a pitfall:
    // TreeInstance is NOT an interface for constructing tree instances; it will NOT check for
    // VM Packing invariants besides that the tree structure is sound.
    // For example, TreeInstanceParser will load a tree "as is" from disk, and if a page exists in
    // both a descendant and an ancestor, that redundancy will NOT be detected!
    const size_t n = tree.addInner(vmp::TreeInstance::getRootNode(), {});
    const size_t left = tree.addInner(n, { 1 });
    tree.addLeaf(left, guest1, {});  // the "1" page is removed now
    tree.addLeaf(left, guest3, {});

    const size_t right = tree.addInner(n, { 3, 5 });
    tree.addLeaf(right, guest2, { 2, 8 });
    tree.addLeaf(right, guest4, {});

    tree.addLeaf(n, guest5, { 6, 8 });

    return tree;
}
