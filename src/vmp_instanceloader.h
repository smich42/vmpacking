#ifndef SOLVERS_INSTANCELOADER_H
#define SOLVERS_INSTANCELOADER_H

#include <vector>
#include <vmp_instance.h>

namespace vmp
{

class InstanceLoader
{
  public:
    explicit InstanceLoader(std::string directory);

    void loadInstanceData(int max_instances = -1,
                          const std::string &capacity_field_name = "capacity",
                          const std::string &guests_field_name = "guests");
    [[nodiscard]] std::vector<Instance> makeInstances() const;

  private:
    const std::string directory;

    std::vector<int> capacityData;
    std::vector<std::vector<std::vector<int>>> guestData;
};

}  // namespace vmp

#endif  // SOLVERS_INSTANCELOADER_H