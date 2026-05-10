#include <gtest/gtest.h>
#include "ColAndreasDatabaseReader.h"
#include <fstream>
#include <cstdio>
#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers to build a minimal in-memory .cadb binary
// ---------------------------------------------------------------------------

static void appendU16(std::vector<uint8_t>& v, uint16_t val) {
    v.push_back(static_cast<uint8_t>(val & 0xFF));
    v.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
}

static void appendU32(std::vector<uint8_t>& v, uint32_t val) {
    v.push_back(static_cast<uint8_t>(val & 0xFF));
    v.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
    v.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
    v.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
}

static void appendF32(std::vector<uint8_t>& v, float val) {
    uint8_t bytes[4];
    memcpy(bytes, &val, 4);
    v.insert(v.end(), bytes, bytes + 4);
}

static void appendHeader(std::vector<uint8_t>& v, uint16_t modelCount, uint32_t iplCount) {
    v.push_back('c'); v.push_back('a'); v.push_back('d'); v.push_back('f');
    appendU16(v, 2); // CA_DATABASE_VERSION
    appendU16(v, modelCount);
    appendU32(v, iplCount);
}

static void appendModel(std::vector<uint8_t>& v,
                         uint16_t modelid,
                         uint16_t spheres, uint16_t boxes, uint16_t faces) {
    appendU16(v, modelid);
    appendU16(v, spheres);
    appendU16(v, boxes);
    appendU16(v, faces);
    // sphere data: Vector(xyz) + radius = 4 floats = 16 bytes each
    for (uint16_t i = 0; i < spheres; i++) {
        appendF32(v, 0.0f); appendF32(v, 0.0f); appendF32(v, 0.0f);
        appendF32(v, 1.0f);
    }
    // box data: Vector center(xyz) + Vector size(xyz) = 6 floats = 24 bytes each
    for (uint16_t i = 0; i < boxes; i++) {
        appendF32(v, 0.0f); appendF32(v, 0.0f); appendF32(v, 0.0f);
        appendF32(v, 1.0f); appendF32(v, 1.0f); appendF32(v, 1.0f);
    }
    // face data: 3 vertices × 3 floats = 9 floats = 36 bytes each
    for (uint16_t i = 0; i < faces; i++) {
        for (int j = 0; j < 9; j++) appendF32(v, 0.0f);
    }
}

static void appendIPL(std::vector<uint8_t>& v,
                       uint16_t modelid,
                       float px, float py, float pz,
                       float qx, float qy, float qz, float qw) {
    appendU16(v, modelid);
    appendF32(v, px); appendF32(v, py); appendF32(v, pz);
    appendF32(v, qx); appendF32(v, qy); appendF32(v, qz); appendF32(v, qw);
}

// RAII wrapper: writes bytes to a file, removes it on destruction.
struct TempCadb {
    std::string path;
    TempCadb(const char* name, const std::vector<uint8_t>& data) : path(name) {
        std::ofstream f(name, std::ios::binary);
        f.write(reinterpret_cast<const char*>(data.data()),
                static_cast<std::streamsize>(data.size()));
    }
    ~TempCadb() { std::remove(path.c_str()); }
};

// ---------------------------------------------------------------------------
// Test fixture: resets global state before and after each test
// ---------------------------------------------------------------------------

