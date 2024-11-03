#include <iostream>
#include <vmp_instanceloader.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

void runSolver(
    const std::string &name,
    const std::function<vmp::Packing(std::shared_ptr<vmp::Instance>)> &solver,
    const std::shared_ptr<vmp::Instance> &instance)
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
    vmp::InstanceLoader loader("../resource/gauss");
    loader.loadInstanceData(1, "capacity", "tiles");

    const auto instance =
        std::make_shared<vmp::Instance>(loader.makeInstances().front());

    std::cout << *instance << std::endl;

    runSolver("Next Fit", vmp::solveNextFit, instance);
    runSolver("First Fit", vmp::solveFirstFit, instance);
    runSolver("Best Fusion", vmp::solveBestFusion, instance);
    runSolver("Overload and Remove", vmp::solveOverloadAndRemove, instance);

    return 0;
}