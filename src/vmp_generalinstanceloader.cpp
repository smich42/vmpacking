#include <vmp_generalinstanceloader.h>

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <vector>

using json = nlohmann::json;

namespace vmp
{

GeneralInstanceLoader::GeneralInstanceLoader(std::string directory)
    : directory(std::move(directory))
{
}

static std::vector<GeneralInstance>
makeInstances(const std::vector<int> &capacityData,
              const std::vector<std::vector<std::vector<int>>> &guestData)
{
    assert(capacityData.size() == guestData.size());

    std::vector<GeneralInstance> instances;
    for (int i = 0; i < capacityData.size(); ++i) {
        std::vector<std::shared_ptr<const Guest>> guests;
        for (const auto &guestPages : guestData[i]) {
            guests.push_back(
                std::make_shared<Guest>(std::unordered_set(guestPages.begin(), guestPages.end())));
        }
        instances.emplace_back(capacityData[i], std::move(guests));
    }

    return instances;
}

std::vector<GeneralInstance> GeneralInstanceLoader::load(const int maxInstances,
                                                         const std::string &capacityFieldName,
                                                         const std::string &guestsFieldName) const
{
    namespace fs = std::filesystem;

    std::vector<int> capacityData;
    std::vector<std::vector<std::vector<int>>> guestData;

    capacityData.reserve(maxInstances);
    guestData.reserve(maxInstances);

    for (const auto &directoryEntry : fs::directory_iterator(directory)) {
        if (directoryEntry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(directoryEntry.path());
        assert(file.is_open());

        for (const auto &instance_json : json::parse(file)) {
            assert(instance_json.contains(capacityFieldName));
            assert(instance_json.contains(guestsFieldName));

            capacityData.push_back(instance_json[capacityFieldName].get<int>());
            guestData.push_back(
                instance_json[guestsFieldName].get<std::vector<std::vector<int>>>());

            if (guestData.size() == maxInstances) {
                return makeInstances(capacityData, guestData);
            }
        }
    }

    return makeInstances(capacityData, guestData);
}

}  // namespace vmp