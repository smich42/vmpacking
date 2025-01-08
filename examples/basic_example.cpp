#include <vmp_maximisers.h>

#include <iostream>
#include <vmp_generalinstanceloader.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

void runSolver(vmp::Packing (*solver)(const vmp::GeneralInstance &),
               const std::string &name, const vmp::GeneralInstance &instance)
{
    const auto start = std::chrono::high_resolution_clock::now();
    const auto packing = solver(instance);
    const auto end = std::chrono::high_resolution_clock::now();

    const double elapsed =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()) /
        1000.0;

    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Elapsed: " << elapsed << " s\n";
    std::cout << "Result: " << packing.hosts.size() << ", "
              << (packing.validate() ? "valid" : "invalid") << std::endl;
}

void runSingleHostMaximiser(vmp::Host (*maximiser)(vmp::GuestProfitVecIt,
                                                   vmp::GuestProfitVecIt,
                                                   size_t),
                            const std::string &name,
                            const vmp::GeneralInstance &instance)
{
    std::vector<std::pair<std::shared_ptr<vmp::Guest>, int>> guests;
    guests.reserve(instance.guests.size());
    for (const auto &guest : instance.guests) {
        guests.emplace_back(guest, 1);
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const auto host =
        maximiser(guests.begin(), guests.end(), instance.capacity);

    const auto end = std::chrono::high_resolution_clock::now();

    const double elapsed =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count()) /
        1000.0;

    std::cout << "=== " << name << " ===" << std::endl;
    std::cout << "Elapsed: " << elapsed << " s\n";
    std::cout << "Result: " << host.guestCount() << " guests on host, "
              << (!host.isOverfull() ? "valid" : "invalid") << std::endl;
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

    runSingleHostMaximiser(
        [](vmp::GuestProfitVecIt guestsBegin, vmp::GuestProfitVecIt guestsEnd,
           const size_t capacity) {
            return maximiseSingleHostBySimpleEfficiency(guestsBegin, guestsEnd,
                                                        capacity, 1);
        },
        "GSAVVM", instance);

    constexpr int clusterSize = 1;

    runSolver(
        [](const vmp::GeneralInstance &instance) {
            return solveByMaximiser(
                instance, [](const vmp::GeneralInstance &instance,
                             const size_t allowedHostCount) {
                    return maximiseByLocalSearch(
                        instance, allowedHostCount,
                        [](const vmp::GuestProfitVecIt guestsBegin,
                           const vmp::GuestProfitVecIt guestsEnd,
                           const size_t capacity) {
                            return maximiseSingleHostBySimpleEfficiency(
                                guestsBegin, guestsEnd, capacity, clusterSize);
                        },
                        static_cast<double>(instance.guests.size()) /
                            clusterSize,
                        0.0001);
                });
        },
        "Local Search", instance);
    return 0;
}