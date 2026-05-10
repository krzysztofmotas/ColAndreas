#include "ColObject.h"
#include "WaterArray.h"
#include "DynamicWorld.h"
#include "ColAndreasDatabaseReader.h"

// SAMP objects only go up 19999
#define WATER_MESH_ID 20000

std::vector <ColAndreasColObject*> colObjects;
std::vector <ColAndreasColObject*> colConvex;

ColAndreasColObject::ColAndreasColObject(uint16_t colindex, bool thirdparty = false)
{
	bool useCompound = CollisionModels[colindex].SphereCount > 0 || CollisionModels[colindex].BoxCount > 0 || CollisionModels[colindex].FaceCount == 0;
	colMapObject = useCompound ? new btCompoundShape() : NULL;
	collisionShape = NULL;
	trimesh = NULL;
	meshshape = NULL;
	ownsTriangleMesh = true;

	// Build any spheres
	for (uint16_t i = 0; i < CollisionModels[colindex].SphereCount; i++)
	{
		btSphereShape* sphere = new btSphereShape(CollisionModels[colindex].SphereData[i].Radius);
		colMapObject->addChildShape(btTransform(btQuaternion(0, 0, 0, 1), btVector3(CollisionModels[colindex].SphereData[i].Offset.x, CollisionModels[colindex].SphereData[i].Offset.y, CollisionModels[colindex].SphereData[i].Offset.z)), sphere);
		spheres.push_back(sphere);
	}

	for (uint16_t i = 0; i < CollisionModels[colindex].BoxCount; i++)
	{
		// Create a box shape
		btBoxShape* box = new btBoxShape(btVector3(CollisionModels[colindex].BoxData[i].Size.x, CollisionModels[colindex].BoxData[i].Size.y, CollisionModels[colindex].BoxData[i].Size.z));
		colMapObject->addChildShape(btTransform(btQuaternion(0, 0, 0, 1), btVector3(CollisionModels[colindex].BoxData[i].Center.x, CollisionModels[colindex].BoxData[i].Center.y, CollisionModels[colindex].BoxData[i].Center.z)), box);
		boxes.push_back(box);
	}

	if (CollisionModels[colindex].FaceCount > 0)
	{
		// Create a triangular mesh
		trimesh = new btTriangleMesh(CollisionModels[colindex].FaceCount > 21845, false);
		for (int i = 0; i < CollisionModels[colindex].FaceCount; i++)
		{
			// Add triangle faces
			trimesh->addTriangle(btVector3(CollisionModels[colindex].FacesData[i].FaceA.x, CollisionModels[colindex].FacesData[i].FaceA.y, CollisionModels[colindex].FacesData[i].FaceA.z),
				btVector3(CollisionModels[colindex].FacesData[i].FaceB.x, CollisionModels[colindex].FacesData[i].FaceB.y, CollisionModels[colindex].FacesData[i].FaceB.z),
				btVector3(CollisionModels[colindex].FacesData[i].FaceC.x, CollisionModels[colindex].FacesData[i].FaceC.y, CollisionModels[colindex].FacesData[i].FaceC.z));
		}
		if (thirdparty) //will be true for convex objects
		{
			meshshape = new btConvexTriangleMeshShape(trimesh);
		}
		else
		{
			meshshape = new btBvhTriangleMeshShape(trimesh, true);
		}
		if (useCompound)
		{
			colMapObject->addChildShape(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)), meshshape);
		}
		else
		{
			collisionShape = meshshape;
		}
	}

	if (collisionShape == NULL)
	{
		collisionShape = colMapObject;
	}
}

