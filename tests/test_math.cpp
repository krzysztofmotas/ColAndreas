#include <gtest/gtest.h>
#include "DynamicWorld.h"

class MathFixture : public ::testing::Test {
protected:
    ColAndreasWorld* world = nullptr;

    void SetUp() override {
        world = new ColAndreasWorld();
    }
    void TearDown() override {
        delete world;
    }
};

// --- getDist3D ---

TEST_F(MathFixture, GetDist3D_SamePoint) {
    btVector3 p(1.0f, 2.0f, 3.0f);
    EXPECT_FLOAT_EQ(world->getDist3D(p, p), 0.0f);
}

TEST_F(MathFixture, GetDist3D_AlongXAxis) {
    EXPECT_NEAR(world->getDist3D(btVector3(0,0,0), btVector3(7,0,0)), 7.0f, 1e-4f);
}

TEST_F(MathFixture, GetDist3D_3DPythagorean) {
    // sqrt(1^2 + 2^2 + 2^2) == 3
    EXPECT_NEAR(world->getDist3D(btVector3(0,0,0), btVector3(1,2,2)), 3.0f, 1e-4f);
}

TEST_F(MathFixture, GetDist3D_Symmetric) {
    btVector3 a(1,2,3), b(4,6,3);
    EXPECT_FLOAT_EQ(world->getDist3D(a, b), world->getDist3D(b, a));
}

// --- EulerToQuat ---

TEST_F(MathFixture, EulerToQuat_Identity) {
    btVector3 rot(0.0f, 0.0f, 0.0f);
    btQuaternion q;
    world->EulerToQuat(rot, q);
    EXPECT_NEAR(q.getW(), 1.0f, 1e-5f);
    EXPECT_NEAR(q.getX(), 0.0f, 1e-5f);
    EXPECT_NEAR(q.getY(), 0.0f, 1e-5f);
    EXPECT_NEAR(q.getZ(), 0.0f, 1e-5f);
}

TEST_F(MathFixture, EulerToQuat_90DegX) {
    // X=90, Y=0, Z=0 → q = (x≈0.7071, y=0, z=0, w≈0.7071)
    btVector3 rot(90.0f, 0.0f, 0.0f);
    btQuaternion q;
    world->EulerToQuat(rot, q);
    EXPECT_NEAR(q.getW(), 0.7071f, 1e-3f);
    EXPECT_NEAR(q.getX(), 0.7071f, 1e-3f);
    EXPECT_NEAR(q.getY(), 0.0f,    1e-3f);
    EXPECT_NEAR(q.getZ(), 0.0f,    1e-3f);
}

TEST_F(MathFixture, EulerToQuat_ProducesUnitQuaternion) {
    btVector3 rot(30.0f, 45.0f, 60.0f);
    btQuaternion q;
    world->EulerToQuat(rot, q);
    float len = q.getX()*q.getX() + q.getY()*q.getY() +
                q.getZ()*q.getZ() + q.getW()*q.getW();
    EXPECT_NEAR(len, 1.0f, 1e-4f);
}

// --- QuatToEuler ---

TEST_F(MathFixture, QuatToEuler_Identity) {
    btQuaternion q(0.0f, 0.0f, 0.0f, 1.0f);
    btVector3 euler;
    world->QuatToEuler(q, euler);
    EXPECT_NEAR(euler.getX(), 0.0f, 1e-3f);
    EXPECT_NEAR(euler.getY(), 0.0f, 1e-3f);
    EXPECT_NEAR(euler.getZ(), 0.0f, 1e-3f);
}

// --- EulerToQuat / QuatToEuler roundtrip ---

TEST_F(MathFixture, EulerQuat_Roundtrip_XOnly) {
    btVector3 original(45.0f, 0.0f, 0.0f);
    btVector3 rot = original;
    btQuaternion q;
    world->EulerToQuat(rot, q);
    btVector3 result;
    world->QuatToEuler(q, result);
    EXPECT_NEAR(result.getX(), original.getX(), 0.01f);
    EXPECT_NEAR(result.getY(), original.getY(), 0.01f);
    EXPECT_NEAR(result.getZ(), original.getZ(), 0.01f);
}

TEST_F(MathFixture, EulerQuat_Roundtrip_YOnly) {
    btVector3 original(0.0f, 60.0f, 0.0f);
    btVector3 rot = original;
    btQuaternion q;
    world->EulerToQuat(rot, q);
    btVector3 result;
    world->QuatToEuler(q, result);
    EXPECT_NEAR(result.getX(), original.getX(), 0.01f);
    EXPECT_NEAR(result.getY(), original.getY(), 0.01f);
    EXPECT_NEAR(result.getZ(), original.getZ(), 0.01f);
}

TEST_F(MathFixture, EulerQuat_Roundtrip_ZOnly) {
    btVector3 original(0.0f, 0.0f, 30.0f);
    btVector3 rot = original;
    btQuaternion q;
    world->EulerToQuat(rot, q);
    btVector3 result;
    world->QuatToEuler(q, result);
    EXPECT_NEAR(result.getX(), original.getX(), 0.01f);
    EXPECT_NEAR(result.getY(), original.getY(), 0.01f);
    EXPECT_NEAR(result.getZ(), original.getZ(), 0.01f);
}
