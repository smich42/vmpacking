#include <iostream>
#include <vmp_generalinstanceloader.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

void runSolver(const std::string &name,
               const std::function<vmp::Packing(vmp::GeneralInstance)> &solver,
               const vmp::GeneralInstance &instance)
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
    std::cout << "Result: " << packing.hosts->size() << ", "
              << (packing.validate() ? "valid" : "invalid") << std::endl;
}

int main()
{
    vmp::GeneralInstanceLoader loader("../resource/gauss");
    loader.load(1, "capacity", "tiles");

    const auto instance = loader.makeGeneralInstances().front();

    std::cout << instance << std::endl;

    runSolver("Next Fit", vmp::solveByNextFit, instance);
    runSolver("First Fit", vmp::solveByFirstFit, instance);
    runSolver("Best Fusion", vmp::solveByBestFusion, instance);
    runSolver("Overload and Remove", vmp::solveByOverloadAndRemove, instance);

    return 0;
}