#pragma once

#include "km_lib.h"
#include "gui.h"

struct FilterEntry
{
    int type;   // translate = 0, rotate = 1, etc...
    void* data; // args
};

// Button press (add to filter list)
void FilterButtonNone(Button* button, void* data);

void FilterButtonTranslate(Button* button, void* data);
void FilterButtonRotate(Button* button, void* data);
void FilterButtonScale(Button* button, void* data);

void FilterButtonTwist(Button* button, void* data);
void FilterButtonInflate(Button* button, void* data);
void FilterButtonWacky(Button* button, void* data);

// Filter actions (bread & butter)
struct TranslateArgs
{
    Vec3 offset;
};
void FilterTranslate(SharedState* state, void* data);

struct RotateArgs
{
    Vec3 rot;
};
void FilterRotate(SharedState* state, void* data);

struct ScaleArgs
{
    float scale;
};
void FilterScale(SharedState* state, void* data);