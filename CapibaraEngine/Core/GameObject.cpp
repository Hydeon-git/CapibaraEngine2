#include "GameObject.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ImGui/imgui.h"
#include "Algorithm/Random/LCG.h"

GameObject::GameObject() {

	name = name + ("GameObject");
	parent = nullptr;

	LCG num;
	UUID = num.Int();

	transform = CreateComponent<ComponentTransform>();

	active = true;
}

GameObject::GameObject(const std::string name, const int UUID) : name(name), UUID(UUID)
{
	transform = CreateComponent<ComponentTransform>();

	active = true;
}


GameObject::~GameObject() {

	for (size_t i = 0; i < components.size(); i++) {
		RELEASE(components[i]);
	}

	components.clear();

	for (GameObject* go : children)
	{
		RELEASE(go);
	}

	parent = nullptr;
}

void GameObject::Update(float dt) 
{
	for (Component* component : components)
	{
		component->Update(dt);
	}
}

void GameObject::OnGui()
{
	if (App->scene->root != this)
	{
		ImGui::Text("%s", name.c_str());
		ImGui::Separator();

		for (Component* component : components)
		{
			component->OnGui();
		}
	}
}

void GameObject::DeleteComponent(Component* component) {

	auto componentIt = std::find(components.begin(), components.end(), component);
	if (componentIt != components.end())
	{
		components.erase(componentIt);
		components.shrink_to_fit();
	}
}

void GameObject::AddComponent(Component* component)
{
	components.push_back(component);
}

void GameObject::AttachChild(GameObject* child)
{
	child->parent = this;
	children.push_back(child);
	child->transform->NewAttachment();
	child->PropagateTransform();
	App->scene->gameObjectList.push_back(child);
}

void GameObject::RemoveChild(GameObject* child)
{
	auto it = std::find(children.begin(), children.end(), child);
	if (it != children.end())
	{
		children.erase(it);
	}
}

void GameObject::PropagateTransform()
{
	for (GameObject* go : children)
	{
		go->transform->OnParentMoved();
	}
}

void GameObject::Save(JSONWriter& writer)
{
	// Object material
	writer.StartObject();
	writer.String("material");
	writer.StartArray();

	// Pos 0 name
	writer.String("name");
	writer.String(name.c_str());	
	// Pos 1 components
	writer.String("components");
	for (size_t i = 0; i < components.size(); ++i)
	{
		components[i]->Save(writer);
	}
	// Pos 2 goUUID
	writer.String("goUUID");
	writer.Uint(UUID);
	// Pos 3 parentUUID
	writer.String("parentUUID");
	if (parent != nullptr) writer.Uint(parent->UUID);
	else writer.Uint(0);

	// Closing first the array, then the object
	writer.EndArray();
	writer.EndObject();

	for (size_t i = 0; i < children.size(); ++i)
	{
		children[i]->Save(writer);
	}
}
void GameObject::Load(const JSONReader& reader)
{
	if (reader.HasMember("name"))
	{
		name = reader["name"].GetString();
	}	
	if (reader.HasMember("components"))
	{
		auto& rapidAuto = reader["components"];
		for (int i = 0; i < rapidAuto.MemberCount(); ++i)
		{
			Component* newComponent = nullptr;

			if (rapidAuto[i].HasMember("material"))
			{
				newComponent = new ComponentMaterial(0);
			}
			if (rapidAuto[i].HasMember("mesh"))
			{
				newComponent = new ComponentMesh(0);
			}
			if (rapidAuto[i].HasMember("camera"))
			{
				newComponent = new ComponentCamera(0);
			}
			if (rapidAuto[i].HasMember("transform"))
			{
				newComponent = new ComponentTransform(0);
			}
			if (rapidAuto[i] != nullptr)
			{
				newComponent->Load(reader);
				components.push_back(newComponent);
			}
		}
	}
	if (reader.HasMember("goUUID"))
	{
		UUID = reader["goUUID"].GetUint();
	}
	if (reader.HasMember("parentUUID"))
	{
		parent->UUID = reader["parentUUID"].GetUint();
	}	
}