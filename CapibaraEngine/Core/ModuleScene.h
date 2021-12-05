#pragma once

// -- Tools
#include "Module.h"
#include "Globals.h"
#include "ModuleImport.h"

#include "GameObject.h"
class ModuleScene : public Module
{
public:
	ModuleScene(Application* app, bool start_enabled = true);

	bool Start() override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	GameObject* CreateGameObject(GameObject* parent = nullptr);	
	GameObject* CreateGameObject(const std::string name, GameObject* parent = nullptr);	
	
	bool CleanUpAllGameObjects();
	bool CleanUpSelectedGameObject(GameObject* selectedGameObject);

	void CreateRoot();
public:
	
	GameObject* root;
	std::vector<GameObject*> gameObjectList;
	std::vector<GameObject*> rootList;

	int countGO = 0;
};
