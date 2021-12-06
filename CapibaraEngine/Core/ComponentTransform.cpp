#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleScene.h"
#include "Math/TransformOps.h"
#include "glew.h"
#include "ImGui/imgui.h"

ComponentTransform::ComponentTransform(GameObject* parent) : Component(parent) {
	
	position = float3::zero;
	rotation = Quat::identity;
	scale = float3::one;

	transformMatrix.SetIdentity();
	transformMatrixLocal.SetIdentity();
}


bool ComponentTransform::Update(float dt) {
	if (isDirty)
	{
		transformMatrixLocal = float4x4::FromTRS(position, rotation, scale);
		right = transformMatrixLocal.Col3(0).Normalized();
		up = transformMatrixLocal.Col3(1).Normalized();
		front = transformMatrixLocal.Col3(2).Normalized();
		RecomputeGlobalMatrix();
		owner->PropagateTransform();
		isDirty = false;
	}
	return true;
}

void ComponentTransform::OnGui()
{
	if (ImGui::CollapsingHeader("Transform"))
	{
		float3 newPosition = position;
		if (ImGui::DragFloat3("Location", &newPosition[0]))
		{
			SetPosition(newPosition);
		}
		float3 newRotationEuler;
		newRotationEuler.x = RADTODEG * rotationEuler.x;
		newRotationEuler.y = RADTODEG * rotationEuler.y;
		newRotationEuler.z = RADTODEG * rotationEuler.z;
		if (ImGui::DragFloat3("Rotation", &(newRotationEuler[0])))
		{
			newRotationEuler.x = DEGTORAD * newRotationEuler.x;
			newRotationEuler.y = DEGTORAD * newRotationEuler.y;
			newRotationEuler.z = DEGTORAD * newRotationEuler.z;
			SetRotation(newRotationEuler);
		}
		float3 newScale = scale;
		if (ImGui::DragFloat3("Scale", &(newScale[0])))
		{
			SetScale(newScale);
		}
	}
}

void ComponentTransform::SetPosition(const float3& newPosition)
{
	position = newPosition;
	isDirty = true;
}

void ComponentTransform::SetRotation(const float3& newRotation)
{
	Quat rotationDelta = Quat::FromEulerXYZ(newRotation.x - rotationEuler.x, newRotation.y - rotationEuler.y, newRotation.z - rotationEuler.z);
	rotation = rotation * rotationDelta;
	rotationEuler = newRotation;
	isDirty = true;
}

void ComponentTransform::SetScale(const float3& newScale)
{
	scale = newScale;
	isDirty = true;
}

void ComponentTransform::NewAttachment()
{
	if (owner->parent != App->scene->root)
		transformMatrixLocal = owner->parent->transform->transformMatrix.Inverted().Mul(transformMatrix);

	float3x3 rot;
	transformMatrixLocal.Decompose(position, rot, scale);
	rotationEuler = rot.ToEulerXYZ();
}

void ComponentTransform::OnParentMoved()
{
	RecomputeGlobalMatrix();
}

void ComponentTransform::RecomputeGlobalMatrix()
{
	if (owner->parent != nullptr)
	{
		transformMatrix = owner->parent->transform->transformMatrix.Mul(transformMatrixLocal);
	}
	else
	{
		transformMatrix = transformMatrixLocal;
	}
	if (owner->GetComponent<ComponentMesh>() != nullptr)
		owner->GetComponent<ComponentMesh>()->GenerateBounds();
}

void ComponentTransform::Save(JSONWriter& writer)
{
	// Object material
	writer.StartObject();
	writer.String("transform");
	writer.StartArray();

	// Pos 1 obj position
	writer.StartObject();
	writer.String("position");
	writer.StartArray();
	writer.Double(position.x);
	writer.Double(position.y);
	writer.Double(position.z);
	writer.EndArray();
	writer.EndObject();
	// Pos 2 obj rotation
	writer.StartObject();
	writer.String("rotation");
	writer.StartArray();
	writer.Double(rotation.x);
	writer.Double(rotation.y);
	writer.Double(rotation.z);
	writer.EndArray();
	writer.EndObject();
	// Pos 3 obj scale
	writer.StartObject();
	writer.String("scale");
	writer.StartArray();
	writer.Double(scale.x);
	writer.Double(scale.y);
	writer.Double(scale.z);
	writer.EndArray();
	writer.EndObject();

	// Closing first the array, then the object
	writer.EndArray();
	writer.EndObject();
}
void ComponentTransform::Load(const JSONReader& reader)
{	
	if (reader.HasMember("position"))
	{
		auto& rapidAuto = reader["position"].GetArray();
		position.x = rapidAuto[0].GetFloat();
		position.y = rapidAuto[1].GetFloat();
		position.z = rapidAuto[2].GetFloat();
	}
	if (reader.HasMember("rotation"))
	{
		auto& rapidAuto = reader["rotation"].GetArray();
		rotation.x = rapidAuto[0].GetFloat();
		rotation.y = rapidAuto[1].GetFloat();
		rotation.z = rapidAuto[2].GetFloat();
	}
	if (reader.HasMember("scale"))
	{
		auto& rapidAuto = reader["scale"].GetArray();
		rotation.x = rapidAuto[0].GetFloat();
		rotation.y = rapidAuto[1].GetFloat();
		rotation.z = rapidAuto[2].GetFloat();
	}
}