ColAndreasColObject::ColAndreasColObject(ColAndreasColObject* source)
{
	colMapObject = source->colMapObject != NULL ? new btCompoundShape() : NULL;
	collisionShape = NULL;
	trimesh = source->trimesh;
	meshshape = NULL;
	ownsTriangleMesh = false;

	if (source->colMapObject == NULL)
	{
		if (trimesh != NULL)
		{
			meshshape = new btConvexTriangleMeshShape(trimesh);
			collisionShape = meshshape;
		}
		else
		{
			collisionShape = source->collisionShape;
		}
		return;
	}

	for (int i = 0; i < source->colMapObject->getNumChildShapes(); i++)
	{
		btCollisionShape* childShape = source->colMapObject->getChildShape(i);
		btTransform childTransform = source->colMapObject->getChildTransform(i);

		if (childShape == source->meshshape && trimesh != NULL)
		{
			meshshape = new btConvexTriangleMeshShape(trimesh);
			colMapObject->addChildShape(childTransform, meshshape);
		}
		else
		{
			colMapObject->addChildShape(childTransform, childShape);
		}
	}
	collisionShape = colMapObject;
}

ColAndreasColObject::~ColAndreasColObject()
{	
	delete meshshape;
	if (ownsTriangleMesh)
	{
		delete trimesh;
	}
	for (uint16_t i = 0; i < boxes.size(); i++)
	{
		delete boxes[i];
	}
	for (uint16_t i = 0; i < spheres.size(); i++)
	{
		delete spheres[i];
	}
	delete colMapObject;
}


btCollisionShape* ColAndreasColObject::getCollisionShape()
{
	return collisionShape;
}


bool LoadCollisionData(btDynamicsWorld* collisionWorld)
{
	if (ReadColandreasDatabaseFile("scriptfiles/colandreas/ColAndreas.cadb"))
	{
		colObjects.reserve(ModelCount);
		colConvex.reserve(ModelCount);
		for (uint16_t i = 0; i < ModelCount; i++)
		{
			if (i % 100 == 0)
			{
				printf("\33Loading: %0.1f\r", ((double)i / ModelCount) * 100);
			}
			ColAndreasColObject* colObject = new ColAndreasColObject(i);
			colObjects.push_back(colObject);
			colConvex.push_back(NULL);
		}
		FreeCollisionModelGeometry();
		return true;
	}
	return false;
}

btCollisionObject* CreateStaticCollisionObject(int32_t modelid, const btQuaternion& objectRot, const btVector3& objectPos, btDynamicsWorld* world)
{
	uint16_t modelRef = GetModelRef(modelid);
	btCollisionObject* collisionObject = new btCollisionObject();
	collisionObject->setCollisionShape(colObjects[modelRef]->getCollisionShape());
	collisionObject->setWorldTransform(btTransform(objectRot, objectPos));
	collisionObject->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
	collisionObject->setUserIndex(modelid);
	world->addCollisionObject(collisionObject);
	return collisionObject;
}


ColAndreasMapObject::ColAndreasMapObject(int32_t modelid, const btQuaternion& objectRot, const btVector3& objectPos, btDynamicsWorld* world, bool createTracker)
{
	colindex = GetModelRef(modelid);
	tracker = NULL;

	collisionWorld = world;
	colMapObject = CreateStaticCollisionObject(modelid, objectRot, objectPos, world);
	
	if (createTracker)
	{
		colMapObject->internalSetExtensionPointer(this);
		tracker = new ColAndreasObjectTracker();
		colMapObject->setUserPointer(tracker);
	}
}

ColAndreasMapObject::~ColAndreasMapObject()
{
	if (tracker != NULL)
	{
		delete tracker;
	}

	collisionWorld->removeCollisionObject(colMapObject);
	delete colMapObject;
}

void ColAndreasMapObject::setMapObjectPosition(btVector3& position)
{
	colMapObject->setWorldTransform(btTransform(colMapObject->getWorldTransform().getRotation(), position));
	collisionWorld->removeCollisionObject(colMapObject);
	collisionWorld->addCollisionObject(colMapObject);
}


void ColAndreasMapObject::setMapObjectRotation(btQuaternion& rotation)
{
	colMapObject->setWorldTransform(btTransform(rotation, colMapObject->getWorldTransform().getOrigin()));
	collisionWorld->removeCollisionObject(colMapObject);
	collisionWorld->addCollisionObject(colMapObject);
}


