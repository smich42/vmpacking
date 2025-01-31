#ifndef VMP_CLUSTERTREEINSTANCELOADER_H
#define VMP_CLUSTERTREEINSTANCELOADER_H

#include <vmp_clustertreeinstance.h>

#include <vector>

namespace vmp
{

class ClusterTreeInstanceLoader
{
  public:
    explicit ClusterTreeInstanceLoader(std::string directory);

    [[nodiscard]]
    std::vector<ClusterTreeInstance> load(int max_instances = -1,
                                          const std::string &capacity_field_name = "capacity",
                                          const std::string &guests_field_name = "guests") const;

    ~ClusterTreeInstanceLoader() = default;

  private:
    const std::string directory;
};

};  // namespace vmp

#endif  // VMP_TREEINSTANCELOADER_H
