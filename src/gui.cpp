#include "gui.h"

#include "km_math.h"
#include "ogl_base.h"
#include "text.h"

ClickableBox CreateClickableBox(Vec2 origin, Vec2 size,
    Vec4 color, Vec4 hoverColor, Vec4 pressColor)
{
    ClickableBox box = {};
    box.origin = origin;
    box.size = size;

    box.hovered = false;
    box.pressed = false;

    box.color = color;
    box.hoverColor = hoverColor;
    box.pressColor = pressColor;

    return box;
}

Button CreateButton(Vec2 origin, Vec2 size,
    const char* text, ButtonCallback callback,
    Vec4 color, Vec4 hoverColor, Vec4 pressColor)
{
    Button button = {};
    button.box = CreateClickableBox(origin, size,
        color, hoverColor, pressColor);

    uint32 textLen = (uint32)strnlen(text, INPUT_BUFFER_SIZE - 1);
    strncpy(button.text, text, textLen);
    button.text[textLen] = '\0';
    button.callback = callback;

    return button;
}

InputField CreateInputField(Vec2 origin, Vec2 size,
    Vec4 color, Vec4 hoverColor, Vec4 pressColor)
{
    InputField inputField = {};
    inputField.box = CreateClickableBox(origin, size,
        color, hoverColor, pressColor);

    inputField.text[0] = '\0';
    inputField.textLen = 0;

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
            boxes[i].pressed = (clickState & CLICKSTATE_LEFT_PRESS) != 0;
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
        Vec4 color = boxes[i].color;
        if (boxes[i].hovered) {
            color = boxes[i].hoverColor;
        }
        if (boxes[i].pressed) {
            color = boxes[i].pressColor;
        }

        DrawRect(rectGL, pos, Vec2::zero, boxes[i].size, color);
    }
}

void UpdateButtons(Button buttons[], uint32 n, Vec2 mousePos, int clickState)
{
    for (uint32 i = 0; i < n; i++) {
        bool wasPressed = buttons[i].box.pressed;
        UpdateClickableBoxes(&buttons[i].box, n, mousePos, clickState);

        if (buttons[i].box.hovered && wasPressed && !buttons[i].box.pressed) {
            buttons[i].callback();
        }
    }
}

void DrawButtons(Button buttons[], uint32 n,
    RectGL rectGL, TextGL textGL, const FontFace& face)
{
    for (uint32 i = 0; i < n; i++) {
        DrawClickableBoxes(&buttons[i].box, n, rectGL);
        Vec3 textPos = {
            buttons[i].box.origin.x, buttons[i].box.origin.y, 0.0f
        };
        DrawText(textGL, face, buttons[i].text, textPos, Vec4::black);
    }
}

void UpdateInputFields(InputField fields[], uint32 n,
    Vec2 mousePos, int clickState, KeyEvent* keyBuf, uint32 keyBufSize)
{
    static int focus = -1;
    bool anyPressed = false;
    for (uint32 i = 0; i < n; i++) {
        UpdateClickableBoxes(&fields[i].box, n, mousePos, clickState);
        
        if (fields[i].box.pressed) {
            // TODO picks the last one for now. sort based on Z
            printf("new input focus: %d\n", i);
            focus = i;
            anyPressed = true;
        }
    }

    if (focus != -1 && (clickState & CLICKSTATE_LEFT_PRESS) != 0
    && !anyPressed) {
        printf("lost focus\n");
        focus = -1;
    }

    // TODO mysterious bug: can't type in more than 16 characters...
    if (focus != -1 && keyBufSize != 0) {
        printf("size: %d\n", (int)sizeof(fields[focus]));
        for (uint32 i = 0; i < keyBufSize; i++) {
            if (!keyBuf[i].pressed) {
                continue;
            }

            if (keyBuf[i].ascii == 8) {
                printf(">> backspaced\n");
                if (fields[focus].textLen > 0) {
                    fields[focus].textLen--;
                }
            }
            else if (fields[focus].textLen < INPUT_BUFFER_SIZE - 1) {
                printf("added %c\n", keyBuf[i].ascii);
                printf("textLen before: %d\n", fields[focus].textLen);
                fields[focus].text[fields[focus].textLen++] = keyBuf[i].ascii;
            }
        }
        fields[focus].text[fields[focus].textLen] = '\0';
        printf("new focus (%d) length: %d\n", focus, fields[focus].textLen);
        for (uint32 i = 0; i < fields[focus].textLen; i++) {
            printf("chardump: %c\n", fields[focus].text[i]);
        }
        printf("new text: %s\n", fields[focus].text);
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