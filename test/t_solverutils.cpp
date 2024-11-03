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

        testGuests = { guest1, guest2, guest3 };
    }

    std::shared_ptr<vmp::Guest> guest1;
    std::shared_ptr<vmp::Guest> guest2;
    std::shared_ptr<vmp::Guest> guest3;
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
        1.0 / 2.0 + 1.0 / 2.0 + 1.0 / 3.0 + 1.0 / 2.0;
    const double actualRelativeSize = relativeSize(guest1, frequencies);

    EXPECT_NEAR(actualRelativeSize, expectedRelativeSize, 0.0001);
}

TEST_F(SolverUtilsTest, SizeOverRelativeSizeBasicTest)
{
    const auto frequencies =
        calculatePageFrequencies(testGuests.begin(), testGuests.end());

    const double relSize = relativeSize(guest1, frequencies);
    const double expectedRatio = 4.0 / relSize;
    const double actualRatio = sizeOverRelativeSize(guest1, frequencies);

    EXPECT_NEAR(actualRatio, expectedRatio, 0.0001);
}

TEST_F(SolverUtilsTest, SingleGuestTest)
{
    const auto singleGuest = std::vector{ guest1 };
    const auto frequencies =
        calculatePageFrequencies(singleGuest.begin(), singleGuest.end());

    EXPECT_EQ(frequencies.at(1), 1);
    EXPECT_EQ(frequencies.at(2), 1);
    EXPECT_EQ(frequencies.at(3), 1);
}

TEST_F(SolverUtilsTest, OverlappingGuestsTest)
{
    const auto guest4 = std::make_shared<vmp::Guest>(std::set{ 5 });
    const auto guest5 = std::make_shared<vmp::Guest>(std::set{ 5 });
    const auto singlePageGuests = std::vector{ guest4, guest5 };

    auto frequencies = calculatePageFrequencies(singlePageGuests.begin(),
                                                singlePageGuests.end());

    EXPECT_EQ(frequencies[5], 2);

    constexpr double expectedRelativeSize = 1.0 / 2.0;
    EXPECT_NEAR(relativeSize(guest4, frequencies), expectedRelativeSize,
                0.0001);
}
