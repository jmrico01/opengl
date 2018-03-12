#pragma once

#include <GL/glew.h>

#include "km_lib.h"
#include "km_math.h"

struct HalfEdge;

struct Vertex {
    Vec3 pos;
    uint32 halfEdge;
};

struct Face {
    uint32 halfEdge;
};

struct HalfEdge {
    uint32 next;
    uint32 twin;
    uint32 vertex;
    uint32 face;
};

struct HalfEdgeMesh {
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
void SplitFaceMakeEdge(HalfEdgeMesh* mesh, uint32 f, uint32 v1, uint32 v2);
void TriangulateMesh(HalfEdgeMesh* mesh);
void FreeHalfEdgeMesh(HalfEdgeMesh* mesh);

void PrintHalfEdgeMesh(const HalfEdgeMesh& mesh);

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh);
void FreeHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL);

void DrawHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL, Mat4 proj, Mat4 view);