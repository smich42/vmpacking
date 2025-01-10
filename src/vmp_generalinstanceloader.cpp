#include <vmp_generalinstanceloader.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <vector>

using json = nlohmann::json;

namespace vmp
{

GeneralInstanceLoader::GeneralInstanceLoader(std::string directory)
    : directory(std::move(directory))
{
}

static void assertHasFieldName(const json &obj, const std::string &field_name)
{
    if (!obj.contains(field_name)) {
        throw std::runtime_error("No such field: " + field_name);
    }
}

void GeneralInstanceLoader::load(const int max_instances,
                                 const std::string &capacity_field_name,
                                 const std::string &guests_field_name)
{
    namespace fs = std::filesystem;

    for (const auto &entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() != ".json") {
            continue;
        }

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << entry.path() << std::endl;
            continue;
        }

        try {
            for (const auto &instance_json : json::parse(file)) {
                assertHasFieldName(instance_json, capacity_field_name);
                assertHasFieldName(instance_json, guests_field_name);

                capacityData.push_back(
                    instance_json[capacity_field_name].get<int>());
                guestData.push_back(instance_json[guests_field_name]
                                        .get<std::vector<std::vector<int>>>());

                if (guestData.size() == max_instances) {
                    return;
                }
            }
        }
        catch (const std::exception &e) {
            std::cerr << "Error loading instance data from file "
                      << entry.path() << ": " << e.what() << std::endl;
        }
    }
}

std::vector<GeneralInstance>
GeneralInstanceLoader::makeGeneralInstances() const
{
    assert(capacityData.size() == guestData.size());

    std::vector<GeneralInstance> instances;
    for (int i = 0; i < capacityData.size(); i++) {
        std::vector<std::shared_ptr<const Guest>> guests;
        std::ranges::transform(
            guestData[i], std::back_inserter(guests), [](const auto &pages) {
                return std::make_shared<Guest>(
                    std::set(std::make_move_iterator(pages.begin()),
                             std::make_move_iterator(pages.end())));
            });

        instances.emplace_back(capacityData[i], std::move(guests));
    }

    return instances;
}
}  // namespace vmp