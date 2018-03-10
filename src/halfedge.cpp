#include "halfedge.h"

#include <stdio.h>
#include <map>

#include "main.h"
#include "km_math.h"
#include "ogl_base.h"

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
    HalfEdgeMesh mesh;
    mesh.vertices.Init();
    mesh.faces.Init();
    mesh.halfEdges.Init();

    char fullPath[PATH_MAX_LENGTH];
    CatStrings(GetAppPath(), filePath, fullPath, PATH_MAX_LENGTH);
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Failed to open OBJ file at: %s\n", filePath);
        return mesh;
    }

    char line[OBJ_LINE_MAX];
    DynamicArray<int> faceInds;
    faceInds.Init();

    while (fgets(line, OBJ_LINE_MAX, file)) {
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            if (line[1] == ' ') {
                Vertex vertex;
                vertex.pos = ParseVec3(&line[2]);
                vertex.halfEdge = 0; // This is set later
                mesh.vertices.Append(vertex);
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

    std::map<std::pair<uint32, uint32>, uint32> edgeMap;

    uint32 faceVerts = 0;
    for (uint32 i = 0; i < faceInds.size; i++) {
        if (faceInds[i] == -1 || i == faceInds.size - 1) {
            // Create face
            Face face;
            face.halfEdge = mesh.halfEdges.size - 1;
            mesh.faces.Append(face);
            
            faceVerts = 0;
            continue;
        }

        uint32 vertSrc = faceInds[i] - 1;
        uint32 vertDst = faceInds[i + 1] - 1;
        if (faceInds[i + 1] == -1) {
            vertDst = faceInds[i - faceVerts] - 1;
        }

        // Create edge, i -> i + 1
        // Last edge will have wrong "next"
        std::pair<int, int> forward(vertSrc, vertDst);
        std::pair<int, int> backward(vertDst, vertSrc);
        HalfEdge he;
        he.next = mesh.halfEdges.size + 1;
        if (faceInds[i + 1] == -1) {
            he.next = mesh.halfEdges.size - faceVerts;
        }
        auto edgeBackward = edgeMap.find(backward);
        if (edgeBackward != edgeMap.end()) {
            he.twin = edgeBackward->second;
            mesh.halfEdges[edgeBackward->second].twin =
                mesh.halfEdges.size;
        }
        else {
            he.twin = 0;
        }
        he.vertex = vertDst;
        he.face = mesh.faces.size;

        // Assign forward edge i -> i + 1 to vertex i
        mesh.vertices[vertSrc].halfEdge = mesh.halfEdges.size;

        // Add edge to edgeMap and mesh.halfEdges array
        auto res = edgeMap.insert(
            std::make_pair(forward, mesh.halfEdges.size));
        if (!res.second) {
            printf("ERROR: Edge already in edgeMap\n");
        }
        mesh.halfEdges.Append(he);
        faceVerts++;
    }

    printf("----- VERTICES -----\n");
    for (uint32 i = 0; i < mesh.vertices.size; i++) {
        printf("%d: (%f, %f, %f) ; edge %d\n",
            i,
            mesh.vertices[i].pos.x,
            mesh.vertices[i].pos.y,
            mesh.vertices[i].pos.z,
            mesh.vertices[i].halfEdge);
    }
    printf("----- HALF EDGES -----\n");
    for (uint32 i = 0; i < mesh.halfEdges.size; i++) {
        printf("%d: next %d, twin: %d, vert %d, face %d\n",
            i,
            mesh.halfEdges[i].next, mesh.halfEdges[i].twin,
            mesh.halfEdges[i].vertex,
            mesh.halfEdges[i].face);
    }
    printf("----- FACES -----\n");
    for (uint32 i = 0; i < mesh.faces.size; i++) {
        printf("%d: edge %d\n",
            i,
            mesh.faces[i].halfEdge);
    }

    faceInds.Free();

    // TODO free mesh after this
    return mesh;
}

// Make a new edge v1 -> v2, splitting face f.
// f will now be v1 -> ... -> v2 -> v1
// new face will be v2 -> ... -> v1 -> v2
void SplitFaceMakeEdge(HalfEdgeMesh* mesh, uint32 f, uint32 v1, uint32 v2)
{
    uint32 edge = mesh->faces[f].halfEdge;
    uint32 eToV1 = edge;
    while (mesh->halfEdges[eToV1].vertex != v1) {
        eToV1 = mesh->halfEdges[eToV1].next;
    }
    uint32 eToV2 = edge;
    while (mesh->halfEdges[eToV2].vertex != v2) {
        eToV2 = mesh->halfEdges[eToV2].next;
    }

    uint32 e = eToV2;
    while (mesh->halfEdges[e].vertex != v1) {
        e = mesh->halfEdges[e].next;
        // Assign new face to edges v2 -> ... -> v1
        mesh->halfEdges[e].face = mesh->faces.size;
    }

    // Update original face f
    mesh->faces[f].halfEdge = mesh->halfEdges.size + 1; // v2 -> v1
    // Create new face
    Face newFace;
    newFace.halfEdge = mesh->halfEdges.size; // v1 -> v2

    // Create new half edges
    HalfEdge v1v2;
    v1v2.next = mesh->halfEdges[eToV2].next;
    v1v2.twin = mesh->halfEdges.size + 1;
    v1v2.vertex = v2;
    v1v2.face = mesh->faces.size;
    HalfEdge v2v1;
    v2v1.next = mesh->halfEdges[eToV1].next;
    v2v1.twin = mesh->halfEdges.size;
    v2v1.vertex = v1;
    v2v1.face = f;

    mesh->halfEdges.Append(v1v2);
    mesh->halfEdges.Append(v2v1);
    mesh->faces.Append(newFace);
}

void Triangulate(HalfEdgeMesh* mesh)
{
}

void FreeHalfEdgeMesh(HalfEdgeMesh* mesh)
{
    mesh->vertices.Free();
    mesh->faces.Free();
    mesh->halfEdges.Free();
}

HalfEdgeMeshGL LoadHalfEdgeMeshGL(const HalfEdgeMesh& mesh)
{
    HalfEdgeMeshGL meshGL = {};

    // TODO use indexing for this
    DynamicArray<Vec3> vertices;
    vertices.Init();

    for (uint32 i = 0; i < mesh.faces.size; i++) {
        uint32 numEdges = 0;
        uint32 edge = mesh.faces[i].halfEdge;
        uint32 e = edge;
        do {
            numEdges++;
            e = mesh.halfEdges[e].next;
        } while (e != edge);

        if (numEdges == 3) {
            do {
                vertices.Append(mesh.vertices[mesh.halfEdges[e].vertex].pos);
                e = mesh.halfEdges[e].next;
            } while (e != edge);
        }
        else if (numEdges == 4) {
            printf("4 edges in face, split\n");
        }
        else {
            // TODO error
            printf("ERROR: Unsupported %d-edge face\n", numEdges);
        }
    }

    glGenVertexArrays(1, &meshGL.vertexArray);
    glBindVertexArray(meshGL.vertexArray);

    glGenBuffers(1, &meshGL.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, meshGL.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size * sizeof(Vec3),
        vertices.data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // match shader layout location
        3, // size (vec3)
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    meshGL.programID = LoadShaders(
        "shaders/model.vert",
        "shaders/model.frag");

    return meshGL;
}

void FreeHalfEdgeMeshGL(const HalfEdgeMeshGL& meshGL)
{
}