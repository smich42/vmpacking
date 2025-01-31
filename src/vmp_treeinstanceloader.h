#ifndef VMP_TREEINSTANCELOADER_H
#define VMP_TREEINSTANCELOADER_H

#include <vmp_treeinstance.h>

#include <vector>

namespace vmp
{

class TreeInstanceLoader
{
  public:
    explicit TreeInstanceLoader(std::string directory);

    [[nodiscard]]
    std::vector<TreeInstance> load(int maxInstances = -1,
                                   const std::string &capacityFieldName = "capacity",
                                   const std::string &guestPagesFieldName = "guestPages",
                                   const std::string &pagesFieldName = "pages",
                                   const std::string &childrenFieldName = "children") const;

    ~TreeInstanceLoader() = default;

  private:
    const std::string directory;
};

}  // namespace vmp

#endif  // VMP_TREEINSTANCELOADER_H
