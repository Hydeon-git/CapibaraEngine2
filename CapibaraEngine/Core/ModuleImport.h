#pragma once
#include "Module.h"
#include <string>

struct aiNode;
class aiScene;
class aiMaterial;
class aiMesh;

struct TextureObject;
class GameObject;
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
};

namespace MeshImporter
{
	// Import FBX + Reorder it's child
	GameObject* FbxImport(const char* filePath);
	GameObject* ChildReorder(const aiScene* aScene, aiNode* aNode,
		aiNode* parentNode, GameObject* parentGO, const char* filePath);

	// DDS Process
	void Import(const aiMesh* assimpMesh, ComponentMesh* ourMesh);
	uint64 Save(const ComponentMesh* ourMesh, char** fileBuffer);
	void Load(const char* fileBuffer, ComponentMesh* ourMesh);
	
}
namespace MaterialImporter
{
	// DDS Process
	void Import(const aiMaterial* material, ComponentMaterial* ourMaterial, const char* path);
	uint64 Save(const ComponentMaterial* ourMaterial, char** fileBuffer);
	void Load(const char* fileBuffer, ComponentMaterial* ourMaterial);
	TextureObject* LoadMaterialTexture(const char* path);
}
