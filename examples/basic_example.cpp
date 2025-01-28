#include <vmp_maximisers.h>

#include <iostream>
#include <vmp_generalinstanceloader.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

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

    const auto guest1 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 5, 6 });
    const auto guest2 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 5, 8, 9 });
    const auto guest3 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 9, 10 });
    const auto guest4 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 11, 12 });

    const size_t leaf1 = instance.addLeaf({ nodeA }, guest1, std::set{ 3, 5, 6 });
    const size_t leaf2 = instance.addLeaf({ nodeA }, guest2, std::set{ 3, 5, 8, 9 });
    const size_t leaf3 = instance.addLeaf({ nodeB }, guest3, std::set{ 9, 10 });
    const size_t leaf4 = instance.addLeaf({ nodeB }, guest4, std::set{ 11, 12 });

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
        "Local Search on Cluster Tree Maximiser", instance);
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

    runSingleHostMaximiser<vmp::GeneralInstance>(
        [](const vmp::GeneralInstance &inst) {
            std::map<std::shared_ptr<const vmp::Guest>, int> profits;
            for (const auto &guest : inst.getGuests()) {
                profits[guest] = 1;
            }
            return maximiseOneHostBySubsetEfficiency(inst, profits, 1);
        },
        "GSAVVM", instance);

    constexpr int initialSubsetSize = 1;
    constexpr double oneHostApprox = 25;  // Throwaway, base it on clusterSize
    constexpr double epsilon = 0.0001;    // Throwaway, base it on oneHostApprox

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

    return 0;
}