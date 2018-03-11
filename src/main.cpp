#include "main.h"

#ifdef _WIN32
#define GLEW_STATIC     // required by Windows
#include <Windows.h>    // ensure this is included before GLFW
#endif

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "km_defines.h"
#include "km_types.h"
#include "km_math.h"
#include "ogl_base.h"
#include "text.h"
#include "gui.h"
#include "load_bmp.h"
#include "halfedge.h"

// TODO (on linux at least) the window's top bar is part
// of the viewport, so the render scene is cropped.

// TODO get rid of this... getting tired of writing that
global_var char pathToApp_[PATH_MAX_LENGTH];
global_var int width_ = 1024;
global_var int height_ = 768;
global_var int clickState_ = CLICKSTATE_NONE;

global_var KeyEvent keyInputBuffer[KEY_INPUT_BUFFER_MAX];
global_var uint32 keyInputBufferSize = 0;

internal void RemoveFileNameFromPath(
    char* filePath, char* dest, uint64 destLength)
{
    unsigned int lastSlash = 0;
#ifdef _WIN32
    char pathSep = '\\';
#else
    char pathSep = '/';
#endif
    while (filePath[lastSlash] != '\0') {
        lastSlash++;
    }
    // TODO unsafe!
    while (filePath[lastSlash] != pathSep) {
        lastSlash--;
    }
    if (lastSlash + 2 > destLength) {
        return;
    }
    for (unsigned int i = 0; i < lastSlash + 1; i++) {
        dest[i] = filePath[i];
    }
    dest[lastSlash + 1] = '\0';
}

internal void ErrorCallback(int err, const char* msg)
{
    printf("GLFW error code %d: %s\n", err, msg);
}

// NOTE: for now, I'm assuming input callbacks occur on the same thread
// as the main loop. have to research that.
internal void CharModsCallback(GLFWwindow* window,
    uint32 codepoint, int mods)
{
    KeyEvent keyEvent = {
        (char)codepoint,
        true
    };

    keyInputBuffer[keyInputBufferSize++] = keyEvent;
}
internal void KeyCallback(GLFWwindow* window,
    int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE) {
        if (action == GLFW_PRESS) {
            // TODO exit
            printf("TODO: ESC detected, application should exit...");
        }
    }

    if (key == GLFW_KEY_BACKSPACE) {
        KeyEvent keyEvent;
        keyEvent.ascii = 8;
        if (action == GLFW_PRESS) {
            keyEvent.pressed = true;
        }
        else if (action == GLFW_RELEASE) {
            keyEvent.pressed = false;
        }
        else {
            return;
        }

        keyInputBuffer[keyInputBufferSize++] = keyEvent;
    }
}
internal void MouseCallback(GLFWwindow* window,
    int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            clickState_ |= CLICKSTATE_LEFT_PRESS;
        }
        else if (action == GLFW_RELEASE) {
            clickState_ &= ~CLICKSTATE_LEFT_PRESS;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            clickState_ |= CLICKSTATE_RIGHT_PRESS;
        }
        else if (action == GLFW_RELEASE) {
            clickState_ &= ~CLICKSTATE_RIGHT_PRESS;
        }
    }
}

internal void FramebufferSizeCallback(
    GLFWwindow* window, int width, int height)
{
    width_ = width;
    height_ = height;
    ResizeGL(width, height);
}

char* GetAppPath()
{
    return pathToApp_;
}

// TODO this is kind of quick and dirty.
void CatStrings(const char* str1, const char* str2, char* dest, uint64 destLen)
{
    int i = 0;
    unsigned int iDest = 0;

    if (destLen == 0) return;

    while (str1[i] != '\0') {
        if (iDest >= destLen - 1) {
            dest[destLen - 1] = '\0';
            return;
        }
        dest[iDest++] = str1[i++];
    }
    i = 0;
    while (str2[i] != '\0') {
        if (iDest >= destLen - 1) {
            dest[destLen - 1] = '\0';
            return;
        }
        dest[iDest++] = str2[i++];
    }

    dest[iDest] = '\0'; // this might be unsafe? don't wanna think about it
}

