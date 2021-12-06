#pragma once

#include "Component.h"
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Geometry/Frustum.h"
#include "Globals.h"
#include "ModuleViewportFrameBuffer.h"
#include "ImGui/imgui.h"

#define FRUSTUM_POINTS { 0,2, 2,6, 6,4, 4,0, 0,1, 1,3, 3,2, 3,7, 7,5, 5,1, 6,7, 5,4 }
#define FRUSTUM_MAX_POINTS 24

class ComponentCamera : public Component
{
public:
	ComponentCamera(GameObject* parent);
	~ComponentCamera();

	void LookAt(const float3& point);
	void CalculateViewMatrix();
	void RecalculateProjection();
	void OnGui() override;
	void DrawCamera();
	void DrawCameraBoundaries();

	// Scene Serialization
	void Save(JSONWriter& writer) override;
	void Load(const JSONReader& reader) override;


	float3 right, up, front, position, reference;
	Frustum cameraFrustum;
	float4x4 viewMatrix;
	float aspectRatio = 1.f;
	float verticalFOV = 100.f;
	float nearPlaneDistance = 0.1f;
	float farPlaneDistance = 250.f;
	bool projectionIsDirty = false;
};