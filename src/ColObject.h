#ifndef _INCL_COLOBJECT
#define _INCL_COLOBJECT

#include <btBulletDynamicsCommon.h>
#include "ColAndreasDatabaseReader.h"
#include "ColAndreas.h"
#include "LodArray.h"

#define MAX_MAP_OBJECTS 50000

struct removeBuildingData
{
	int16_t r_Model;
	float r_X;
	float r_Y;
	float r_Z;
	float r_Radius;
};

static uint16_t colindex = 0;
 		 
// Data structure to track in-game objects with respect to their colindex
struct ColAndreasObjectTracker
{
    int32_t realIndex;
    int32_t extraData[10];

    ColAndreasObjectTracker(){
        realIndex = -1;
        for(int i = 0; i < 10; i++)
            extraData[i] = -1;
    }
    ~ColAndreasObjectTracker() {
        realIndex = -1;
        for(int i = 0; i < 10; i++)
            extraData[i] = -1;
    }
};

// Collision Objects
class ColAndreasColObject
{
public:
	ColAndreasColObject(uint16_t colindex, bool thirdparty);
	ColAndreasColObject(ColAndreasColObject* source);
	~ColAndreasColObject();
	btCollisionShape* getCollisionShape();
private:
	// Object Data
	btCompoundShape* colMapObject;
	btCollisionShape* collisionShape;

	// Mesh
	btTriangleMesh* trimesh;
	btCollisionShape* meshshape;
	bool ownsTriangleMesh;
	std::vector <btSphereShape*> spheres;
	std::vector <btBoxShape*> boxes;
};

// Map Objects
class ColAndreasMapObject
{
public:
	ColAndreasMapObject(int32_t modelid, const btQuaternion& objectRot, const btVector3& objectPos, btDynamicsWorld* world, bool createTracker);
	~ColAndreasMapObject();
	void setMapObjectPosition(btVector3& position);
	void setMapObjectRotation(btQuaternion& rotation);
	ColAndreasObjectTracker* tracker;
private:
	// Object Data
	btCollisionObject* colMapObject;
	btDynamicsWorld* collisionWorld;
};


class MapWaterMesh
{
public:
	MapWaterMesh(btDynamicsWorld* world);
	~MapWaterMesh();
private:
	btTriangleMesh* trimesh;
	btBvhTriangleMeshShape* meshshape;
	btCollisionObject* meshObject;
	btDynamicsWorld* collisionWorld;
};


class ObjectManager
{
public:
	ObjectManager();
	~ObjectManager();
	int addObjectManager(ColAndreasMapObject* mapObject);
	int removeObjectManager(const uint16_t index);
	int validObjectManager(const uint16_t index);
	int setObjectPosition(const uint16_t index, btVector3& position);
	int setObjectRotation(const uint16_t index, btQuaternion& rotation);
	int getBoundingSphere(int32_t modelid, btVector3& center, btScalar& radius);
	int getBoundingBox(int32_t modelid, btVector3& min, btVector3& max);
	int setExtraID(const uint16_t index, int type, int data);
	int getExtraID(const uint16_t index, int type);
private:
	bool slotUsed[MAX_MAP_OBJECTS];
	ColAndreasMapObject* mapObjects[MAX_MAP_OBJECTS];

};


class RemovedBuildingManager
{
public:
	RemovedBuildingManager();
	bool isRemoved(int16_t model, Vector position);
	void addBuilding(removeBuildingData removeData);
	void restoreBuilding(removeBuildingData targetData);
private:
	std::vector <removeBuildingData> removedBuildings;
};

bool LoadCollisionData(btDynamicsWorld* collisionWorld);
btCollisionObject* CreateStaticCollisionObject(int32_t modelid, const btQuaternion& objectRot, const btVector3& objectPos, btDynamicsWorld* world);
void InitCollisionMap(btDynamicsWorld* collisionWorld, RemovedBuildingManager* removeManager);
uint16_t GetModelRef(int32_t model);

// Pointer reference
extern std::vector <ColAndreasColObject*> colObjects;
extern std::vector <ColAndreasColObject*> colConvex; //for the sake of contact tests

typedef struct {
	btVector3 pos;
	btScalar dist;
} btMultiCast;


#endif
