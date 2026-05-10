#include "ColAndreasDatabaseReader.h"
#include "ColAndreas.h"

std::vector<CollisionModelstructure> CollisionModels;
ItemPlacementstructure* ModelPlacements;
std::vector<ItemPlacementstructure*> RemovedGameObjects;
uint16_t ModelCount = 0;
uint32_t IPLCount = 0;
uint16_t ModelRef[20001];
std::map<int32_t, uint16_t> CustomModelRef;

void ClearModelRefs()
{
	for (int i = 0; i <= 20000; i++)
	{
		ModelRef[i] = 65535;
	}
	CustomModelRef.clear();
}

void SetModelRef(int32_t model, uint16_t index)
{
	if (model >= 0 && model <= 20000)
	{
		ModelRef[model] = index;
	}
	else
	{
		CustomModelRef[model] = index;
	}
}

uint16_t LookupModelRef(int32_t model)
{
	if (model >= 0 && model <= 20000)
	{
		return ModelRef[model];
	}

	std::map<int32_t, uint16_t>::iterator it = CustomModelRef.find(model);
	if (it != CustomModelRef.end())
	{
		return it->second;
	}
	return 65535;
}

void FreeCollisionModelGeometry()
{
	for (std::vector<CollisionModelstructure>::iterator it = CollisionModels.begin(); it != CollisionModels.end(); ++it)
	{
		delete[] it->SphereData;
		delete[] it->BoxData;
		delete[] it->FacesData;
		it->SphereData = NULL;
		it->BoxData = NULL;
		it->FacesData = NULL;
	}
}

void FreeModelPlacements()
{
	delete[] ModelPlacements;
	ModelPlacements = NULL;
	IPLCount = 0;
}

void DeleteCollisionData()
{
	FreeCollisionModelGeometry();
	CollisionModels.clear();
	FreeModelPlacements();
	for (std::vector<ItemPlacementstructure*>::size_type i = 0; i < RemovedGameObjects.size(); i++)
	{
		delete RemovedGameObjects[i];
	}
	RemovedGameObjects.clear();
	ClearModelRefs();
	ModelCount = 0;
	IPLCount = 0;
}

bool ReadColandreasDatabaseFile(std::string FileLocation)
{
	bool returnValue = false;

	ifstream ColAndreasBinaryfile;

	ColAndreasBinaryfile.open(FileLocation, ios::in | ios::binary);

	if (ColAndreasBinaryfile.is_open()) {
		ColAndreasBinaryfile.seekg(0, ColAndreasBinaryfile.end);
		int length = static_cast<int>(ColAndreasBinaryfile.tellg());

		ColAndreasBinaryfile.seekg(0, ColAndreasBinaryfile.beg);

		char * buffer = new char[length];

		ColAndreasBinaryfile.read(buffer, length);

		uint32_t FileIndex = 0;

		char FileExtension[4];
		GetBytes(buffer, FileExtension, FileIndex, 4);
		
		//If is a ColAndreas binary file.
		if (!strncmp(FileExtension, "cadf", 4))
		{
			uint16_t fileVersion;			
			GetBytes(buffer, fileVersion, FileIndex, 2);
			
			if(fileVersion == CA_DATABASE_VERSION)
			{
				GetBytes(buffer, ModelCount, FileIndex, 2);
				GetBytes(buffer, IPLCount, FileIndex, 4);
				ClearModelRefs();
				CollisionModels.resize(ModelCount);

				if (ModelCount > 0) {

					for (uint16_t i = 0; i < ModelCount; i++) {
						GetBytes(buffer, CollisionModels[i].Modelid, FileIndex, 2);
						GetBytes(buffer, CollisionModels[i].SphereCount, FileIndex, 2);
						GetBytes(buffer, CollisionModels[i].BoxCount, FileIndex, 2);
						GetBytes(buffer, CollisionModels[i].FaceCount, FileIndex, 2);
						CollisionModels[i].SphereData = NULL;
						CollisionModels[i].BoxData = NULL;
						CollisionModels[i].FacesData = NULL;

						if (CollisionModels[i].SphereCount > 0) {
							CollisionModels[i].SphereData = new structSphereData[CollisionModels[i].SphereCount];

							for (uint16_t j = 0; j < CollisionModels[i].SphereCount; j++) {
								GetBytes(buffer, CollisionModels[i].SphereData[j], FileIndex, sizeof(structSphereData));
							}
						}

						if (CollisionModels[i].BoxCount > 0) {
							CollisionModels[i].BoxData = new structBoxData[CollisionModels[i].BoxCount];

							for (uint16_t j = 0; j < CollisionModels[i].BoxCount; j++) {
								GetBytes(buffer, CollisionModels[i].BoxData[j], FileIndex, sizeof(structBoxData));
							}
						}

						if (CollisionModels[i].FaceCount > 0)
						{
							CollisionModels[i].FacesData = new structFacesData[CollisionModels[i].FaceCount];

							for (uint16_t j = 0; j < CollisionModels[i].FaceCount; j++) {
								GetBytes(buffer, CollisionModels[i].FacesData[j], FileIndex, sizeof(structFacesData));
							}
						}
					}
				}
				
				if (IPLCount > 0) {
					ModelPlacements = new ItemPlacementstructure[IPLCount];

					for (uint32_t i = 0; i < IPLCount; i++) {
						GetBytes(buffer, ModelPlacements[i].Modelid, FileIndex, sizeof(uint16_t));
						GetBytes(buffer, ModelPlacements[i].Position, FileIndex, sizeof(Vector));
						GetBytes(buffer, ModelPlacements[i].Rotation, FileIndex, sizeof(Quaternion));
					}
				}

				// Initialize model reference
				for (int i = 0; i < ModelCount; i++)
				{
					SetModelRef(CollisionModels[i].Modelid, i);
				}

				returnValue = true;
			}
			else
			{
				logprintf("ERROR: Incompatible database file, expecting version 0x%04X, but found 0x%04X.", CA_DATABASE_VERSION, fileVersion);
				returnValue = false;
			}
		}
		
		delete [] buffer;
	}
	ColAndreasBinaryfile.close();
	return returnValue;
}
