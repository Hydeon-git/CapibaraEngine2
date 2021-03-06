#pragma once
#include "Component.h"
#include "Globals.h"
#include <string.h>
#include "Math/float3.h"
#include "Math/float2.h"
#include "Geometry/Frustum.h"
#include "Geometry/OBB.h"
#include "Geometry/AABB.h"
#include "par_shapes.h"

class ComponentMesh : public Component 
{
public:	
	enum class Shape
	{
		CUBE,
		SPHERE,
		CYLINDER,
		PLANE
	};

	ComponentMesh(GameObject* parent);
	ComponentMesh(GameObject* parent, Shape shape);
	~ComponentMesh();

	void CopyParMesh(par_shapes_mesh* parMesh);

	void GenerateBuffers();
	void ComputeNormals();
	void GenerateBounds();
	void DrawNormals() const;
	float3 GetCenterPointInWorldCoords() const;
	inline float GetSphereRadius() const { return radius; }
	inline AABB GetAABB() { return localAABB; }

	void DrawBoundingBox(float3* points, float3 color) const;
	bool GameCamera(Frustum* cam);
	bool Update(float dt) override;
	void OnGui() override;

	// Scene Serialization
	void Save(JSONWriter& writer) override;
	void Load(const JSONReader& reader) override;

	uint vertexBufferId = 0, indexBufferId = 0, textureBufferId = 0;
	std::string texturePath;
	
	uint numVertices = 0;
	std::vector<float3> vertices;

	uint numNormalFaces = 0;
	std::vector<float3> normals;
	std::vector<float3> faceNormals;
	std::vector<float3> faceCenters;

	std::vector<float2> texCoords;

	uint numIndices = 0;
	std::vector<uint> indices;

	bool drawWireframe = false;
	bool drawVertexNormals = false;
	bool drawFaceNormals = false;
	float normalScale = 1.f;
	
private:

	//Bounding sphere
	float3 centerPoint = float3::zero;
	float radius;

	//Local coords AABB
	AABB localAABB;

	bool drawAABB = true;
	bool drawOBB = false;
};