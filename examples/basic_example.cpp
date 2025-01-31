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

int main()
{
    const vmp::GeneralInstanceLoader generalLoader("../resource/gauss");
    const auto generalInstance = generalLoader.load(1, "capacity", "tiles").front();

    const vmp::TreeInstanceLoader treeLoader("../resource/tree");
    const auto treeInstance = treeLoader.load(1).front();

    const vmp::ClusterTreeInstanceLoader clusterTreeLoader("../resource/clustertree");
    const auto clusterTreeInstance = clusterTreeLoader.load(1).front();

    runSolver(vmp::solveByNextFit, "Next Fit", generalInstance);
    runSolver(vmp::solveByFirstFit, "First Fit", generalInstance);
    runSolver(vmp::solveByBestFusion, "Best Fusion", generalInstance);
    runSolver(vmp::solveByOverloadAndRemove, "Overload and Remove", generalInstance);
    runSolver(vmp::solveByLocalityScore, "Locality Score", generalInstance);

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
        "Local Search on GSAVVM", generalInstance);

    constexpr int initialSubsetSize = 1;  // This makes the below equivalent to GSAVVM

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
        "Local Search on Subset Value Maximiser", generalInstance);

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
        "Local Search on Cluster-Tree Maximiser", clusterTreeInstance);

    runSolver<vmp::TreeInstance>(vmp::solveSimpleTree, "Tree Model", treeInstance);

    return 0;
}