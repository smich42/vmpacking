#ifndef INSTANCELOADER_H
#define INSTANCELOADER_H

#include <Instance.h>
#include <vector>

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

#endif  // INSTANCELOADER_H