#include "ComponentMesh.h"

#include "glew.h"
#include "SDL/include/SDL_opengl.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ImGui/imgui.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include "Geometry/Sphere.h"
#include "par_shapes.h"

ComponentMesh::ComponentMesh(GameObject* parent) : Component(parent) {}

ComponentMesh::ComponentMesh(GameObject* parent, Shape shape) : Component(parent)
{
	switch (shape)
	{
	case Shape::CUBE:
		CopyParMesh(par_shapes_create_cube());		
		break;
	case Shape::CYLINDER:
		CopyParMesh(par_shapes_create_cylinder(20, 20));
		break;
	case Shape::SPHERE:
		CopyParMesh(par_shapes_create_parametric_sphere(20, 20));
		break;
	case Shape::PLANE:
		CopyParMesh(par_shapes_create_plane(20, 20));
		break;
	}
}

ComponentMesh::~ComponentMesh()
{
	vertexBufferId ? glDeleteBuffers(1, &vertexBufferId) : 0;
	textureBufferId ? glDeleteBuffers(1, &textureBufferId) : 0;
	indexBufferId ? glDeleteBuffers(1, &indexBufferId) : 0;
}

void ComponentMesh::CopyParMesh(par_shapes_mesh* parMesh)
{
	numVertices = parMesh->npoints;
	numIndices = parMesh->ntriangles * 3;
	numNormalFaces = parMesh->ntriangles;
	vertices.resize(numVertices);
	normals.resize(numVertices);
	indices.resize(numIndices);
	par_shapes_compute_normals(parMesh);
	for (size_t i = 0; i < numVertices; ++i)
	{
		memcpy(&vertices[i], &parMesh->points[i * 3], sizeof(float) * 3);
		memcpy(&normals[i], &parMesh->normals[i * 3], sizeof(float) * 3);
	}
	for (size_t i = 0; i < indices.size(); ++i)
	{
		indices[i] = parMesh->triangles[i];
	}
	memcpy(&normals[0], parMesh->normals, numVertices);

	par_shapes_free_mesh(parMesh);

	GenerateBuffers();
	ComputeNormals();
	GenerateBounds();
}


void ComponentMesh::GenerateBuffers()
{
	
	//-- Generate Vertex
	vertexBufferId = 0;
	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * numVertices, &vertices[0], GL_STATIC_DRAW);

	//-- Generate Index
	indexBufferId = 0;
	glGenBuffers(1, &indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * numIndices, &indices[0], GL_STATIC_DRAW);

	//-- Generate Texture_Buffers
	if (texCoords.size() != 0)
	{
		textureBufferId = 0;
		glGenBuffers(1, &textureBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, textureBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float2) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
	}
	if (vertexBufferId == 0 || indexBufferId == 0)
		LOG("Error creating mesh on gameobject %s", owner->name.c_str());
}

void ComponentMesh::ComputeNormals()
{

	numNormalFaces = numIndices * 3;
	faceNormals.resize(numNormalFaces);
	faceCenters.resize(numNormalFaces);

	for (size_t i = 0; i < numIndices; i += 3)
	{
		const float3 p0 = vertices[indices[i + 1]] - vertices[indices[i]];
		const float3 p1 = vertices[indices[i + 2]] - vertices[indices[i]];

		const float3 faceNormal = float3(p0.x, p0.y, p0.z).Cross(float3(p1.x, p1.y, p1.z));
		faceNormals[i / 3] = faceNormal.Normalized();

		const float3 faceCenter = (vertices[indices[i]] + vertices[indices[i + 1]] + vertices[indices[i + 2]]) / 3.f;
		faceCenters[i / 3] = faceCenter;
	}
	int kk = 0;
}

void ComponentMesh::GenerateBounds()
{
	localAABB.SetNegativeInfinity();
	localAABB.Enclose(&vertices[0], vertices.size());
		
	Sphere sphere;	
	sphere.r = 0.f;
	sphere.pos = localAABB.CenterPoint();
	sphere.Enclose(localAABB);

	radius = sphere.r;
	centerPoint = sphere.pos;

	owner->globalOBB = GetAABB();
	owner->globalOBB.Transform(owner->transform->transformMatrixLocal);

	owner->globalAABB.SetNegativeInfinity();
	owner->globalAABB.Enclose(owner->globalOBB);
}