ReadFileResult ReadFile(const char* path)
{
    ReadFileResult result = {};
    char fullPath[PATH_MAX_LENGTH];
    CatStrings(GetAppPath(), path, fullPath, PATH_MAX_LENGTH);
    FILE *f = fopen(fullPath, "rb");
    if (!f) {
        printf("Couldn't open file: %s\n", path);
        return result;
    }
    fseek(f, 0, SEEK_END);
    result.size = (uint64)ftell(f);
    fseek(f, 0, SEEK_SET);

    result.data = malloc((size_t)(result.size));
    fread(result.data, (size_t)result.size, 1, f);
    fclose(f);

    return result;
}

void CallbackTest()
{
    printf("Button has been pressed\n");
}

int main(int argc, char* argv[])
{
    RemoveFileNameFromPath(argv[0], pathToApp_, PATH_MAX_LENGTH);
    printf("Path to C application: %s\n", pathToApp_);

    // Initialize GLFW.
    if (glfwInit() != GLFW_TRUE) {
        return 1;
    }
    // TODO do this only in debug build?
    glfwSetErrorCallback(&ErrorCallback);
    
    // Try to get OpenGL 3.3 context on window creation.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Create GLFW window.
    GLFWwindow* window = glfwCreateWindow(
        width_, height_, "OpenGL", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    printf("GLFW: Created window\n");

    // Set window's GL context as the current context.
    glfwMakeContextCurrent(window);
    // Initialize GLEW for OpenGL 3.3 procedures.
    glewExperimental = true;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        glfwTerminate();
        printf("GLEW: Error - %s\n", glewGetErrorString(err));
    }
    printf("GLEW: Initialized\n");

    // Verify that we obtained an OpenGL 3.3+ context.
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    int glVerMajor, glVerMinor;
    glGetIntegerv(GL_MAJOR_VERSION, &glVerMajor);
    glGetIntegerv(GL_MINOR_VERSION, &glVerMinor);
    if (glVerMajor < 3 || (glVerMajor == 3 && glVerMinor < 3)) {
        glfwTerminate();
        printf("Requires OpenGL 3.3 or later.\n");
        printf("If your hardware supports it, update your drivers.\n");
        return 1;
    }
    printf("GLSL version:   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GL renderer:    %s\n", glGetString(GL_RENDERER));
    
    // Initialize app-specific OpenGL.
    // TODO error checking?
    InitOpenGL();
    ResizeGL(width_, height_);
    
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        printf("FreeType init error: %d\n", error);
        return 1;
    }

    // NOTE this isn't like "create a rect instance", but more like
    // "initialize rect drawing in general"
    RectGL rectGL = CreateRectGL();
    //TexturedRectGL texturedRectGL = CreateTexturedRectGL();
    TextGL textGL = CreateTextGL();
    LineGL lineGL = CreateLineGL();

    GLuint textureKM = OpenGLLoadBMP("data/images/kapricorn.bmp");
    FontFace cmSerif = LoadFontFace(
        library, "data/fonts/computer-modern/serif.ttf", 32);
    FontFace cmSerifBold = LoadFontFace(
        library, "data/fonts/computer-modern/serif-bold.ttf", 48);
    FontFace cmSerifBold128 = LoadFontFace(
        library, "data/fonts/computer-modern/serif-bold.ttf", 128);

    DynamicArray<ClickableBox> boxes;
    boxes.Init();
    DynamicArray<Button> buttons;
    buttons.Init();
    DynamicArray<InputField> fields;
    fields.Init();
    {
        /*Vec2 guiBoxOrigin = { 100.0f, 100.0f };
        Vec2 guiBoxSize = { 100.0f, 80.0f };
        boxes.Append(CreateClickableBox(guiBoxOrigin, guiBoxSize));

        Vec2 fieldOrigin = { 250.0f, 100.0f };
        Vec2 fieldSize = { 400.0f, 32.0f };
        fields.Append(CreateInputField(fieldOrigin, fieldSize));
        fieldOrigin = { 250.0f, 200.0f };
        fields.Append(CreateInputField(fieldOrigin, fieldSize));
        fieldOrigin = { 250.0f, 300.0f };
        fields.Append(CreateInputField(fieldOrigin, fieldSize));*/

        Vec2 buttonOrigin = { 100.0f, 200.0f };
        Vec2 buttonSize = { 200.0f, 32.0f };
        buttons.Append(CreateButton(buttonOrigin, buttonSize,
            "Button Test", CallbackTest,
            Vec4::zero,
            Vec4 {1.0f, 0.0f, 0.0f, 1.0f},
            Vec4 {0.0f, 1.0f, 0.0f, 1.0f}));
    }

    HalfEdgeMesh mesh = HalfEdgeMeshFromObj("data/models/octopus.obj");
    if (mesh.vertices.size == 0) {
        printf("not loaded\n");
    }
    HalfEdgeMeshGL meshGL = LoadHalfEdgeMeshGL(mesh);
    printf("meshGL: va %d, vb %d, pid %d\n",
        meshGL.vertexArray,
        meshGL.vertexBuffer,
        meshGL.programID);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    // Set up input handling.
    glfwSetCharModsCallback(window, &CharModsCallback);
    glfwSetKeyCallback(window, &KeyCallback);
    glfwSetMouseButtonCallback(window, &MouseCallback);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Catch all GL errors before loop
    {
        GLenum glError;
        while ((glError = glGetError()) != GL_NO_ERROR) {
            printf("GL ERROR: %x\n", glError);
        }
    }

    Vec3 cameraPos = { 0.0f, 0.0f, 5.0f };
    Vec3 modelRotation = Vec3::zero;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (uint32 i = 0; i < keyInputBufferSize; i++) {
            if (!keyInputBuffer[i].pressed) {
                continue;
            }
            const float ZOOM_STEP = 0.8f;
            const float ROT_STEP = 0.1f;

            if (keyInputBuffer[i].ascii == 'z'
            || keyInputBuffer[i].ascii == 'Z') {
                cameraPos.z += ZOOM_STEP;
            }
            if (keyInputBuffer[i].ascii == 'x'
            || keyInputBuffer[i].ascii == 'X') {
                cameraPos.z -= ZOOM_STEP;
            }

            if (keyInputBuffer[i].ascii == 'a'
            || keyInputBuffer[i].ascii == 'A') {
                modelRotation.y -= ROT_STEP;
            }
            if (keyInputBuffer[i].ascii == 'd'
            || keyInputBuffer[i].ascii == 'D') {
                modelRotation.y += ROT_STEP;
            }
            if (keyInputBuffer[i].ascii == 's'
            || keyInputBuffer[i].ascii == 'S') {
                modelRotation.x -= ROT_STEP;
            }
            if (keyInputBuffer[i].ascii == 'w'
            || keyInputBuffer[i].ascii == 'W') {
                modelRotation.x += ROT_STEP;
            }
            if (keyInputBuffer[i].ascii == 'q'
            || keyInputBuffer[i].ascii == 'Q') {
                modelRotation.z -= ROT_STEP;
            }
            if (keyInputBuffer[i].ascii == 'e'
            || keyInputBuffer[i].ascii == 'E') {
                modelRotation.z += ROT_STEP;
            }
        }
        // Clamp rotation angles
        for (int i = 0; i < 3; i++) {
            while (modelRotation.e[i] < -PI_F) {
                modelRotation.e[i] += 2.0f * PI_F;
            }
            while (modelRotation.e[i] > PI_F) {
                modelRotation.e[i] -= 2.0f * PI_F;
            }
        }

        // 3D rendering start
        glEnable(GL_DEPTH_TEST);

        Mat4 proj = Projection(110.0f, (float32)width_ / (float32)height_,
            0.1f, 10.0f);
        Mat4 view = Translate(-cameraPos) * Rotate(modelRotation);
        DrawHalfEdgeMeshGL(meshGL, proj, view);

        // Draw axis lines
        const float AXIS_LINE_LEN = 20.0f;
        for (int i = 0; i < 3; i++) {
            Vec3 v1 = Vec3::zero;
            v1.e[i] = -AXIS_LINE_LEN / 2.0f;
            Vec3 v2 = Vec3::zero;
            v2.e[i] = AXIS_LINE_LEN / 2.0f;
            Vec4 color = Vec4::zero;
            color.e[i] = 1.0f;
            color.a = 1.0f;

            DrawLine(lineGL, proj, view, v1, v2, color);
        }

        glDisable(GL_DEPTH_TEST);
        // 3D rendering end

        { // Test draws (primitives & text)
            /*Vec3 kmPos = { 100.0f, 100.0f, 0.0f };
            Vec2 kmSize = { 400.0f, 400.0f };
            DrawTexturedRect(texturedRectGL,
                kmPos, Vec2::zero, kmSize, textureKM);*/

            Vec3 textPos = { 100.0f, 600.0f, 0.0f };
            Vec4 textColor = { 0.1f, 0.1f, 0.1f, 1.0f };
            const char* text = "The quick brown fox jumps over the lazy dog.";
            int textWidth = GetTextWidth(cmSerif, text);
            DrawText(textGL, cmSerif, text, textPos, textColor);
            Vec3 underlinePos = textPos;
            underlinePos.y -= 2.0f;
            Vec2 underlineSize = { (float)textWidth, 1.0f };
            DrawRect(rectGL,
                underlinePos, Vec2::zero, underlineSize, textColor);

            Vec3 symbolsPos = { 100.0f, 550.0f, 0.0f };
            Vec4 symbolsColor = { 0.1f, 0.5f, 0.25f, 1.0f };
            const char* symbols = "~`!@#$%^&*()-_=+\\|[]{}'\";:/.,<>???";
            DrawText(textGL, cmSerif, symbols, symbolsPos, symbolsColor);

            Vec3 paragraphPos = { 500.0f, 300.0f, 0.0f };
            Vec4 paragraphColor = { 0.1f, 0.1f, 0.1f, 1.0f };
            float lineHeight = 32.0f;
            const char* p1 = "This is a line spacing test.";
            const char* p2 =
                "I want to see how far apart lines look by default.";
            const char* p3 =
                "Probably best to have line heights be face heights.";
            DrawText(textGL, cmSerif, p1, paragraphPos, paragraphColor);
            paragraphPos.y -= lineHeight;
            DrawText(textGL, cmSerif, p2, paragraphPos, paragraphColor);
            paragraphPos.y -= lineHeight;
            DrawText(textGL, cmSerif, p3, paragraphPos, paragraphColor);

            Vec3 titlePos = { 512.0f, 680.0f, 0.0f };
            Vec4 titleColor = { 0.1f, 0.1f, 0.1f, 1.0f };
            Vec2 titleAnchor = { 0.5f, 0.0f };
            const char* title = "OpenGL";
            DrawText(textGL,
                cmSerifBold, title, titlePos, titleAnchor, titleColor);
            int titleWidth = GetTextWidth(cmSerifBold, title);
            Vec2 titleBackgroundSize = { (float)titleWidth, 48.0f };
            Vec4 titleBackgroundColor = { 0.0f, 0.0f, 0.0f, 0.2f };
            DrawRect(rectGL,
                titlePos, titleAnchor, titleBackgroundSize,
                titleBackgroundColor);
            // TODO this is pixelated. Check why.
            // pos: { 512.0f, 680.0f, 0.0f };
            // anchor: { 0.5f, 0.0f }
            // text: "PRINCE"
            // font: cm serif bold, 48
            titlePos.y -= 128.0f;
            DrawText(textGL,
                cmSerifBold128, "HUGE", titlePos, titleAnchor, titleColor);

            Vec3 rect42Pos    = { 400.0f, 50.0f, 0.0f };
            Vec2 rect42Size   = { 800.0f, 300.0f };
            Vec4 rect42Color  = { 1.0f, 0.7f, 0.7f, 0.3f };
            DrawRect(rectGL, rect42Pos, Vec2::zero, rect42Size, rect42Color);
        }

        { // Test GUI
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            mouseY = (double)height_ - mouseY;
            Vec2 mousePos = { (float)mouseX, (float)mouseY };

            UpdateClickableBoxes(boxes.data, boxes.size, mousePos, clickState_);
            UpdateButtons(buttons.data, buttons.size, mousePos, clickState_);
            UpdateInputFields(fields.data, fields.size, mousePos, clickState_,
                keyInputBuffer, keyInputBufferSize);

            DrawClickableBoxes(boxes.data, boxes.size, rectGL);
            DrawButtons(buttons.data, buttons.size, rectGL, textGL, cmSerif);
            DrawInputFields(fields.data, fields.size, rectGL, textGL, cmSerif);
        }

        // Clear all key events
        keyInputBufferSize = 0;

        glfwSwapBuffers(window);
        glfwWaitEvents();
        
        // Catch all GL errors during game loop
        {
            GLenum glError;
            while ((glError = glGetError()) != GL_NO_ERROR) {
                printf("GL ERROR: %x\n", glError);
            }
        }
    }

    glfwTerminate();
    return 0;
}

#include "km_lib.cpp"
#include "ogl_base.cpp"
#include "text.cpp"
#include "load_bmp.cpp"
#include "gui.cpp"
#include "halfedge.cpp"

#include "glew.c"
//#undef internal
//#undef local_persist
//#undef global_var
//#include "lodepng.cpp"