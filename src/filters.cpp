#include "filters.h"

void FilterButtonNone(Button* button, void* data)
{
    printf("Filter not implemented (%s)\n", button->text);
}

void FilterButtonTranslate(Button* button, void* data)
{
    printf("add translation\n");
}

void FilterButtonRotate(Button* button, void* data)
{
    printf("add rotation\n");
}

void FilterButtonScale(Button* button, void* data)
{
    printf("add scaling\n");
}

// Filter implementations

void FilterTranslate(SharedState* state, void* data)
{
    TranslateArgs* args = (TranslateArgs*)data;
    
    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        state->mesh.vertices[i].pos += args->offset;
    }
}
void FilterRotate(SharedState* state, void* data)
{
    RotateArgs* args = (RotateArgs*)data;
}
void FilterScale(SharedState* state, void* data)
{
    ScaleArgs* args = (ScaleArgs*)data;
}