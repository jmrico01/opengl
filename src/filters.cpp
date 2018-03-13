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

internal void FilterUpdateSingleFloat(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    SingleFloatData* data = (SingleFloatData*)entry->data;
    Vec2 pos2D = { pos.x, pos.y };
    Vec2 inputSize = { size.x, (float32)font.height };
    data->inputValue.box.origin = pos2D;
    data->inputValue.box.size = inputSize;
    UpdateInputFields(&data->inputValue, 1, mousePos, clickState,
        keyBuf, keyBufSize);
    DrawInputFields(&data->inputValue, 1, rectGL, textGL, font);

    data->value = (float32)strtod(data->inputValue.text, nullptr);
}

internal void FilterUpdateVec3(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    Vec2 size = {
        FILTER_BOX_WIDTH,
        2.0f * (float32)font.height + 10.0f
    };
    DrawFilterBase(entry, pos, size, rectGL, textGL, font,
        mousePos, clickState);

    Vec3Data* data = (Vec3Data*)entry->data;
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
        data->vector.e[i] =
            (float32)strtod(data->inputCoords[i].text, nullptr);
    }
}

internal void RemoveButtonPress(Button* button, void* data)
{
    FilterEntry* entry = (FilterEntry*)data;
    printf("Appending, size %d, capacity %d\n",
        filtersToDelete_.size, filtersToDelete_.capacity);
    filtersToDelete_.Append(entry->idx);
    printf("Queued for removal - filter #%d: %s\n", entry->idx, entry->name);
}

internal FilterEntry FilterButtonBase(Button* button)
{
    FilterEntry entry;
    entry.idx = filters_.size;
    entry.name = button->text;

    entry.removeButton = CreateButton(
        Vec2::zero, Vec2::zero,
        "X", RemoveButtonPress,
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

internal void FilterCreateSingleFloat(Button* button, void* data,
    const char* initVal, FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateSingleFloat;
    entry.applyFunc = filterFunc;

    SingleFloatData* d =
        (SingleFloatData*)malloc(sizeof(SingleFloatData));
    d->inputValue = CreateInputField(
        Vec2::zero,
        Vec2::zero,
        "1",
        { 1.0f, 1.0f, 1.0f, 0.2f },
        { 1.0f, 1.0f, 1.0f, 0.4f },
        { 1.0f, 1.0f, 1.0f, 0.5f },
        { 1.0f, 1.0f, 1.0f, 0.9f }
    );

    entry.data = (void*)d;

    filters_.Append(entry);
}

internal void FilterCreateVec3(Button* button, void* data,
    const char* initVal, FilterApplyFunc filterFunc)
{
    FilterEntry entry = FilterButtonBase(button);
    entry.updateFunc = FilterUpdateVec3;
    entry.applyFunc = filterFunc;

    Vec3Data* d = (Vec3Data*)malloc(sizeof(Vec3Data));
    for (int i = 0; i < 3; i++) {
        d->inputCoords[i] = CreateInputField(
            Vec2::zero,
            Vec2::zero,
            initVal,
            { 1.0f, 1.0f, 1.0f, 0.2f },
            { 1.0f, 1.0f, 1.0f, 0.4f },
            { 1.0f, 1.0f, 1.0f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 0.9f }
        );
    }

    entry.data = (void*)d;

    filters_.Append(entry);
}

void FilterButtonTranslate(Button* button, void* data)
{
    FilterCreateVec3(button, data, "0", FilterTranslate);
}

void FilterButtonRotate(Button* button, void* data)
{
    FilterCreateVec3(button, data, "0", FilterRotate);
}

void FilterButtonScale(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "1", FilterScale);
}

void FilterButtonTwist(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0.2", FilterTwist);
}

void FilterButtonNoise(Button* button, void* data)
{
    FilterCreateSingleFloat(button, data, "0.5", FilterNoise);
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
    Vec3Data* data = (Vec3Data*)entry->data;
    Vec3 offset = data->vector;

    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        state->mesh.vertices[i].pos += offset;
    }

    printf("Translated by (%f, %f, %f)\n", offset.x, offset.y, offset.z);
}
void FilterRotate(FilterEntry* entry, SharedState* state)
{
    Vec3Data* data = (Vec3Data*)entry->data;
    Vec3 rot = data->vector;
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
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float scale = data->value;

    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        state->mesh.vertices[i].pos *= scale;
    }

    printf("Scaled by %f\n", scale);
}

void FilterTwist(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float value = data->value;

    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        float y = state->mesh.vertices[i].pos.y;
        Quat rot = QuatFromAngleUnitAxis(y * value * PI_F, Vec3::unitY);
        state->mesh.vertices[i].pos = rot * state->mesh.vertices[i].pos;
    }

    printf("Twisted with value %f\n", value);
}

void FilterInflate(FilterEntry* entry, SharedState* state)
{
    // TODO need vertex normals
}

void FilterWacky(FilterEntry* entry, SharedState* state)
{
    // TODO need vertex normals
}

void FilterNoise(FilterEntry* entry, SharedState* state)
{
    SingleFloatData* data = (SingleFloatData*)entry->data;
    float value = data->value;

    for (uint32 i = 0; i < state->mesh.vertices.size; i++) {
        value *= i;
        // TODO need normal
    }
}