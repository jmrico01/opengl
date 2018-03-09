#pragma once

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
    uint32 vertex;
    uint32 face;
};

struct HalfEdgeMesh {
    uint32 numVertices;
    Vertex* vertices;
    uint32 numFaces;
    Face* faces;
    uint32 numEdges;
    HalfEdge* edgesForward;
    HalfEdge* edgesBackward;
};

HalfEdgeMesh HalfEdgeMeshFromObj(const char* filePath);