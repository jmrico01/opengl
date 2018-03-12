#pragma once

#include "km_types.h"
#include "km_lib.h"
#include "gui.h"

struct FilterEntry;

typedef void (*FilterUpdateFunc)(FilterEntry*, Vec3,
    RectGL, TextGL, const FontFace&,
    Vec2, int, KeyEvent*, uint32);

typedef void (*FilterApplyFunc)(FilterEntry*, SharedState*);

struct FilterEntry
{
    int idx;
    const char* name;
    FilterUpdateFunc updateFunc;
    FilterApplyFunc applyFunc;
    Button removeButton;
    void* data; // data struct, malloc'd
};

void FilterUpdateModelLoad(FilterEntry* entry, Vec3 pos,
    RectGL rectGL, TextGL textGL, const FontFace& font,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize);

// Button press (add to filter list)
void FilterButtonNone(Button* button, void* data);

void FilterButtonTranslate(Button* button, void* data);
void FilterButtonRotate(Button* button, void* data);
void FilterButtonScale(Button* button, void* data);

void FilterButtonTwist(Button* button, void* data);
void FilterButtonInflate(Button* button, void* data);
void FilterButtonWacky(Button* button, void* data);

// Filter actions (bread & butter)
struct ModelLoadData
{
    InputField inputField;
};
void FilterModelLoad(FilterEntry* entry, SharedState* state);

struct TranslateData
{
    InputField inputCoords[3];
    Vec3 offset;
};
void FilterTranslate(FilterEntry* entry, SharedState* state);

struct RotateData
{
    InputField inputCoords[3];
    Vec3 rot;
};
void FilterRotate(FilterEntry* entry, SharedState* state);

struct ScaleData
{
    float scale;
};
void FilterScale(FilterEntry* entry, SharedState* state);

struct FilterInfo {
    const char* name;
    ButtonCallback callback;
} filterInfo_[] = {
    { "Translate",  FilterButtonTranslate },
    { "Rotate",     FilterButtonRotate },
    { "Scale",      FilterButtonScale },
    { "",           FilterButtonNone },
    { "Twist",      FilterButtonNone },
    { "Inflate",    FilterButtonNone },
    { "Wacky",      FilterButtonNone }
};
const int numFilters_ = sizeof(filterInfo_) / sizeof(filterInfo_[0]);

global_var DynamicArray<FilterEntry> filters_;
global_var DynamicArray<int> filtersToDelete_;