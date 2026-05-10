#include <gtest/gtest.h>
#include "ColAndreasDatabaseReader.h"
#include "ColObject.h"

class ModelRefFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset all lookup tables to 65535 (= "not found")
        DeleteCollisionData();
    }
};

TEST_F(ModelRefFixture, UnknownStandardModelReturns65535) {
    EXPECT_EQ(LookupModelRef(1000), static_cast<uint16_t>(65535));
}

TEST_F(ModelRefFixture, SetAndLookupStandardRange) {
    SetModelRef(1337, 42);
    EXPECT_EQ(LookupModelRef(1337), static_cast<uint16_t>(42));
}

TEST_F(ModelRefFixture, SetAndLookupModelZero) {
    SetModelRef(0, 0);
    EXPECT_EQ(LookupModelRef(0), static_cast<uint16_t>(0));
}

TEST_F(ModelRefFixture, SetAndLookupModelMax) {
    SetModelRef(20000, 100);
    EXPECT_EQ(LookupModelRef(20000), static_cast<uint16_t>(100));
}

TEST_F(ModelRefFixture, SetAndLookupCustomNegativeModel) {
    // Custom model IDs are stored in a separate map
    SetModelRef(-1000, 5);
    EXPECT_EQ(LookupModelRef(-1000), static_cast<uint16_t>(5));
}

TEST_F(ModelRefFixture, UnknownCustomModelReturns65535) {
    EXPECT_EQ(LookupModelRef(-5000), static_cast<uint16_t>(65535));
}

TEST_F(ModelRefFixture, GetModelRef_OutOfRangeReturns65535) {
    // GetModelRef enforces valid SA-MP ranges; 20001 is outside valid standard range
    EXPECT_EQ(GetModelRef(20001), static_cast<uint16_t>(65535));
    // Below -29999 is outside valid custom range
    EXPECT_EQ(GetModelRef(-30000), static_cast<uint16_t>(65535));
}

TEST_F(ModelRefFixture, GetModelRef_ValidStandardRange) {
    SetModelRef(500, 7);
    EXPECT_EQ(GetModelRef(500), static_cast<uint16_t>(7));
}

TEST_F(ModelRefFixture, GetModelRef_ValidCustomRange) {
    SetModelRef(-1500, 3);
    EXPECT_EQ(GetModelRef(-1500), static_cast<uint16_t>(3));
}

TEST_F(ModelRefFixture, DeleteCollisionData_ResetsRefs) {
    SetModelRef(100, 10);
    EXPECT_EQ(LookupModelRef(100), static_cast<uint16_t>(10));
    DeleteCollisionData();
    EXPECT_EQ(LookupModelRef(100), static_cast<uint16_t>(65535));
}
