#ifndef PACKING_H
#define PACKING_H

#include <Host.h>
#include <Instance.h>

class Packing
{
  public:
    explicit Packing(const std::shared_ptr<Instance> &instance,
                     const std::vector<std::shared_ptr<Host>> &hosts);

    const std::shared_ptr<Instance> instance;
    const std::unique_ptr<std::vector<std::shared_ptr<Host>>> hosts;
};

#endif  // PACKING_H