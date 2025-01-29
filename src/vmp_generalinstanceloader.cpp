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

void GeneralInstanceLoader::load(const int max_instances, const std::string &capacity_field_name,
                                 const std::string &guests_field_name)
{
    namespace fs = std::filesystem;

    for (const auto &entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(entry.path());
        assert(file.is_open());

        for (const auto &instance_json : json::parse(file)) {
            assert(instance_json.contains(capacity_field_name));
            assert(instance_json.contains(guests_field_name));

            capacityData.push_back(instance_json[capacity_field_name].get<int>());
            guestData.push_back(
                instance_json[guests_field_name].get<std::vector<std::vector<int>>>());

            if (guestData.size() == max_instances) {
                return;
            }
        }
    }
}

std::vector<GeneralInstance> GeneralInstanceLoader::makeGeneralInstances() const
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
}  // namespace vmp