void ComponentMesh::DrawNormals() const
{
	if (drawFaceNormals)
	{
		for (size_t i = 0; i < faceNormals.size(); ++i)
		{
			glColor3f(0.f, 0.f, 1.f);
			glBegin(GL_LINES);			
			const float3 faceCenter = owner->transform->transformMatrix.TransformPos(faceCenters[i]);
			const float3 faceNormalPoint = faceCenter + faceNormals[i] * normalScale;
			glVertex3f(faceCenter.x, faceCenter.y, faceCenter.z);
			glVertex3f(faceNormalPoint.x, faceNormalPoint.y, faceNormalPoint.z);
			glEnd();
		}
	}
	if (drawVertexNormals)
	{
		for (size_t i = 0; i < normals.size(); ++i)
		{
			glColor3f(1.f, 0.f, 0.f);
			glBegin(GL_LINES);
			const float3 vertexPos = owner->transform->transformMatrix.TransformPos(vertices[i]);
			const float3 vertexNormalPoint = vertexPos + normals[i] * normalScale;
			glVertex3f(vertexPos.x, vertexPos.y, vertexPos.z);
			glVertex3f(vertexNormalPoint.x, vertexNormalPoint.y, vertexNormalPoint.z);
			glEnd();
		}
	}
}

float3 ComponentMesh::GetCenterPointInWorldCoords() const
{
	return owner->transform->transformMatrix.TransformPos(centerPoint);
}

void ComponentMesh::DrawBoundingBox(float3* points, float3 color) const
{
	glColor3fv(&color.x);
	glLineWidth(2.f);
	glBegin(GL_LINES);

	std::vector<int> ind =
	{
		0,2,2,6,6,4,4,0,
		0,1,1,3,3,2,4,5,
		6,7,5,7,3,7,1,5,
	};
	for (const auto& i : ind)
	{
		glVertex3fv(&points[i].x);
	}

	glEnd();
	glLineWidth(1.f);
	glColor3f(1.f, 1.f, 1.f);
}

bool ComponentMesh::GameCamera(Frustum* frustumCam)
{
	float3 obb[8];
	Plane frustum[6];
	int total = 0;

	frustumCam->GetPlanes(frustum);
	owner->globalAABB.GetCornerPoints(obb);

	for (int i = 0; i < 6; i++)
	{
		int count = 8;
		int points = 1;

		for (int b = 0; b < 8; b++)
		{
			if (frustum[i].IsOnPositiveSide(obb[b]))
			{
				points = 0;
				--count;
			}

			if (count <= 0) return false;

			total += points;
		}
	}

	if (total >= 6) return true;

	return true;
}

bool ComponentMesh::Update(float dt)
{
	if (GameCamera(&App->editor->cameraGame->cameraFrustum))
	{
		drawWireframe || App->renderer3D->wireframeMode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindTexture(GL_TEXTURE_2D, 0);

		//--Enable States--//
		glEnableClientState(GL_VERTEX_ARRAY);

		if (this->textureBufferId)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, this->textureBufferId);
			glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		}

		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferId);
		glVertexPointer(3, GL_FLOAT, 0, NULL);

		if (ComponentMaterial* material = owner->GetComponent<ComponentMaterial>())
		{
			drawWireframe || !App->renderer3D->useTexture || App->renderer3D->wireframeMode ? 0 : glBindTexture(GL_TEXTURE_2D, material->GetTextureId());
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferId);

		//-- Draw --//
		glPushMatrix();
		glMultMatrixf(owner->transform->transformMatrix.Transposed().ptr());
		glColor3f(1.0f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLES, this->numIndices, GL_UNSIGNED_INT, NULL);
		glPopMatrix();
		//-- UnBind Buffers--//
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		if (this->textureBufferId)
		{
			glBindBuffer(GL_TEXTURE_COORD_ARRAY, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		}
		glBindTexture(GL_TEXTURE_2D, 0);

		//--Disables States--//
		glDisableClientState(GL_VERTEX_ARRAY);

		App->renderer3D->wireframeMode ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (drawFaceNormals || drawVertexNormals)
			DrawNormals();

		if (drawAABB)
		{
			float3 points[8];
			owner->globalAABB.GetCornerPoints(points);
			DrawBoundingBox(points, float3(1.0f, 0.5f, 0.5f));
		}

		if (drawOBB)
		{
			float3 points[8];
			owner->globalOBB.GetCornerPoints(points);
			DrawBoundingBox(points, float3(0.5f, 0.5f, 1.0f));
		}
	}
	
	return true;
}

void ComponentMesh::OnGui()
{
	if (ImGui::CollapsingHeader("Mesh"))
	{
		ImGui::Text("Num vertices %d", numVertices);
		ImGui::Text("Num faces %d", numIndices / 3);
		ImGui::Checkbox("Wireframe", &drawWireframe);
		ImGui::DragFloat("Normal draw scale", &normalScale);
		ImGui::Checkbox("Draw face normals", &drawFaceNormals);
		ImGui::Checkbox("Draw vertex normals", &drawVertexNormals);
		ImGui::Checkbox("Draw AABB", &drawAABB);
		ImGui::Checkbox("Draw OBB", &drawOBB);
	}
}

void ComponentMesh::Save(JSONWriter& writer)
{
	// Object material
	writer.StartObject();
	writer.String("mesh");
	writer.StartArray();

	// Closing first the array, then the object
	writer.EndArray();
	writer.EndObject();
}
void ComponentMesh::Load(const JSONReader& reader)
{	

}