MapWaterMesh::MapWaterMesh(btDynamicsWorld* world)
{
	collisionWorld = world;

	// Create a triangular mesh
	trimesh = new btTriangleMesh(false, false);

	for (uint16_t i = 0; i < 616; i++)
	{
		trimesh->addTriangle(btVector3(btScalar(waterData[i][0]), btScalar(waterData[i][1]), btScalar(waterData[i][2])),
			btVector3(btScalar(waterData[i][3]), btScalar(waterData[i][4]), btScalar(waterData[i][5])),
			btVector3(btScalar(waterData[i][6]), btScalar(waterData[i][7]), btScalar(waterData[i][8])));
	}

	meshshape = new btBvhTriangleMeshShape(trimesh, true);
	meshObject = new btCollisionObject();
	meshObject->setCollisionShape(meshshape);
	meshObject->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	meshObject->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);

	// Water mesh is defined as 20000
	meshObject->setUserIndex(WATER_MESH_ID);

	collisionWorld->addCollisionObject(meshObject);

}

MapWaterMesh::~MapWaterMesh()
{
	collisionWorld->removeCollisionObject(meshObject);
	delete meshObject;
	delete meshshape;
	delete trimesh;
}



ObjectManager::ObjectManager()
{
	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		slotUsed[i] = false;
	}
}

ObjectManager::~ObjectManager()
{
	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		if (slotUsed[i])
		{
			delete mapObjects[i];
			slotUsed[i] = false;
		}
	}
}
 		 
int ObjectManager::setExtraID(const uint16_t index, int type, int data)
{
	if (slotUsed[index] && type >= 0 && type < 10)
	{
		mapObjects[index]->tracker->extraData[type] = data;
		return 1;
	}
	return 0;
}

int ObjectManager::getExtraID(const uint16_t index, int type)
{
	if (slotUsed[index] && type >= 0 && type < 10)
	{
		return mapObjects[index]->tracker->extraData[type];
	}
	return -1;
}


int ObjectManager::addObjectManager(ColAndreasMapObject* mapObject)
{
	for (int i = 0; i < MAX_MAP_OBJECTS; i++)
	{
		if (!slotUsed[i])
		{
			slotUsed[i] = true;
			mapObjects[i] = mapObject;
			mapObjects[i]->tracker->realIndex = i;
			return i;
		}
	}
	return -1;
}

int ObjectManager::removeObjectManager(const uint16_t index)
{
	if (slotUsed[index])
	{
		slotUsed[index] = false;
		delete mapObjects[index];
		return 1;
	}
	return 0;
}

int ObjectManager::validObjectManager(const uint16_t index)
{
	if (slotUsed[index])
	{
		return 1;
	}
	return 0;
}

int ObjectManager::setObjectPosition(const uint16_t index, btVector3& position)
{
	if (slotUsed[index])
	{
		mapObjects[index]->setMapObjectPosition(position);
		return 1;
	}
	return 0;
}

int ObjectManager::setObjectRotation(const uint16_t index, btQuaternion& rotation)
{
	if (slotUsed[index])
	{
		mapObjects[index]->setMapObjectRotation(rotation);
		return 1;
	}
	return 0;
}


int ObjectManager::getBoundingSphere(int32_t modelid, btVector3& center, btScalar& radius)
{
	uint16_t colindex = GetModelRef(modelid);

	// Check for LOD objects
	if (colindex == 65535 && modelid >= 0 && modelid <= 20000)
	{
		if (LodReference[modelid] > 0)
			colindex = GetModelRef(LodReference[modelid]);
	}


	if (colindex != 65535)
	{
		colObjects[colindex]->getCollisionShape()->getBoundingSphere(center, radius);
		return 1;
	}
	return 0;
}


int ObjectManager::getBoundingBox(int32_t modelid, btVector3& min, btVector3& max)
{
	uint16_t colindex = GetModelRef(modelid);
	btTransform t;
	t.setIdentity();

	// Check for LOD objects
	if (colindex == 65535 && modelid >= 0 && modelid <= 20000)
	{
		if (LodReference[modelid] > 0)
			colindex = GetModelRef(LodReference[modelid]);
	}


	if (colindex != 65535)
	{
		colObjects[colindex]->getCollisionShape()->getAabb(t, min, max);
		return 1;
	}
	return 0;
}


