#include <vmp_generalinstanceloader.h>
#include <vmp_maximisers.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

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
              << (packing.validateForInstance(instance) ? "valid" : "invalid") << std::endl;
}

template <typename InstanceType>
    requires vmp::Instance<InstanceType>
void runSingleHostMaximiser(vmp::Host (*maximiser)(const InstanceType &instance),
                            const std::string &name, const InstanceType &instance)
{
    std::vector<std::pair<std::shared_ptr<const vmp::Guest>, int>> guests;
    guests.reserve(instance.getGuestCount());
    for (const auto &guest : instance.getGuests()) {
        guests.emplace_back(guest, 1);
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const auto host = maximiser(instance);

    const auto end = std::chrono::high_resolution_clock::now();

    const double elapsed =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) /
        1000.0;

    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Elapsed: " << elapsed << " s\n";
    std::cout << "Result: " << host.getGuestCount() << " guests on host, "
              << (!host.isOverfull() ? "valid" : "INVALID") << std::endl;
}

void runClusterTree()
{
    vmp::ClusterTreeInstance instance(8);

    const size_t rootCluster = vmp::ClusterTreeInstance::getRootCluster();
    const size_t clusterA = instance.createCluster(rootCluster);
    const size_t clusterB = instance.createCluster(rootCluster);
    const size_t nodeR1 = instance.addInner(rootCluster, {}, { 1, 2 });
    const size_t nodeA = instance.addInner(clusterA, { nodeR1 }, { 4 });
    const size_t nodeB = instance.addInner(clusterB, { nodeR1 }, { 3, 4 });

    const auto guest1 = std::make_shared<vmp::Guest>(std::unordered_set{ 1, 2, 3, 4, 5, 6 });
    const auto guest2 = std::make_shared<vmp::Guest>(std::unordered_set{ 1, 2, 3, 4, 5, 8, 9 });
    const auto guest3 = std::make_shared<vmp::Guest>(std::unordered_set{ 1, 2, 3, 4, 9, 10 });
    const auto guest4 = std::make_shared<vmp::Guest>(std::unordered_set{ 1, 2, 3, 4, 11, 12 });

    const size_t leaf1 = instance.addLeaf({ nodeA }, guest1, std::unordered_set{ 3, 5, 6 });
    const size_t leaf2 = instance.addLeaf({ nodeA }, guest2, std::unordered_set{ 3, 5, 8, 9 });
    const size_t leaf3 = instance.addLeaf({ nodeB }, guest3, std::unordered_set{ 9, 10 });
    const size_t leaf4 = instance.addLeaf({ nodeB }, guest4, std::unordered_set{ 11, 12 });

    constexpr double oneHostApprox = 25;  // Throwaway, base it on clusterSize
    constexpr double epsilon = 0.0001;    // Throwaway, base it on oneHostApprox

    runSolver<vmp::ClusterTreeInstance>(
        [](const vmp::ClusterTreeInstance &inst) {
            return solveByMaximiser<vmp::ClusterTreeInstance>(
                inst, [](const auto &inst, const size_t hostCount) {
                    return maximiseByLocalSearch<vmp::ClusterTreeInstance>(
                        inst, hostCount,
                        [](const auto &inst, const auto &profits) {
                            return vmp::maximiseOneHostByClusterTree(inst, profits);
                        },
                        oneHostApprox, epsilon);
                });
        },
        "Local Search on Cluster-Tree Maximiser", instance);
}

void runTree()
{
    vmp::TreeInstance instance(4, { 1, 2 });

    const size_t nodeA = instance.addInner(vmp::TreeInstance::getRootNode(), { 3, 4 });
    const size_t nodeB = instance.addInner(nodeA, { 5 });

    const auto guest1 = std::make_shared<vmp::Guest>(std::unordered_set{ 1, 3, 5 });
    const auto guest2 = std::make_shared<vmp::Guest>(std::unordered_set{ 2, 4, 6 });
    const auto guest3 = std::make_shared<vmp::Guest>(std::unordered_set{ 3, 5, 7 });

    const size_t leaf1 = instance.addLeaf(nodeA, guest1, { 3, 5 });
    const size_t leaf2 = instance.addLeaf(nodeA, guest2, { 2, 4 });
    const size_t leaf3 = instance.addLeaf(nodeB, guest3, { 3, 5, 7 });

    runSolver<vmp::TreeInstance>(vmp::solveSimpleTree, "Tree Model", instance);
}

int main()
{
    vmp::GeneralInstanceLoader loader("../resource/gauss");
    loader.load(1, "capacity", "tiles");

    const auto instance = loader.makeGeneralInstances().front();

    std::cout << instance << std::endl;

    runSolver(vmp::solveByNextFit, "Next Fit", instance);
    runSolver(vmp::solveByFirstFit, "First Fit", instance);
    runSolver(vmp::solveByBestFusion, "Best Fusion", instance);
    runSolver(vmp::solveByOverloadAndRemove, "Overload and Remove", instance);
    runSolver(vmp::solveByLocalityScore, "Locality Score", instance);

    constexpr double epsilon = 0.0001;    // Throwaway, base it on oneHostApprox
    constexpr double oneHostApprox = 25;  // Throwaway, base it on clusterSize

    runSolver<vmp::GeneralInstance>(
        [](const vmp::GeneralInstance &inst) {
            return solveByMaximiser<vmp::GeneralInstance>(
                inst, [](const auto &inst, const size_t hostCount) {
                    return maximiseByLocalSearch<vmp::GeneralInstance>(
                        inst, hostCount,
                        [](const auto &inst, const auto &profits) {
                            return maximiseOneHostBySubsetEfficiency(inst, profits, 1);
                        },
                        oneHostApprox, epsilon);
                });
        },
        "Local Search on GSAVVM", instance);

    constexpr int initialSubsetSize = 1;

    // Compose one host maximiser -> local search maximiser -> solver
    runSolver<vmp::GeneralInstance>(
        [](const vmp::GeneralInstance &inst) {
            return solveByMaximiser<vmp::GeneralInstance>(inst, [](const auto &inst,
                                                                   const size_t hostCount) {
                return maximiseByLocalSearch<vmp::GeneralInstance>(
                    inst, hostCount,
                    [](const auto &inst, const auto &profits) {
                        return maximiseOneHostBySubsetEfficiency(inst, profits, initialSubsetSize);
                    },
                    oneHostApprox, epsilon);
            });
        },
        "Local Search on Subset Value Maximiser", instance);

    runClusterTree();
    runTree();

    return 0;
}