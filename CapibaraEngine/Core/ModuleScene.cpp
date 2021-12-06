#include "Globals.h"
#include "Application.h"
#include "ModuleScene.h"
#include "glew.h"
#include "ImGui/imgui.h"
#include "ModuleImport.h"
#include "ModuleTextures.h"
#include "ModuleCamera3D.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "Algorithm/Random/LCG.h"
#include <stack>
#include <queue>

ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

bool ModuleScene::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	
	LCG num;
	int UUID = num.Int();

	root = new GameObject("Root", UUID);

	//Loading house and textures since beginning
	App->import->LoadGeometry("Assets/Models/street2.fbx");
	//App->import->Load("Library/");
	return ret;
}

bool ModuleScene::CleanUp()
{
	std::stack<GameObject*> S;
	for (GameObject* child : root->children)	
	{
		S.push(child);
	}
	root->children.clear();

	while (!S.empty())
	{
		GameObject* go = S.top();
		S.pop();
		for (GameObject* child : go->children)
		{
			S.push(child);
		}
		go->children.clear();
		delete go;
	}

	delete root;

	return true;
}

update_status ModuleScene::Update(float dt)
{
	std::queue<GameObject*> S;
	for (GameObject* child : root->children)
	{
		S.push(child);
	}

	while (!S.empty())
	{
		GameObject* go = S.front();
		go->Update(dt);
		S.pop();
		for (GameObject* child : go->children)
		{
			S.push(child);
		}
	}

	glDisable(GL_DEPTH_TEST);

	if (App->editor->gameobjectSelected)
	{
		ComponentTransform* transform = App->editor->gameobjectSelected->GetComponent<ComponentTransform>();
		float3 pos = transform->GetPosition();
		glLineWidth(10.f);
		glBegin(GL_LINES);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3f(pos.x + transform->Right().x, pos.y + transform->Right().y, pos.z + transform->Right().z);
		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3f(pos.x + transform->Front().x, pos.y + transform->Front().y, pos.z + transform->Front().z);
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3f(pos.x + transform->Up().x, pos.y + transform->Up().y, pos.z + transform->Up().z);
		glEnd();
		glLineWidth(1.f);
	}

	glEnable(GL_DEPTH_TEST);

	App->editor->DrawGrid();
	App->viewportBuffer->PostUpdate(dt);

	if (App->editor->cameraGame != nullptr)
	{
		App->editor->cameraGame->DrawCamera();
		std::queue<GameObject*> S;
		for (GameObject* child : root->children)
		{
			S.push(child);
		}

		while (!S.empty())
		{
			GameObject* go = S.front();
			go->Update(dt);
			S.pop();
			for (GameObject* child : go->children)
			{
				S.push(child);
			}
		}
		App->viewportBufferGame->PostUpdate(dt);
	}

	

	return UPDATE_CONTINUE;
}

GameObject* ModuleScene::CreateGameObject(GameObject* parent) {

	GameObject* temp = new GameObject();

	if (countGO > 0)
	{
		temp->name += std::to_string(countGO);
	}
	++countGO;

	if (parent)
		parent->AttachChild(temp);
	else
		root->AttachChild(temp);
	return temp;
}
GameObject* ModuleScene::CreateGameObject(const std::string name, GameObject* parent)
{
	GameObject* temp = new GameObject(name, root->UUID);
	if (parent)
		parent->AttachChild(temp);
	else
		root->AttachChild(temp);
	return temp;
}

void ModuleScene::CreateRoot()
{
	rootList.clear();

	root = new GameObject("Root", root->UUID);
	gameObjectList.push_back(root);
	rootList.push_back(root);

	for (GameObject* child : root->children)
	{
		gameObjectList.push_back(child);
	}
}
bool ModuleScene::CleanUpAllGameObjects()
{
	std::stack<GameObject*> S;
	for (GameObject* child : root->children)
	{
		S.push(child);
	}
	root->children.clear();

	while (!S.empty())
	{
		GameObject* go = S.top();
		S.pop();
		for (GameObject* child : go->children)
		{
			S.push(child);
		}
		go->children.clear();
		delete go;
	}

	return true;
}
bool ModuleScene::CleanUpSelectedGameObject(GameObject* selectedGameObject)
{
	bool ret = true;

	if (selectedGameObject)
	{
		if (selectedGameObject != root)
		{
			for (int i = 0; i < gameObjectList.size(); i++)
			{
				if (gameObjectList[i] == selectedGameObject && selectedGameObject->parent->name == "Root")
				{
					root->RemoveChild(selectedGameObject);
					gameObjectList.erase(gameObjectList.begin() + i);
				}
				else
				{
					selectedGameObject->parent->RemoveChild(selectedGameObject);
					gameObjectList.erase(gameObjectList.begin() + i);
				}
			}
		}
		else
		{
			root->~GameObject();
		}
	}

	return ret;
}

void ModuleScene::Save()
{
	rapidjson::StringBuffer sceneBuffer;
	JSONWriter writer(sceneBuffer);

	writer.StartObject();
	writer.String("GameObjects");
	writer.StartArray();
	root->Save(writer);
	writer.EndArray();
	writer.EndObject();

	if (App->fileSystem->Save("Library/Scenes/scene.capi", sceneBuffer.GetString(), strlen(sceneBuffer.GetString()), false))
	{
		LOG("Capibara Scene saved on Library/Scenes succesfully!!");
	}
	else LOG("Capibara Scene save FAILED!");	
}

void ModuleScene::Load(const char* destinationPath)
{
	char* loadBuffer = nullptr;

	if (App->fileSystem->Load(destinationPath, &loadBuffer))
	{
		rapidjson::Document document;
		if (document.Parse<rapidjson::kParseStopWhenDoneFlag>(loadBuffer).HasParseError() == false)
		{
			const rapidjson::Value reader = document.GetObjectJSON();

			if (reader.HasMember("GameObjects"))
			{
				auto& a = reader["GameObjects"];
				if (a.IsArray())
				{
					for (int i = 0; i < a.MemberCount(); ++i)
					{
						GameObject newGO;
						newGO.Load(reader);
						root->AttachChild(&newGO);
					}
				}
			}
			LOG("CapibaraEngine Scene Imported Succesfully!!");
		}
	}
	RELEASE_ARRAY(loadBuffer);
}