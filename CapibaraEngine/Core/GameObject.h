#pragma once

#include <vector>
#include <string>
#include "Geometry/OBB.h"
#include "Geometry/AABB.h"

#include "rapidjson-1.1.0/include/rapidjson/prettywriter.h"
#include "rapidjson-1.1.0/include/rapidjson/document.h"

class Component;
class ComponentTransform;

typedef rapidjson::PrettyWriter<rapidjson::StringBuffer> JSONWriter;
typedef rapidjson::Value JSONReader;

class GameObject 
{
public:

	GameObject();
	GameObject(const std::string name, const int UUID);

	~GameObject();

	void Update(float dt);
	void OnGui();

	template<class T> T* CreateComponent()
	{
		T* newComponent = new T(this);
		return newComponent;
	}

	template<class T> T* GetComponent()
	{
		T* component = nullptr; 
		for (Component* c : components)
		{
			component = dynamic_cast<T*>(c);
			if (component)
				break;
		}
		return component;
	}

	void DeleteComponent(Component* component);
	void AddComponent(Component* component);
	void AttachChild(GameObject* child);
	void RemoveChild(GameObject* child);
	void PropagateTransform();

	// Scene Serialization
	void Save(JSONWriter& writer);
	void Load(const JSONReader& reader);

	std::string name;
	GameObject* parent = nullptr;
	ComponentTransform* transform = nullptr;
	std::vector<GameObject*> children;
	std::vector<Component*> components;
	
	bool active = true;
	bool isSelected = false;

	OBB globalOBB;
	AABB globalAABB;

	int UUID;
};

