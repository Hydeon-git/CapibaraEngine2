#include "ComponentCamera.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleCamera3D.h"
#include "ComponentTransform.h"
#include "SDL/include/SDL_opengl.h"


ComponentCamera::ComponentCamera(GameObject* parent) : Component(parent)
{
	right = float3(1.0f, 0.0f, 0.0f);
	up = float3(0.0f, 1.0f, 0.0f);
	front = float3(0.0f, 0.0f, 1.0f);

	position = float3(0.0f, 5.0f, -15.0f);
	reference = float3(0.0f, 0.0f, 0.0f);

	owner->transform->SetPosition(position);

	CalculateViewMatrix();

	LookAt(float3::zero);
}

ComponentCamera::~ComponentCamera()
{}

void ComponentCamera::LookAt(const float3& point)
{
	reference = point;

	front = (reference - position).Normalized();
	right = float3(0.0f, 1.0f, 0.0f).Cross(front).Normalized();
	up = front.Cross(right);

	CalculateViewMatrix();
}

void ComponentCamera::CalculateViewMatrix()
{
	if (projectionIsDirty)
		RecalculateProjection();

	cameraFrustum.pos = position;
	cameraFrustum.front = front.Normalized();
	cameraFrustum.up = up.Normalized();
	float3::Orthonormalize(cameraFrustum.front, cameraFrustum.up);
	right = up.Cross(front);
	viewMatrix = cameraFrustum.ViewMatrix();
}

void ComponentCamera::RecalculateProjection()
{
	cameraFrustum.type = FrustumType::PerspectiveFrustum;
	cameraFrustum.nearPlaneDistance = nearPlaneDistance;
	cameraFrustum.farPlaneDistance = farPlaneDistance;
	cameraFrustum.verticalFov = (verticalFOV * 3.141592 / 2) / 180.f;
	cameraFrustum.horizontalFov = 2.f * atanf(tanf(cameraFrustum.verticalFov * 0.5f) * aspectRatio);
}

void ComponentCamera::OnGui()
{
	if (ImGui::CollapsingHeader("Editor Camera"))
	{
		if (ImGui::DragFloat("Vertical FOV", &verticalFOV))
		{
			RecalculateProjection();
		}
		if (ImGui::DragFloat("Near Plane Distance", &nearPlaneDistance))
		{
			RecalculateProjection();
		}
		if (ImGui::DragFloat("Far Plane Distance", &farPlaneDistance))
		{
			RecalculateProjection();
		}
	}
}

void ComponentCamera::DrawCamera()
{
	position = owner->transform->GetPosition();

	CalculateViewMatrix();

	App->viewportBufferGame->PreUpdate(App->dt);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(cameraFrustum.ProjectionMatrix().Transposed().ptr());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(viewMatrix.Transposed().ptr());
}