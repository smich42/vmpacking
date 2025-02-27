#include <vmp_clustertreeinstanceloader.h>
#include <vmp_generalinstanceloader.h>
#include <vmp_maximisers.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>
#include <vmp_treeinstanceloader.h>

#include <iostream>

template <typename InstanceType>
    requires vmp::Instance<InstanceType>
void runSolver(vmp::Packing (*solver)(const InstanceType &), const std::string &name,
               const InstanceType &instance)
{
    const auto start = std::chrono::high_resolution_clock::now();
    const auto packing = solver(instance);
    const auto end = std::chrono::high_resolution_clock::now();

    const double elapsed =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) /
        1000.0;

    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Elapsed: " << elapsed << " s\n";
    std::cout << "Result: " << packing.getHostCount() << ", "
              << packing.validateForInstance(instance) << std::endl;
}

int main(int argc, char *argv[])
{
    assert(argc == 4);

    const vmp::GeneralInstanceLoader generalLoader(argv[1]);
    const auto generalInstance = generalLoader.load(1)[0];
    std::cout << "Loaded general instance with " << generalInstance.getGuestCount() << " guests "
              << std::endl;

    const vmp::TreeInstanceLoader treeLoader(argv[2]);
    const auto treeInstance = treeLoader.load(1)[0];
    std::cout << "Loaded tree instance with " << treeInstance.getGuests().size() << " guests "
              << std::endl;

    const vmp::ClusterTreeInstanceLoader clusterTreeLoader(argv[3]);
    const auto clusterTreeInstance = clusterTreeLoader.load(1)[0];
    std::cout << "Loaded cluster-tree instance with " << clusterTreeInstance.getGuests().size()
              << " guests " << std::endl;

    // === General Solvers ===
    runSolver(vmp::solveByNextFit, "Next Fit", generalInstance);
    runSolver(vmp::solveByFirstFit, "First Fit", generalInstance);
    runSolver(vmp::solveByBestFusion, "Best Fusion", generalInstance);
    runSolver(vmp::solveByOverloadAndRemove, "Overload and Remove", generalInstance);
    runSolver(vmp::solveByLocalityScore, "Locality Score", generalInstance);
    runSolver<vmp::GeneralInstance>(
        [](const auto &instance) { return vmp::solveBySubsetEfficiency(instance, 1); },
        "Local Search on GSAVVM", generalInstance);
    runSolver<vmp::GeneralInstance>(
        [](const auto &instance) { return vmp::solveBySubsetEfficiency(instance, 2); },
        "Local Search by Subset Efficiency", generalInstance);

    // === Tree Solver ===
    runSolver(vmp::solveBySimpleTree, "Tree Model", treeInstance);

    // === Cluster-Tree Solver ===
    runSolver<vmp::ClusterTreeInstance>(
        [](const auto &instance) { return vmp::solveByClusterTree(instance); },
        "Local Search on Cluster-Tree Maximiser", clusterTreeInstance);

    return 0;
}