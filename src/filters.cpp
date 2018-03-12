#include "filters.h"

void FilterUpdateModelLoad(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    ModelLoadData* data = (ModelLoadData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };

    Vec2 modelFilterSize = {
        200.0f,
        2.0f * (float32)font.height + 10.0f
    };
    DrawRect(rectGL,
        pos, Vec2::zero, modelFilterSize,
        { 1.0f, 1.0f, 1.0f, 0.2f });
    Vec3 modelFilterTextPos = pos;
    modelFilterTextPos.y += modelFilterSize.y;
    DrawText(textGL, font, "Model", modelFilterTextPos,
        { 0.0f, 0.95f }, { 1.0f, 1.0f, 1.0f, 0.6f });

    data->inputField.box.origin = pos2D;
    data->inputField.box.origin.y += 4.0f;
    UpdateInputFields(&data->inputField, 1, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(&data->inputField, 1, rectGL, textGL, font);
}

internal void FilterUpdateTranslate(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
}

void FilterButtonNone(Button* button, void* data)
{
    printf("Filter not implemented (%s)\n", button->text);
}

void FilterButtonTranslate(Button* button, void* data)
{
    FilterEntry entry;
    entry.updateFunc = FilterUpdateTranslate;
    entry.applyFunc = FilterTranslate;

    TranslateData* translateData =
        (TranslateData*)malloc(sizeof(TranslateData));
    for (int i = 0; i < 3; i++) {
        translateData->inputCoords[i] = CreateInputField(
            Vec2::zero,
            { 75.0f, (float32)cmSerif16.height },
            MODEL_START,
            { 1.0f, 1.0f, 1.0f, 0.2f },
            { 1.0f, 1.0f, 1.0f, 0.4f },
            { 1.0f, 1.0f, 1.0f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 0.9f }
        );
    }

    entry.data = (void*)translateData;

    filters_.Append(entry);
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
void FilterModelLoad(FilterEntry* entry, SharedState* state)
{
    ModelLoadData* data = (ModelLoadData*)entry->data;

    FreeHalfEdgeMeshGL(state->meshGL);
    FreeHalfEdgeMesh(&state->mesh);
    state->mesh = HalfEdgeMeshFromObj(data->inputField.text);
    if (state->mesh.vertices.size == 0) {
        printf("ERROR: model not loaded\n");
        return;
    }
    state->meshGL = LoadHalfEdgeMeshGL(state->mesh);

    printf("Loaded model: %s\n", data->inputField.text);
}

void FilterTranslate(FilterEntry* entry, SharedState* state)
{
    TranslateData* data = (TranslateData*)entry->data;
    Vec3 offset = data->offset;

    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        state->mesh.vertices[i].pos += offset;
    }
}
void FilterRotate(FilterEntry* entry, SharedState* state)
{
    RotateData* data = (RotateData*)entry->data;
}
void FilterScale(FilterEntry* entry, SharedState* state)
{
    ScaleData* data = (ScaleData*)entry->data;
}