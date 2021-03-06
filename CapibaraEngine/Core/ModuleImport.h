#pragma once
#include "Module.h"
#include <string>

struct aiNode;
class aiScene;
class aiMaterial;
class aiMesh;

class ComponentMesh;
class ComponentMaterial;
class ComponentTransform;


class ModuleImport : public Module
{
public:
	ModuleImport(Application* app, bool start_enabled = true);

	bool Init() override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	bool LoadGeometry(const char* path);

	bool Save(std::string path);
	void Load(std::string path);

	void FindNodeName(const aiScene* scene, const size_t i, std::string& name);



	std::vector<std::string> check;
};

namespace MeshImporter
{
	//void Import(const aiMesh* assimpMesh, ComponentMesh* ourMesh);
	//uint64 Save(const ComponentMesh* ourMesh, char** fileBuffer);
	//void Load(const char* fileBuffer, ComponentMesh* ourMesh);
	
	
	/*GameObject* ImportFBX(const char* path);
	GameObject* PreorderChildren(const aiScene* scene, aiNode* node, aiNode* parentNode, GameObject* parentGO, const char* path);
	void LoadTransform(aiNode* node, ComponentTransform* transform);*/
}
