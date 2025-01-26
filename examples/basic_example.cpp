#include <vmp_maximisers.h>

#include <iostream>
#include <vmp_generalinstanceloader.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

void runSolver(vmp::Packing (*solver)(const vmp::GeneralInstance &), const std::string &name,
               const vmp::GeneralInstance &instance)
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
    std::cout << "Result: " << packing.hostCount() << ", "
              << (packing.validateForInstance(instance) ? "valid" : "invalid") << std::endl;
}

void runSingleHostMaximiser(vmp::Host (*maximiser)(vmp::GuestProfitVecIt, vmp::GuestProfitVecIt,
                                                   size_t),
                            const std::string &name, const vmp::GeneralInstance &instance)
{
    std::vector<std::pair<std::shared_ptr<const vmp::Guest>, int>> guests;
    guests.reserve(instance.guestCount());
    for (const auto &guest : instance.guests) {
        guests.emplace_back(guest, 1);
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const auto host = maximiser(guests.begin(), guests.end(), instance.capacity);

    const auto end = std::chrono::high_resolution_clock::now();

    const double elapsed =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) /
        1000.0;

    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Elapsed: " << elapsed << " s\n";
    std::cout << "Result: " << host.guestCount() << " guests on host, "
              << (!host.isOverfull() ? "valid" : "INVALID") << std::endl;
}

void runClusterTree()
{
    vmp::ClusterTreeInstance instance(11);

    const size_t rootCluster = vmp::ClusterTreeInstance::rootCluster();
    const size_t clusterA = instance.createCluster(rootCluster);
    const size_t clusterB = instance.createCluster(rootCluster);
    const size_t nodeR1 = instance.addInner(rootCluster, {}, { 1, 2 });
    const size_t nodeA = instance.addInner(clusterA, { nodeR1 }, { 3, 4 });
    const size_t nodeB = instance.addInner(clusterB, { nodeR1 }, { 3, 4 });

    const auto guest1 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 5, 6 });
    const auto guest2 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 7, 8 });
    const auto guest3 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 9, 10 });
    const auto guest4 = std::make_shared<vmp::Guest>(std::set{ 1, 2, 3, 4, 11, 12 });

    const size_t leaf1 = instance.addLeaf({ nodeA }, guest1, std::set{ 5, 6 });
    const size_t leaf2 = instance.addLeaf({ nodeA }, guest2, std::set{ 7, 8 });
    const size_t leaf3 = instance.addLeaf({ nodeB }, guest3, std::set{ 9, 10 });
    const size_t leaf4 = instance.addLeaf({ nodeB }, guest4, std::set{ 11, 12 });

    const vmp::Host host = maximiseOneHostByClusterTree(instance);

    std::cout << "Pages used: " << host.uniquePageCount() << std::endl;
    std::cout << "Selected guests:" << std::endl;
    for (const auto &guest : host.guests) {
        std::cout << guest << std::endl;
    }
}

int main()
{
    runClusterTree();
    return 0;

    vmp::GeneralInstanceLoader loader("../resource/gauss");
    loader.load(1, "capacity", "tiles");

    const auto instance = loader.makeGeneralInstances().front();

    std::cout << instance << std::endl;

    runSolver(vmp::solveByNextFit, "Next Fit", instance);
    runSolver(vmp::solveByFirstFit, "First Fit", instance);
    runSolver(vmp::solveByBestFusion, "Best Fusion", instance);
    runSolver(vmp::solveByOverloadAndRemove, "Overload and Remove", instance);
    runSolver(vmp::solveByLocalityScore, "Locality Score", instance);

    runSingleHostMaximiser(
        [](vmp::GuestProfitVecIt guestsBegin, vmp::GuestProfitVecIt guestsEnd,
           const size_t capacity) {
            return maximiseOneHostByClusterValues(guestsBegin, guestsEnd, capacity, 1);
        },
        "GSAVVM", instance);

    constexpr int clusterSize = 1;
    constexpr double oneHostApprox = 25;  // Throwaway, base it on clusterSize
    constexpr double epsilon = 0.0001;    // Throwaway, base it on oneHostApprox

    // Compose one host maximiser -> local search maximiser -> solver
    runSolver(
        [](const auto &inst) {
            return solveByMaximiser(inst, [](const auto &inst, const size_t hostCount) {
                return maximiseByLocalSearch(
                    inst, hostCount,
                    [](const auto beg, const auto end, const size_t cap) {
                        return maximiseOneHostByClusterValues(beg, end, cap, clusterSize);
                    },
                    oneHostApprox, epsilon);
            });
        },
        "Local Search", instance);
    return 0;
}