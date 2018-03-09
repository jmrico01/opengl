#pragma once

#include "km_math.h"
#include "ogl_base.h"
#include "text.h"

#define INPUT_BUFFER_SIZE 512

enum ClickStateFlags {
    CLICKSTATE_NONE = 0,
    CLICKSTATE_LEFT_PRESS = 1 << 0,
    CLICKSTATE_RIGHT_PRESS = 1 << 1
};

/*struct ClickState
{
    bool leftPressed;
    bool rightPressed;
};*/

struct ClickableBox
{
    Vec2 origin;
    Vec2 size;

    bool hovered;
    bool pressed;
};

struct InputField
{
    ClickableBox box;
    char text[INPUT_BUFFER_SIZE];
};

ClickableBox CreateClickableBox(Vec2 origin, Vec2 size);
InputField CreateInputField(Vec2 origin, Vec2 size);

void UpdateClickableBoxes(ClickableBox boxes[], uint32 n,
    Vec2 mousePos, int clickState);
void DrawClickableBoxes(ClickableBox boxes[], uint32 n, RectGL rectGL);

void UpdateInputFields(InputField fields[], uint32 n,
    Vec2 mousePos, int clickState);
void DrawInputFields(InputField fields[], uint32 n,
    RectGL rectGL, TextGL textGL, const FontFace& face);