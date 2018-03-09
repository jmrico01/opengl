#include "halfedge.h"

#include <stdio.h>

#include "main.h"
#include "km_lib.h"
#include "km_math.h"

#define OBJ_LINE_MAX 512

Vec3 ParseVec3(char* str)
{
    Vec3 result;
    char* c = str;
    char* e = str;
    int i = 0;
    bool done = false;
    while (!done) {
        if (*c == ' ' || *c == '\0') {
            if (*c == '\0') {
                done = true;
            }
            char* endptr = e;
            *c = '\0';
            result.e[i++] = strtod(e, &endptr);
            if (endptr != c) {
                // TODO uhh... idk
                //printf("failed to parse number\n");
            }
            e = c + 1;
        }

        c++;
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
    DynamicArray<Vec3> vertices;
    vertices.Init();

    while (fgets(line, OBJ_LINE_MAX, file)) {
        if (line[0] == '#') {
            continue;
        }
        else if (line[0] == 'v') {
            if (line[1] == ' ') {
                Vec3 vertex = ParseVec3(&line[2]);
                printf("vertex! %f, %f, %f\n",
                    vertex.x, vertex.y, vertex.z);
                vertices.Append(vertex);
            }
            else if (line[1] == 'n') {
                printf("vertex normal!\n");
            }
            else {
                // idk
            }
        }
        else if (line[0] == 'f') {
            printf("face!\n");
        }
        else {
            // idk
        }
    }

    return mesh;
}