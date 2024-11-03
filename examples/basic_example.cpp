#include <iostream>
#include <vmp_instanceloader.h>
#include <vmp_packing.h>
#include <vmp_solvers.h>

int main()
{
    vmp::InstanceLoader loader("../resource/gauss");
    loader.loadInstanceData(1, "capacity", "tiles");

    const auto instance =
        std::make_shared<vmp::Instance>(loader.makeInstances().front());

    std::cout << *instance << std::endl;

    const auto packingNextFit = solveNextFit(instance);
    std::cout << packingNextFit.hosts->size() << " "
              << packingNextFit.validate() << std::endl;

    const auto packingFirstFit = solveFirstFit(instance);
    std::cout << packingFirstFit.hosts->size() << " "
              << packingFirstFit.validate() << std::endl;

    const auto packingBestFusion = solveBestFusion(instance);
    std::cout << packingBestFusion.hosts->size() << " "
              << packingBestFusion.validate() << std::endl;

    const auto packingOverloadAndRemove = solveOverloadAndRemove(instance);
    std::cout << packingOverloadAndRemove.hosts->size() << " "
              << packingOverloadAndRemove.validate() << std::endl;

    return 0;
}
