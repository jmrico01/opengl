#include "filters.h"

#define FILTER_BOX_WIDTH 200.0f

internal void DrawFilterBase(FilterEntry* entry, Vec3 pos, Vec2 size,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState)
{
    DrawRect(rectGL,
        pos, Vec2::zero, size,
        { 1.0f, 1.0f, 1.0f, 0.2f });
    Vec3 titlePos = pos;
    titlePos.y += size.y;
    DrawText(textGL, font, entry->name, titlePos,
        { 0.0f, 0.95f }, { 1.0f, 1.0f, 1.0f, 0.6f });

    if (entry->idx == -1) {
        // No remove button on ModelLoad filter
        return;
    }

    entry->removeButton.box.origin = {
        pos.x + size.x - (float32)font.height,
        pos.y + size.y - (float32)font.height
    };
    entry->removeButton.box.size = {
        (float32)font.height,
        (float32)font.height
    };
    UpdateButtons(&entry->removeButton, 1, mousePos, clickState, entry);
    DrawButtons(&entry->removeButton, 1, rectGL, textGL, font);
}

void FilterUpdateModelLoad(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    ModelLoadData* data = (ModelLoadData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
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
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    TranslateData* data = (TranslateData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    for (int i = 0; i < 3; i++) {
        Vec2 inputPos = pos2D;
        inputPos.x += i * 80.0f;
        Vec2 inputSize = { size.x / 3.0f, (float32)font.height };
        data->inputCoords[i].box.origin = inputPos;
        data->inputCoords[i].box.size = inputSize;
    }
    UpdateInputFields(data->inputCoords, 3, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(data->inputCoords, 3, rectGL, textGL, font);

    for (int i = 0; i < 3; i++) {
        data->offset.e[i] = (float32)strtod(data->inputCoords[i].text, nullptr);
    }
}

internal void FilterUpdateRotate(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    RotateData* data = (RotateData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    for (int i = 0; i < 3; i++) {
        Vec2 inputPos = pos2D;
        inputPos.x += i * 80.0f;
        Vec2 inputSize = { size.x / 3.0f, (float32)font.height };
        data->inputCoords[i].box.origin = inputPos;
        data->inputCoords[i].box.size = inputSize;
    }
    UpdateInputFields(data->inputCoords, 3, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(data->inputCoords, 3, rectGL, textGL, font);

    for (int i = 0; i < 3; i++) {
        data->rot.e[i] = (float32)strtod(data->inputCoords[i].text, nullptr);
    }
}

internal void CloseButtonPress(Button* button, void* data)
{
    FilterEntry* entry = (FilterEntry*)data;
    filtersToDelete_.Append(entry->idx);
    printf("Queued for deletion - filter #%d: %s\n", entry->idx, entry->name);
}

internal FilterEntry FilterButtonBase(Button* button)
{
    FilterEntry entry;
    entry.idx = filters_.size;
    entry.name = button->text;

    entry.removeButton = CreateButton(
        Vec2::zero, Vec2::zero,
        "X", CloseButtonPress,
        { 1.0f, 0.2f, 0.2f, 0.2f },
        { 1.0f, 0.2f, 0.2f, 0.4f },
        { 1.0f, 0.2f, 0.2f, 0.5f },
        { 1.0f, 0.7f, 0.7f, 1.0f }
    );

    return entry;
}

void FilterButtonNone(Button* button, void* data)
{
    printf("Filter not implemented (%s)\n", button->text);
}

void FilterButtonTranslate(Button* button, void* data)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateTranslate;
    entry.applyFunc = FilterTranslate;

    TranslateData* translateData =
        (TranslateData*)malloc(sizeof(TranslateData));
    for (int i = 0; i < 3; i++) {
        translateData->inputCoords[i] = CreateInputField(
            Vec2::zero,
            Vec2::zero,
            "0",
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
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateRotate;
    entry.applyFunc = FilterRotate;

    RotateData* rotateData =
        (RotateData*)malloc(sizeof(RotateData));
    for (int i = 0; i < 3; i++) {
        rotateData->inputCoords[i] = CreateInputField(
            Vec2::zero,
            Vec2::zero,
            "0",
            { 1.0f, 1.0f, 1.0f, 0.2f },
            { 1.0f, 1.0f, 1.0f, 0.4f },
            { 1.0f, 1.0f, 1.0f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 0.9f }
        );
    }

    entry.data = (void*)rotateData;

    filters_.Append(entry);
}

void FilterButtonScale(Button* button, void* data)
{
    printf("add scaling\n");
}

// ========== Filter implementations ==========
void FilterModelLoad(FilterEntry* entry, SharedState* state)
{
    ModelLoadData* data = (ModelLoadData*)entry->data;

    FreeHalfEdgeMesh(&state->mesh);
    state->mesh = HalfEdgeMeshFromObj(data->inputField.text);
    if (state->mesh.vertices.size == 0) {
        printf("ERROR: model not loaded\n");
        return;
    }

    printf("Loaded model: %s\n", data->inputField.text);
}

void FilterTranslate(FilterEntry* entry, SharedState* state)
{
    TranslateData* data = (TranslateData*)entry->data;
    Vec3 offset = data->offset;
    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        state->mesh.vertices[i].pos += offset;
    }

    printf("Translated by (%f, %f, %f)\n", offset.x, offset.y, offset.z);
}
void FilterRotate(FilterEntry* entry, SharedState* state)
{
    RotateData* data = (RotateData*)entry->data;
    Vec3 rot = data->rot;
    Quat rotQuat =
        QuatFromAngleUnitAxis(rot.z, Vec3::unitZ)
        * QuatFromAngleUnitAxis(rot.y, Vec3::unitY)
        * QuatFromAngleUnitAxis(rot.x, Vec3::unitX);

    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        state->mesh.vertices[i].pos = rotQuat * state->mesh.vertices[i].pos;
    }

    printf("Rotated by (%f, %f, %f)\n", rot.x, rot.y, rot.z);
}
void FilterScale(FilterEntry* entry, SharedState* state)
{
    ScaleData* data = (ScaleData*)entry->data;
}