class DbReaderFixture : public ::testing::Test {
protected:
    void SetUp() override    { DeleteCollisionData(); }
    void TearDown() override { DeleteCollisionData(); }
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_F(DbReaderFixture, NonExistentFile_ReturnsFalse) {
    EXPECT_FALSE(ReadColandreasDatabaseFile("__nonexistent_cadb_file__.cadb"));
}

TEST_F(DbReaderFixture, WrongMagic_ReturnsFalse) {
    std::vector<uint8_t> data;
    data.push_back('X'); data.push_back('X'); data.push_back('X'); data.push_back('X');
    appendU16(data, 2);
    appendU16(data, 0);
    appendU32(data, 0);
    TempCadb tmp("__test_badmagic.cadb", data);
    EXPECT_FALSE(ReadColandreasDatabaseFile(tmp.path));
}

TEST_F(DbReaderFixture, WrongVersion_ReturnsFalse) {
    std::vector<uint8_t> data;
    data.push_back('c'); data.push_back('a'); data.push_back('d'); data.push_back('f');
    appendU16(data, 99); // wrong version
    appendU16(data, 0);
    appendU32(data, 0);
    TempCadb tmp("__test_badversion.cadb", data);
    EXPECT_FALSE(ReadColandreasDatabaseFile(tmp.path));
}

TEST_F(DbReaderFixture, EmptyDb_ReturnsTrue) {
    std::vector<uint8_t> data;
    appendHeader(data, 0, 0);
    TempCadb tmp("__test_empty.cadb", data);

    EXPECT_TRUE(ReadColandreasDatabaseFile(tmp.path));
    EXPECT_EQ(ModelCount, static_cast<uint16_t>(0));
    EXPECT_EQ(IPLCount,   static_cast<uint32_t>(0));
}

TEST_F(DbReaderFixture, OneModel_SphereOnly) {
    std::vector<uint8_t> data;
    appendHeader(data, 1, 0);
    appendModel(data, /*modelid=*/100, /*spheres=*/1, /*boxes=*/0, /*faces=*/0);
    TempCadb tmp("__test_onemodel.cadb", data);

    EXPECT_TRUE(ReadColandreasDatabaseFile(tmp.path));
    EXPECT_EQ(ModelCount, static_cast<uint16_t>(1));
    EXPECT_EQ(CollisionModels[0].Modelid,     static_cast<uint16_t>(100));
    EXPECT_EQ(CollisionModels[0].SphereCount, static_cast<uint16_t>(1));
    EXPECT_EQ(CollisionModels[0].BoxCount,    static_cast<uint16_t>(0));
    EXPECT_EQ(CollisionModels[0].FaceCount,   static_cast<uint16_t>(0));
}

TEST_F(DbReaderFixture, OneModel_BoxOnly) {
    std::vector<uint8_t> data;
    appendHeader(data, 1, 0);
    appendModel(data, 200, 0, 1, 0);
    TempCadb tmp("__test_onebox.cadb", data);

    EXPECT_TRUE(ReadColandreasDatabaseFile(tmp.path));
    EXPECT_EQ(CollisionModels[0].BoxCount,    static_cast<uint16_t>(1));
    EXPECT_EQ(CollisionModels[0].SphereCount, static_cast<uint16_t>(0));
}

TEST_F(DbReaderFixture, ModelRefPopulatedAfterLoad) {
    std::vector<uint8_t> data;
    appendHeader(data, 1, 0);
    appendModel(data, 300, 1, 0, 0);
    TempCadb tmp("__test_modelref.cadb", data);

    EXPECT_TRUE(ReadColandreasDatabaseFile(tmp.path));
    // After load, model 300 must map to index 0
    EXPECT_EQ(LookupModelRef(300), static_cast<uint16_t>(0));
}

TEST_F(DbReaderFixture, WithIPL_ParsedCorrectly) {
    std::vector<uint8_t> data;
    appendHeader(data, 1, 1);
    appendModel(data, 100, 1, 0, 0);
    // IPL entry: model 100 at (1, 2, 3), identity quaternion
    appendIPL(data, 100, 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    TempCadb tmp("__test_ipl.cadb", data);

    EXPECT_TRUE(ReadColandreasDatabaseFile(tmp.path));
    EXPECT_EQ(IPLCount, static_cast<uint32_t>(1));
    EXPECT_EQ(ModelPlacements[0].Modelid, static_cast<uint16_t>(100));
    EXPECT_NEAR(ModelPlacements[0].Position.x, 1.0f, 1e-5f);
    EXPECT_NEAR(ModelPlacements[0].Position.y, 2.0f, 1e-5f);
    EXPECT_NEAR(ModelPlacements[0].Position.z, 3.0f, 1e-5f);
    EXPECT_NEAR(ModelPlacements[0].Rotation.w, 1.0f, 1e-5f);
}

TEST_F(DbReaderFixture, MultipleModels) {
    std::vector<uint8_t> data;
    appendHeader(data, 3, 0);
    appendModel(data, 10, 1, 0, 0);
    appendModel(data, 20, 0, 2, 0);
    appendModel(data, 30, 0, 0, 1);
    TempCadb tmp("__test_multimodel.cadb", data);

    EXPECT_TRUE(ReadColandreasDatabaseFile(tmp.path));
    EXPECT_EQ(ModelCount, static_cast<uint16_t>(3));
    EXPECT_EQ(CollisionModels[0].Modelid, static_cast<uint16_t>(10));
    EXPECT_EQ(CollisionModels[1].Modelid, static_cast<uint16_t>(20));
    EXPECT_EQ(CollisionModels[2].Modelid, static_cast<uint16_t>(30));
    EXPECT_EQ(LookupModelRef(10), static_cast<uint16_t>(0));
    EXPECT_EQ(LookupModelRef(20), static_cast<uint16_t>(1));
    EXPECT_EQ(LookupModelRef(30), static_cast<uint16_t>(2));
}
