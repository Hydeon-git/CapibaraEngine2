#include "Globals.h"

#include "Application.h"
#include "ModuleImport.h"
#include "ModuleWindow.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"
#include "GameObject.h"

#include <vector>
#include <queue>
#include "SDL/include/SDL_opengl.h"
#include "Math/float2.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/mesh.h"

#pragma region ModuleImport
ModuleImport::ModuleImport(Application* app, bool start_enabled) : Module(app, start_enabled) {}


// Called before render is available
bool ModuleImport::Init()
{
	//LOG("Creating 3D Renderer context");
	bool ret = true;

	//Stream log messages to Debug window
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);

	return ret;
}

update_status ModuleImport::Update(float dt) {

	return UPDATE_CONTINUE;
}

bool ModuleImport::LoadGeometry(const char* path) {

	//-- Own structure	
	GameObject* root = nullptr;
	std::string new_root_name(path);

	//-- Assimp stuff
	aiMesh* assimpMesh = nullptr;
	const aiScene* scene = nullptr;
	aiMaterial* texture = nullptr;
	aiString texturePath;

	//Create path buffer and import to scene
	char* buffer = nullptr;
	uint bytesFile = App->fileSystem->Load(path, &buffer);

	if (buffer == nullptr) {
		std::string normPathShort = "Assets/Models/" + App->fileSystem->SetNormalName(path);
		bytesFile = App->fileSystem->Load(normPathShort.c_str(), &buffer);
	}
	if (buffer != nullptr) {
		scene = aiImportFileFromMemory(buffer, bytesFile, aiProcessPreset_TargetRealtime_MaxQuality, NULL);
	}
	else {
		scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	}


	if (scene != nullptr && scene->HasMeshes()) {
		//Use scene->mNumMeshes to iterate on scene->mMeshes array
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{		
			bool nameFound = false;
			std::string name;
			FindNodeName(scene, i, name);

			GameObject* newGameObject = App->scene->CreateGameObject(name);
			ComponentMesh* mesh = newGameObject->CreateComponent<ComponentMesh>();
			assimpMesh = scene->mMeshes[i];
			
			if (scene->HasMaterials()) {
				texture = scene->mMaterials[assimpMesh->mMaterialIndex];

				if (texture != nullptr) {
					aiGetMaterialTexture(texture, aiTextureType_DIFFUSE, assimpMesh->mMaterialIndex, &texturePath);
					std::string new_path(texturePath.C_Str());
					if (new_path.size() > 0) {
						mesh->texturePath = "Assets/Textures/" + new_path;
						if (!App->textures->Find(mesh->texturePath))
						{
							const TextureObject& textureObject = App->textures->Load(mesh->texturePath);							
							ComponentMaterial* materialComp = newGameObject->CreateComponent<ComponentMaterial>();
							materialComp->SetTexture(textureObject);
							
						}
						else
						{
							const TextureObject& textureObject = App->textures->Get(mesh->texturePath);
							ComponentMaterial* materialComp = newGameObject->CreateComponent<ComponentMaterial>();
							materialComp->SetTexture(textureObject);
						}
					}
				}
			}
	
			mesh->numVertices = assimpMesh->mNumVertices;
			mesh->vertices.resize(assimpMesh->mNumVertices);
			
			memcpy(&mesh->vertices[0], assimpMesh->mVertices, sizeof(float3) * assimpMesh->mNumVertices);
			LOG("New mesh with %d vertices", assimpMesh->mNumVertices);

			// -- Copying faces --//
			if (assimpMesh->HasFaces()) {
				mesh->numIndices = assimpMesh->mNumFaces * 3;
				mesh->indices.resize(mesh->numIndices);

				for (size_t i = 0; i < assimpMesh->mNumFaces; i++)
				{
					if (assimpMesh->mFaces[i].mNumIndices != 3) {
						LOG("WARNING, geometry face with != 3 indices!")
					}
					else {
						memcpy(&mesh->indices[i * 3], assimpMesh->mFaces[i].mIndices, 3 * sizeof(uint));
					}
				}
			}
			
			// -- Copying Normals info --//
			if (assimpMesh->HasNormals()) {

				mesh->normals.resize(assimpMesh->mNumVertices);
				memcpy(&mesh->normals[0], assimpMesh->mNormals, sizeof(float3) * assimpMesh->mNumVertices);
			}
			
			// -- Copying UV info --//
			if (assimpMesh->HasTextureCoords(0))
			{
				mesh->texCoords.resize(assimpMesh->mNumVertices);
				for (size_t j = 0; j < assimpMesh->mNumVertices; ++j)
				{
					memcpy(&mesh->texCoords[j], &assimpMesh->mTextureCoords[0][j], sizeof(float2));
				}
			}
			
			mesh->GenerateBuffers();
			mesh->GenerateBounds();
			mesh->ComputeNormals();
		}
		aiReleaseImport(scene);		
		RELEASE_ARRAY(buffer);

	}
	else 
		LOG("Error loading scene %s", path);

	RELEASE_ARRAY(buffer);

	return true;
}
void ModuleImport::FindNodeName(const aiScene* scene, const size_t i, std::string& name)
{
	bool nameFound = false;
	std::queue<aiNode*> Q;
	Q.push(scene->mRootNode);
	while (!Q.empty() && !nameFound)
	{
		aiNode* node = Q.front();
		Q.pop();
		for (size_t j = 0; j < node->mNumMeshes; ++j)
		{
			if (node->mMeshes[j] == i)
			{
				nameFound = true;
				name = node->mName.C_Str();
			}
		}
		if (!nameFound)
		{
			for (size_t j = 0; j < node->mNumChildren; ++j)
			{
				Q.push(node->mChildren[j]);
			}
		}
	}
}

