#include "Application.h"
#include "Globals.h"
#include "ModuleTextures.h"
#include "ImGui/imgui.h"
#include "ComponentMaterial.h"

ComponentMaterial::ComponentMaterial(GameObject* parent) : Component(parent) {}

void ComponentMaterial::SetTexture(const TextureObject& texture)
{
	textureName = texture.name;
	textureId = texture.id;
	width = texture.width;
	height = texture.height;
}

void ComponentMaterial::OnGui()
{
	if (ImGui::CollapsingHeader("Material"))
	{
		if (textureId != 0)
		{
			ImGui::Text("Name: %s", textureName.c_str());
			ImGui::Image((ImTextureID)textureId, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::Text("Size: %d x %d", width, height);
		}
	}
}

void ComponentMaterial::Save(JSONWriter& writer)
{
	// Object material
	writer.StartObject();
	writer.String("material");
	writer.StartArray();

	// Pos 0 name
	writer.String("name");
	writer.String(textureName.c_str());
	// Pos 1 width
	writer.String("width");
	writer.Uint(width);
	// Pos 2 height
	writer.String("height");
	writer.Uint(height);	
	// Pos 3 textureId
	writer.String("textureId");
	writer.Uint(textureId);

	// Closing first the array, then the object
	writer.EndArray();
	writer.EndObject();
}
void ComponentMaterial::Load(const JSONReader& reader)
{
	if (reader.HasMember("name"))
	{
		textureName = reader["name"].GetString();
	}
	if (reader.HasMember("width"))
	{
		width = reader["width"].GetUint();
	}
	if (reader.HasMember("height"))
	{
		height = reader["height"].GetUint();
	}
	if (reader.HasMember("textureId"))
	{
		textureId = reader["textureId"].GetUint();
	}
}