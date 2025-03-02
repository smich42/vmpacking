#ifndef SOLVERS_INSTANCELOADER_H
#define SOLVERS_INSTANCELOADER_H

#include <vmp_generalinstance.h>

#include <json.hpp>
#include <set>
#include <vector>

namespace vmp
{

class GeneralInstanceLoader
{
  public:
    explicit GeneralInstanceLoader(std::string directory,
                                   std::string capacityFieldName = "capacity",
                                   std::string guestsFieldName = "guests");

    [[nodiscard]] std::vector<GeneralInstance> load(int maxInstances = -1);

    ~GeneralInstanceLoader() = default;

  private:
    const std::string directory;

    const std::string capacityFieldName;
    const std::string guestsFieldName;

    std::set<std::filesystem::path> paths;
    std::unordered_map<std::filesystem::path, int> processedInstances;
};

}  // namespace vmp

#endif  // SOLVERS_INSTANCELOADER_H