#ifndef SOLVERS_INSTANCELOADER_H
#define SOLVERS_INSTANCELOADER_H

#include <vmp_generalinstance.h>

#include <vector>

namespace vmp
{

class GeneralInstanceLoader
{
  public:
    explicit GeneralInstanceLoader(std::string directory,
                                   std::string capacityFieldName = "capacity",
                                   std::string guestsFieldName = "guests");

    [[nodiscard]] std::vector<GeneralInstance> load(int maxInstances = -1) const;

    ~GeneralInstanceLoader() = default;

  private:
    const std::string directory;

    const std::string capacityFieldName;
    const std::string guestsFieldName;
};

}  // namespace vmp

#endif  // SOLVERS_INSTANCELOADER_H