// Called before quitting
bool ModuleImport::CleanUp()
{
	//-- Detach log stream
	aiDetachAllLogStreams();

	return true;
}
#pragma endregion
#pragma region MeshImporter
void MeshImporter::Import(const aiMesh* assimpMesh, ComponentMesh* ourMesh)
{
	Timer imporTimer; imporTimer.Start();

	ourMesh->numVertices = assimpMesh->mNumVertices;
	ourMesh->vertices.resize(assimpMesh->mNumVertices);

	memcpy(&ourMesh->vertices[0], assimpMesh->mVertices, sizeof(float3) * assimpMesh->mNumVertices);
	LOG("New mesh with %d vertices", assimpMesh->mNumVertices);

	// -- Copying faces --//
	if (assimpMesh->HasFaces()) 
	{
		ourMesh->numIndices = assimpMesh->mNumFaces * 3;
		ourMesh->indices.resize(ourMesh->numIndices);

		for (size_t i = 0; i < assimpMesh->mNumFaces; i++)
		{
			if (assimpMesh->mFaces[i].mNumIndices != 3) 
			{
				LOG("WARNING, geometry face with != 3 indices!")
			}
			else 
			{
				memcpy(&ourMesh->indices[i * 3], assimpMesh->mFaces[i].mIndices, 3 * sizeof(uint));
			}
		}
	}
	// -- Copying Normals info --//
	if (assimpMesh->HasNormals()) 
	{
		ourMesh->normals.resize(assimpMesh->mNumVertices);
		memcpy(&ourMesh->normals[0], assimpMesh->mNormals, sizeof(float3) * assimpMesh->mNumVertices);
	}
	// -- Copying UV info --//
	if (assimpMesh->HasTextureCoords(0))
	{
		ourMesh->texCoords.resize(assimpMesh->mNumVertices);
		for (size_t j = 0; j < assimpMesh->mNumVertices; ++j)
		{
			memcpy(&ourMesh->texCoords[j], &assimpMesh->mTextureCoords[0][j], sizeof(float2));
		}
	}
	ourMesh->GenerateBuffers();
	LOG("The mesh was imported from Assimp in %f seconds", imporTimer.ReadSec());
}
uint64 MeshImporter::Save(const ComponentMesh* ourMesh, char** fileBuffer)
{
	uint ranges[3] = {ourMesh->numIndices, ourMesh->numVertices, ourMesh->numNormalFaces};
	uint size = sizeof(ranges) + sizeof(uint) * ourMesh->numIndices 
		+ sizeof(float3) * ourMesh->numVertices * 3
		+ sizeof(float3) * ourMesh->numNormalFaces * 3;

	// Allocate Buffer
	char* fileBuffer = new char[size]; 
	char* cursor = *fileBuffer;

	// Store ranges
	uint bytes = sizeof(ranges); 
	memcpy(cursor, ranges, bytes);
	cursor += bytes;
	// Store Indices
	bytes = sizeof(uint) * ourMesh->numIndices;
	memcpy(cursor, &ourMesh->indices, bytes);
	cursor += bytes;	
	// Store Vertex
	bytes = sizeof(float3) * ourMesh->numVertices;
	memcpy(cursor, &ourMesh->vertices, bytes);
	cursor += bytes;	
	// Store Normals
	bytes = sizeof(float3) * ourMesh->numNormalFaces;
	memcpy(cursor, &ourMesh->normals, bytes);
	cursor += bytes;

	return size;
}
void MeshImporter::Load(const char* fileBuffer, ComponentMesh* ourMesh)
{
	Timer imporTimer; imporTimer.Start();
	const char* cursor = fileBuffer;

	// Amount of Indices / Vertices / Normals
	uint ranges[3];
	uint bytes = sizeof(ranges);
	memcpy(ranges, cursor, bytes);
	cursor += bytes;

	ourMesh->numIndices = ranges[0];
	ourMesh->numVertices = ranges[1];
	ourMesh->numNormalFaces = ranges[2];

	// Load indices
	bytes = sizeof(uint) * ourMesh->numIndices;
	ourMesh->indices = new uint[ourMesh->numIndices];
	memcpy(&ourMesh->indices, cursor, bytes);
	cursor += bytes;		
	// Load Vertices
	bytes = sizeof(float3) * ourMesh->numVertices;
	ourMesh->vertices = new float3[ourMesh->numVertices];
	memcpy(&ourMesh->vertices, cursor, bytes);
	cursor += bytes;
	// Load Normals
	bytes = sizeof(float3) * ourMesh->numNormalFaces;
	ourMesh->normals = new float3[ourMesh->numNormalFaces];
	memcpy(&ourMesh->normals, cursor, bytes);
	cursor += bytes;

	LOG("Mesh was loaded succesfully in %f seconds", )
}
#pragma endregion