RemovedBuildingManager::RemovedBuildingManager()
{
}

bool RemovedBuildingManager::isRemoved(int16_t model, Vector position)
{
	for (uint16_t i = 0; i < removedBuildings.size(); i++)
	{
		if (model == removedBuildings[i].r_Model || model == -1)
		{
			btScalar dist = btDistance(btVector3(btScalar(position.x), btScalar(position.y), btScalar(position.z)),
				btVector3(btScalar(removedBuildings[i].r_X), btScalar(removedBuildings[i].r_Y), btScalar(removedBuildings[i].r_Z)));

			if (dist <= btScalar(removedBuildings[i].r_Radius))
			{
				return 1;
			}
		}
	}
	return 0;
}

void RemovedBuildingManager::restoreBuilding(removeBuildingData targetData)
{
	for (uint16_t i = 0; i < RemovedGameObjects.size(); i++)
	{
		if (RemovedGameObjects[i] != nullptr)
		{
			if ((targetData.r_Model == RemovedGameObjects[i]->Modelid || targetData.r_Model == -1))
			{
				btScalar dist = btDistance(btVector3(btScalar(targetData.r_X), btScalar(targetData.r_Y), btScalar(targetData.r_Z)),
					btVector3(btScalar(RemovedGameObjects[i]->Position.x), btScalar(RemovedGameObjects[i]->Position.y), btScalar(RemovedGameObjects[i]->Position.z)));

				if (dist <= btScalar(targetData.r_Radius))
				{
					uint16_t index = GetModelRef(RemovedGameObjects[i]->Modelid);
					if (index == 65535) continue;

					collisionWorld->createColAndreasMapObject(0, RemovedGameObjects[i]->Modelid,
						btQuaternion(RemovedGameObjects[i]->Rotation.x, RemovedGameObjects[i]->Rotation.y, RemovedGameObjects[i]->Rotation.z, RemovedGameObjects[i]->Rotation.w),
						btVector3(RemovedGameObjects[i]->Position.x, RemovedGameObjects[i]->Position.y, RemovedGameObjects[i]->Position.z));

					RemovedGameObjects[i] = nullptr;
				}
			}
		}
	}
}

void RemovedBuildingManager::addBuilding(removeBuildingData removeData)
{
	removedBuildings.push_back(removeData);
}

void InitCollisionMap(btDynamicsWorld* collisionWorld, RemovedBuildingManager* removedManager)
{
	for (uint16_t i = 0; i < IPLCount; i++)
	{
		if (ModelPlacements[i].Modelid > 19999)
		{
			logprintf("ERROR::InitCollisionMap::Invalid ModelID::%i", ModelPlacements[i].Modelid);
			continue;
		}

		uint16_t index = GetModelRef(ModelPlacements[i].Modelid);

		if (i % 100 == 0)
		{
			printf("\33Loading: %0.1f\r", ((double)i / IPLCount) * 100);
		}

		if (!removedManager->isRemoved(ModelPlacements[i].Modelid, ModelPlacements[i].Position))
		{
			// Continue if model has no collision
			if (index == 65535) continue;

			CreateStaticCollisionObject(ModelPlacements[i].Modelid, btQuaternion(ModelPlacements[i].Rotation.x, ModelPlacements[i].Rotation.y, ModelPlacements[i].Rotation.z, ModelPlacements[i].Rotation.w), btVector3(ModelPlacements[i].Position.x, ModelPlacements[i].Position.y, ModelPlacements[i].Position.z), collisionWorld);

		}
		else {
			RemovedGameObjects.push_back(new ItemPlacementstructure(ModelPlacements[i]));
		}
	}
	FreeModelPlacements();
}

uint16_t GetModelRef(int32_t model)
{
	if ((model >= 0 && model <= 20000) || (model <= -1000 && model > -30000))
	{
		return LookupModelRef(model);
	}
	return 65535;
}
