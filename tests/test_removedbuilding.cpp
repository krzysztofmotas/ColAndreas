#include <gtest/gtest.h>
#include "ColObject.h"

// --- ColAndreasObjectTracker ---

TEST(ObjectTrackerTest, DefaultValues) {
    ColAndreasObjectTracker tracker;
    EXPECT_EQ(tracker.realIndex, -1);
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(tracker.extraData[i], -1) << "slot " << i;
    }
}

// --- RemovedBuildingManager ---

static removeBuildingData makeEntry(int16_t model, float x, float y, float z, float radius) {
    removeBuildingData d;
    d.r_Model  = model;
    d.r_X      = x;
    d.r_Y      = y;
    d.r_Z      = z;
    d.r_Radius = radius;
    return d;
}

TEST(RemovedBuildingTest, EmptyManagerNeverRemoved) {
    RemovedBuildingManager mgr;
    Vector pos = {100.0f, 200.0f, 10.0f};
    EXPECT_FALSE(mgr.isRemoved(100, pos));
}

TEST(RemovedBuildingTest, AddThenCheckAtCenter) {
    RemovedBuildingManager mgr;
    mgr.addBuilding(makeEntry(100, 0.0f, 0.0f, 0.0f, 50.0f));

    Vector inside = {0.0f, 0.0f, 0.0f};
    EXPECT_TRUE(mgr.isRemoved(100, inside));
}

TEST(RemovedBuildingTest, PositionOutsideRadius) {
    RemovedBuildingManager mgr;
    mgr.addBuilding(makeEntry(100, 0.0f, 0.0f, 0.0f, 50.0f));

    Vector outside = {100.0f, 0.0f, 0.0f}; // 100 units away, radius = 50
    EXPECT_FALSE(mgr.isRemoved(100, outside));
}

TEST(RemovedBuildingTest, WrongModelNotRemoved) {
    RemovedBuildingManager mgr;
    mgr.addBuilding(makeEntry(100, 0.0f, 0.0f, 0.0f, 50.0f));

    Vector inside = {0.0f, 0.0f, 0.0f};
    EXPECT_FALSE(mgr.isRemoved(999, inside));
}

TEST(RemovedBuildingTest, ModelMinusOneMatchesAnyWithinRadius) {
    RemovedBuildingManager mgr;
    mgr.addBuilding(makeEntry(100, 0.0f, 0.0f, 0.0f, 10.0f));

    Vector inside = {0.0f, 0.0f, 0.0f};
    // model == -1 acts as wildcard matching any stored entry
    EXPECT_TRUE(mgr.isRemoved(-1, inside));
}

TEST(RemovedBuildingTest, MultipleEntries) {
    RemovedBuildingManager mgr;
    mgr.addBuilding(makeEntry(100, 0.0f,   0.0f,   0.0f, 10.0f));
    mgr.addBuilding(makeEntry(200, 500.0f, 500.0f, 0.0f, 10.0f));

    Vector nearFirst  = {2.0f, 0.0f, 0.0f};
    Vector nearSecond = {500.0f, 502.0f, 0.0f};

    EXPECT_TRUE(mgr.isRemoved(100, nearFirst));
    EXPECT_TRUE(mgr.isRemoved(200, nearSecond));
    EXPECT_FALSE(mgr.isRemoved(100, nearSecond));
    EXPECT_FALSE(mgr.isRemoved(200, nearFirst));
}

TEST(RemovedBuildingTest, ExactlyOnRadiusBoundary) {
    RemovedBuildingManager mgr;
    mgr.addBuilding(makeEntry(1, 0.0f, 0.0f, 0.0f, 10.0f));

    // Distance == radius → should be considered removed (dist <= radius)
    Vector onEdge = {10.0f, 0.0f, 0.0f};
    EXPECT_TRUE(mgr.isRemoved(1, onEdge));

    Vector justOutside = {10.01f, 0.0f, 0.0f};
    EXPECT_FALSE(mgr.isRemoved(1, justOutside));
}
