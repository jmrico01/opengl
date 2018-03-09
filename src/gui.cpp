#include "gui.h"

#include "km_math.h"
#include "ogl_base.h"
#include "text.h"

ClickableBox CreateClickableBox(Vec2 origin, Vec2 size)
{
    ClickableBox box = {};
    box.origin = origin;
    box.size = size;

    return box;
}

InputField CreateInputField(Vec2 origin, Vec2 size)
{
    InputField inputField = {};
    ClickableBox box = {};
    box.origin = origin;
    box.size = size;
    
    inputField.box = box;
    inputField.text[0] = '\0';

    inputField.text[0] = 'H';
    inputField.text[1] = 'e';
    inputField.text[2] = 'l';
    inputField.text[3] = 'l';
    inputField.text[4] = 0;

    return inputField;
}

void UpdateClickableBoxes(ClickableBox boxes[], uint32 n,
    Vec2 mousePos, int clickState)
{
    for (uint32 i = 0; i < n; i++) {
        Vec2 boxOrigin = boxes[i].origin;
        Vec2 boxSize = boxes[i].size;

        if ((mousePos.x >= boxOrigin.x && mousePos.x <= boxOrigin.x + boxSize.x)
        && (mousePos.y >= boxOrigin.y && mousePos.y <= boxOrigin.y + boxSize.y)) {
            boxes[i].hovered = true;
            boxes[i].pressed = clickState == CLICKSTATE_LEFT_PRESS;
        }
        else {
            boxes[i].hovered = false;
            boxes[i].pressed = false;
        }
    }
}

void DrawClickableBoxes(ClickableBox boxes[], uint32 n, RectGL rectGL)
{
    for (uint32 i = 0; i < n; i++) {
        Vec3 pos = { boxes[i].origin.x, boxes[i].origin.y, 0.0f };
        Vec4 color = { 0.1f, 0.3f, 0.5f, 1.0f };
        if (boxes[i].hovered) {
            color.a = 0.5f;
        }
        if (boxes[i].pressed) {
            color.a = 0.0f;
        }

        DrawRect(rectGL, pos, Vec2::zero, boxes[i].size, color);
    }
}

void UpdateInputFields(InputField fields[], uint32 n,
    Vec2 mousePos, int clickState)
{
    for (uint32 i = 0; i < n; i++) {
        UpdateClickableBoxes(&fields[i].box, n, mousePos, clickState);
    }
}

void DrawInputFields(InputField fields[], uint32 n,
    RectGL rectGL, TextGL textGL, const FontFace& face)
{
    for (uint32 i = 0; i < n; i++) {
        DrawClickableBoxes(&fields[i].box, n, rectGL);
        Vec3 textPos = { fields[i].box.origin.x, fields[i].box.origin.y, 0.0f };
        DrawText(textGL, face, fields[i].text, textPos, Vec4::black);
    }
}