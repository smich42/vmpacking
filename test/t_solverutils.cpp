#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <vmp_solverutils.h>

class SolverUtilsTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        guest1 =
            std::make_shared<vmp::Guest>(std::move(std::set{ 1, 2, 3, 5 }));
        guest2 = std::make_shared<vmp::Guest>(std::move(std::set{ 2, 3, 4 }));
        guest3 =
            std::make_shared<vmp::Guest>(std::move(std::set{ 1, 3, 4, 5 }));
        guest4 = std::make_shared<vmp::Guest>(std::move(std::set{ 6, 7 }));
        guest5 = std::make_shared<vmp::Guest>(std::move(std::set{ 6, 7, 8 }));

        testGuests = { guest1, guest2, guest3, guest4, guest5 };
    }

    std::shared_ptr<vmp::Guest> guest1, guest2, guest3, guest4, guest5;
    std::vector<std::shared_ptr<vmp::Guest>> testGuests;
};

TEST_F(SolverUtilsTest, CalculatePageFrequenciesBasicTest)
{
    const auto frequencies =
        calculatePageFrequencies(testGuests.begin(), testGuests.end());

    EXPECT_EQ(frequencies.at(1), 2);
    EXPECT_EQ(frequencies.at(2), 2);
    EXPECT_EQ(frequencies.at(3), 3);
    EXPECT_EQ(frequencies.at(4), 2);
    EXPECT_EQ(frequencies.at(5), 2);
    EXPECT_EQ(frequencies.at(6), 2);
    EXPECT_EQ(frequencies.at(7), 2);
    EXPECT_EQ(frequencies.at(8), 1);
}

TEST_F(SolverUtilsTest, CalculatePageFrequenciesEmptyTest)
{
    std::vector<std::shared_ptr<vmp::Guest>> emptyGuests;
    const auto frequencies =
        calculatePageFrequencies(emptyGuests.begin(), emptyGuests.end());

    EXPECT_TRUE(frequencies.empty());
}

TEST_F(SolverUtilsTest, RelativeSizeBasicTest)
{
    const auto frequencies =
        calculatePageFrequencies(testGuests.begin(), testGuests.end());

    constexpr double expectedRelativeSize =
        1. / 2. + 1. / 2. + 1. / 3. + 1. / 2.;
    const double actualRelativeSize = calculateRelSize(guest1, frequencies);

    EXPECT_NEAR(actualRelativeSize, expectedRelativeSize, 0.0001);
}

TEST_F(SolverUtilsTest, SizeOverRelativeSizeBasicTest)
{
    const auto frequencies =
        calculatePageFrequencies(testGuests.begin(), testGuests.end());

    const double relSize = calculateRelSize(guest1, frequencies);
    const double expectedRatio = 4.0 / relSize;
    const double actualRatio = calculateSizeRelRatio(guest1, frequencies);

    EXPECT_NEAR(actualRatio, expectedRatio, 0.0001);
}

TEST_F(SolverUtilsTest, OverlappingGuestsTest)
{
    const auto singlePageGuests =
        std::vector{ std::make_shared<vmp::Guest>(std::set{ 5 }),
                     std::make_shared<vmp::Guest>(std::set{ 5 }) };

    const auto frequencies = calculatePageFrequencies(singlePageGuests.begin(),
                                                      singlePageGuests.end());

    EXPECT_EQ(frequencies.at(5), 2);

    constexpr double expectedRelativeSize = 1.0 / 2.0;
    EXPECT_NEAR(calculateRelSize(singlePageGuests.front(), frequencies),
                expectedRelativeSize, 0.0001);
}

TEST_F(SolverUtilsTest, PartitionComponentsBasicTest)
{
    const auto components =
        makeShareGraphComponentGuestPartitions(testGuests.begin(), testGuests.end());
    const auto expectedComponents = std::vector{
        std::vector{ guest4, guest5 },
        std::vector{ guest3, guest1, guest2 },
    };

    EXPECT_THAT(components,
                testing::UnorderedElementsAre(
                    testing::UnorderedElementsAre(guest4, guest5),
                    testing::UnorderedElementsAre(guest3, guest1, guest2)));
}
