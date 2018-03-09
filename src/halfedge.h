#pragma once

#include <GL/glew.h>

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
    uint32 numVertices;
    Vertex* vertices;
    uint32 numFaces;
    Face* faces;
    uint32 numHalfEdges;
    HalfEdge* halfEdges;
};

struct HalfEdgeMeshGL
{
    GLuint vertexArray;
    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLuint programID;
};

HalfEdgeMesh HalfEdgeMeshFromObj(const char* filePath);
HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh);
void FreeHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL);