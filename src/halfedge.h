#pragma once

#include <GL/glew.h>

#include "km_lib.h"
#include "km_math.h"

struct HalfEdge;

struct Vertex
{
    uint32 halfEdge;
    Vec3 pos;
    Vec3 normal;
};

struct Face
{
    uint32 halfEdge;
    Vec3 normal;
    float32 area;
};

struct HalfEdge
{
    uint32 next;
    uint32 twin;
    uint32 vertex;
    uint32 face;
};

struct HalfEdgeMesh
{
    DynamicArray<Vertex> vertices;
    DynamicArray<Face> faces;
    DynamicArray<HalfEdge> halfEdges;
};

struct HalfEdgeMeshGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint normalBuffer;
    GLuint indexBuffer;
    GLuint programID;
    int vertexCount;
};

HalfEdgeMesh HalfEdgeMeshFromObj(const char* fileName);
HalfEdgeMesh CopyHalfEdgeMesh(const HalfEdgeMesh* mesh);
void FreeHalfEdgeMesh(HalfEdgeMesh* mesh);

void PrintHalfEdgeMesh(const HalfEdgeMesh& mesh);

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh);
void FreeHalfEdgeMeshGL(HalfEdgeMeshGL* meshGL);

void DrawHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL, Mat4 proj, Mat4 view);

// Utility functions
void SplitFaceMakeEdge(HalfEdgeMesh* mesh, uint32 f, uint32 v1, uint32 v2);
void TriangulateMesh(HalfEdgeMesh* mesh);
void ComputeFaceNormals(HalfEdgeMesh* mesh);
void ComputeFaceAreas(HalfEdgeMesh* mesh);

// Mesh traversal functions
void VerticesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<Vertex>& out);
void EdgesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<HalfEdge>& out);
void FacesOnFace(const HalfEdgeMesh& mesh, uint32 f,
    DynamicArray<Face>& out);

void VerticesOnEdge(const HalfEdgeMesh& mesh, uint32 e,
    DynamicArray<Vertex>& out);
void FacesOnEdge(const HalfEdgeMesh& mesh, uint32 e,
    DynamicArray<Face>& out);

void VerticesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<Vertex>& out);
void EdgesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<Vertex>& out);
void FacesOnVertex(const HalfEdgeMesh& mesh, uint32 v,
    DynamicArray<Vertex>& out);