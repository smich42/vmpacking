#include <InstanceLoader.h>
#include <Packing.h>
#include <iostream>
#include <numeric>

Packing solveNextFit(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;

    for (const auto &guest : instance->guests) {
        if (hosts.empty() || !hosts.back()->accommodatesGuest(guest)) {
            hosts.emplace_back(std::make_shared<Host>(instance->capacity));
        }
        hosts.back()->addGuest(guest);
    }

    return Packing(instance, hosts);
}

Packing solveFirstFit(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;

    for (const auto &guest : instance->guests) {
        auto hostIter = std::ranges::find_if(
            hosts.begin(), hosts.end(),
            [&](const auto host) { return host->accommodatesGuest(guest); });

        if (hostIter == hosts.end()) {
            hosts.emplace_back(std::make_shared<Host>(instance->capacity));
            hostIter = std::prev(hosts.end());
        }

        (*hostIter)->addGuest(guest);
    }

    return Packing(instance, hosts);
}

Packing solveBestFusion(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;

    for (const auto &guest : instance->guests) {
        double bestRatio = std::numeric_limits<double>::max();
        std::shared_ptr<Host> bestHost = nullptr;

        for (const auto &host : hosts) {
            if (!host->accommodatesGuest(guest))
                continue;

            if (const double candidateRatio = guest->relativeSizeOn(*host);
                candidateRatio < bestRatio) {
                bestHost = host;
                bestRatio = candidateRatio;
            }
        }

        if (!bestHost) {
            hosts.emplace_back(std::make_shared<Host>(instance->capacity));
            bestHost = hosts.back();
        }
        bestHost->addGuest(guest);
    }

    return Packing(instance, hosts);
}

Packing solveOverloadAndRemove(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    std::unordered_map<std::shared_ptr<Guest>, std::shared_ptr<Host>>
        hostsTried;

    for (const auto &guest : instance->guests) {
    }
}

int main()
{
    InstanceLoader loader("../resource/gauss");
    loader.loadInstanceData(1, "capacity", "tiles");

    const auto instance =
        std::make_shared<Instance>(loader.makeInstances().front());

    const auto packingNextFit = solveNextFit(instance);
    std::cout << packingNextFit.hosts->size() << std::endl;

    const auto packingFirstFit = solveFirstFit(instance);
    std::cout << packingFirstFit.hosts->size() << std::endl;

    const auto packingBestFusion = solveBestFusion(instance);
    std::cout << packingBestFusion.hosts->size() << std::endl;

    return 0;
}
