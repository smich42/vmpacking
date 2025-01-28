#ifndef SOLVERS_PACKING_H
#define SOLVERS_PACKING_H

#include <iostream>

#include <ostream>
#include <vmp_generalinstance.h>
#include <vmp_host.h>

#include <unordered_set>

namespace vmp
{

class Packing
{
  public:
    explicit Packing(const std::vector<std::shared_ptr<Host>> &hosts);

    Packing(Packing &other) noexcept = default;
    Packing(Packing &&other) noexcept = default;

    Packing &operator=(Packing &&other) noexcept = default;

    /**
     * Validate the packing against an instance, by checking that:
     * - All guests are present,
     * - No host is overfull; and
     * - No host is empty
     *
     * @param instance the instance against which to validate
     * @return
     */
    template <typename InstanceType>
        requires Instance<InstanceType>
    bool validateForInstance(const InstanceType &instance) const
    {
        std::cout << "Instance capacity: " << instance.getCapacity() << std::endl;

        std::unordered_set<std::shared_ptr<const Guest>> placedGuests;
        for (const auto &host : hosts) {
            std::cout << *host << std::endl;

            if (host->getGuests().empty()) {
                return false;
            }
            if (host->isOverfull()) {
                return false;
            }
            for (const auto &guest : host->getGuests()) {
                placedGuests.insert(guest);
            }
        }

        const auto guests = instance.getGuests();
        return std::ranges::all_of(guests.begin(), guests.end(),
                                   [&](const auto &guest) { return placedGuests.contains(guest); });
    }

    [[nodiscard]] size_t countGuests() const;
    [[nodiscard]] size_t getHostCount() const;

    std::vector<std::shared_ptr<Host>> hosts;
};

}  // namespace vmp
#endif  // SOLVERS_PACKING_H