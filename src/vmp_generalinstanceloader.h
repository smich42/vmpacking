#ifndef SOLVERS_INSTANCELOADER_H
#define SOLVERS_INSTANCELOADER_H

#include <vmp_generalinstance.h>

#include <vector>

namespace vmp
{

class GeneralInstanceLoader
{
  public:
    explicit GeneralInstanceLoader(std::string directory);

    [[nodiscard]] std::vector<GeneralInstance>
    load(int maxInstances = -1, const std::string &capacityFieldName = "capacity",
         const std::string &guestsFieldName = "guests") const;

    ~GeneralInstanceLoader() = default;

  private:
    const std::string directory;
};

}  // namespace vmp

#endif  // SOLVERS_INSTANCELOADER_H