#include "halfedge.h"

#include <stdio.h>

#include "main.h"
#include "km_lib.h"
#include "km_math.h"

#define OBJ_LINE_MAX 512

Vec3 ParseVec3(char* str)
{
    Vec3 result = { 0.0f, 0.0f, 0.0f };
    char* el = str;
    int i = 0;
    for (char* c = str; *c != '\0'; c++) {
        if (*c == ' ' || *c == '\n') {
            char* endptr = el;
            *c = '\0';
            result.e[i++] = (float32)strtod(el, &endptr);
            // TODO check error
            el = c + 1;
        }
    }

    return result;
}

HalfEdgeMesh HalfEdgeMeshFromObj(const char* filePath)
{
    HalfEdgeMesh mesh = {
        0, nullptr,
        0, nullptr,
        0, nullptr
    };

    char fullPath[PATH_MAX_LENGTH];
    CatStrings(GetAppPath(), filePath, fullPath, PATH_MAX_LENGTH);
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Failed to open OBJ file at: %s\n", filePath);
        return mesh;
    }

    char line[OBJ_LINE_MAX];
    DynamicArray<Vertex> vertices;
    vertices.Init();
    DynamicArray<int> faceInds;
    faceInds.Init();

    while (fgets(line, OBJ_LINE_MAX, file)) {
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            if (line[1] == ' ') {
                Vec3 vertexPos = ParseVec3(&line[2]);
                vertices.Append({ vertexPos, 0 });
            }
            else if (line[1] == 'n') {
                printf("vertex normal! UNHANDLED\n");
            }
            else {
                // idk
            }
        }
        else if (line[0] == 'f') {
            char* el = &line[2];
            int i = 0;
            for (char* c = &line[2]; *c != '\0'; c++) {
                if (*c == ' ' || *c == '\n') {
                    char* endptr = el;
                    *c = '\0';
                    faceInds.Append((int)strtol(el, &endptr, 10));
                    // TODO check error
                    el = c + 1;
                }
            }
            faceInds.Append(-1);
        }
        else {
            // idk
        }
    }

    DynamicArray<HalfEdge> edgesForward;
    edgesForward.Init();
    DynamicArray<HalfEdge> edgesBackward;
    edgesBackward.Init();
    DynamicArray<Face> faces;
    faces.Init();

    uint32 faceVerts = 0;
    for (uint32 i = 0; i < faceInds.size; i++) {
        if (faceInds.data[i] == -1 || i == faceInds.size - 1) {
            // Create face
            Face face;
            face.halfEdge = edgesForward.size - 1;
            faces.Append(face);

            // Connect edge "next" endpoints
            edgesForward.data[edgesForward.size - 1].next =
                edgesForward.size - faceVerts;
            edgesForward.data[edgesForward.size - 1].vertex =
                faceInds.data[i - faceVerts];
            edgesBackward.data[edgesBackward.size - faceVerts].next =
                edgesBackward.size - 1;
            
            faceVerts = 0;
            continue;
        }

        // Assign forward edge i -> i + 1 to vertex i
        vertices.data[faceInds.data[i] - 1].halfEdge = edgesForward.size;

        // Create forward edge, i -> i + 1
        // Last edge will have wrong "next"
        HalfEdge forward;
        forward.next = edgesForward.size + 1;
        forward.vertex = faceInds.data[i + 1] - 1;
        forward.face = faces.size;

        // Create backward edge, i + 1 -> i
        // First edge will have wrong "next"
        HalfEdge backward;
        backward.next = edgesBackward.size - 1;
        backward.vertex = faceInds.data[i] - 1;
        backward.face = faces.size;

        edgesForward.Append(forward);
        edgesBackward.Append(backward);
        faceVerts++;
    }

    if (edgesForward.size != edgesBackward.size) {
        printf("ERROR: should be an assert\n");
        // TODO assert
    }

    mesh.numVertices = vertices.size;
    mesh.vertices = vertices.data;

    mesh.numEdges = edgesForward.size;
    mesh.edgesForward = edgesForward.data;
    mesh.edgesBackward = edgesBackward.data;

    mesh.numFaces = faces.size;
    mesh.faces = faces.data;

    printf("----- VERTICES -----\n");
    for (uint32 i = 0; i < mesh.numVertices; i++) {
        printf("%d: (%f, %f, %f) ; edge %d\n",
            i,
            mesh.vertices[i].pos.x,
            mesh.vertices[i].pos.y,
            mesh.vertices[i].pos.z,
            mesh.vertices[i].halfEdge);
    }
    printf("----- FORWARD EDGES -----\n");
    for (uint32 i = 0; i < mesh.numEdges; i++) {
        printf("%d: next %d, vert %d, face %d\n",
            i,
            mesh.edgesForward[i].next,
            mesh.edgesForward[i].vertex,
            mesh.edgesForward[i].face);
    }
    printf("----- BACKWARD EDGES -----\n");
    for (uint32 i = 0; i < mesh.numEdges; i++) {
        printf("%d: next %d, vert %d, face %d\n",
            i,
            mesh.edgesBackward[i].next,
            mesh.edgesBackward[i].vertex,
            mesh.edgesBackward[i].face);
    }
    printf("----- FACES -----\n");
    for (uint32 i = 0; i < mesh.numFaces; i++) {
        printf("%d: edge %d\n",
            i,
            mesh.faces[i].halfEdge);
    }

    // TODO free mesh after this
    return mesh;
}