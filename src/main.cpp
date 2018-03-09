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

// TODO (on linux at least) the window's top bar is part
// of the viewport, so the render scene is cropped.

// TODO get rid of this... getting tired of writing that
global_var char pathToApp_[PATH_MAX_LENGTH];
global_var int width_ = 1024;
global_var int height_ = 768;
global_var int clickState_ = CLICKSTATE_NONE;

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

internal void KeyCallback(GLFWwindow* window,
    int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE) {
        if (action == GLFW_PRESS) {
            // TODO exit
        }
    }

    if (action == GLFW_PRESS) {
        printf("pressed:  %d\n", key);
    }
    else if (action == GLFW_RELEASE) {
        printf("released: %d\n", key);
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
        width_, height_, "PRINCE", NULL, NULL);
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
        printf("PRINCE requires OpenGL 3.3 or later.\n");
        printf("If your hardware supports it, update your drivers.\n");
        return 1;
    }
    printf("GLSL version:   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GL renderer:    %s\n", glGetString(GL_RENDERER));
    
    // Initialize app-specific OpenGL.
    // TODO error checking?
    InitOpenGL();
    ResizeGL(width_, height_);
    // NOTE this isn't like "create a rect instance", but more like
    // "initialize rect drawing in general"
    RectGL rectGL = CreateRectGL();
    TexturedRectGL texturedRectGL = CreateTexturedRectGL();
    TextGL textGL = CreateTextGL();
    
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        printf("FreeType init error: %d\n", error);
        return 1;
    }

    GLuint textureKM = OpenGLLoadBMP("data/images/kapricorn.bmp");
    FontFace cmSerif = LoadFontFace(
        library, "data/fonts/computer-modern/serif.ttf", 32);
    FontFace cmSerifBold = LoadFontFace(
        library, "data/fonts/computer-modern/serif-bold.ttf", 48);
    FontFace cmSerifBold128 = LoadFontFace(
        library, "data/fonts/computer-modern/serif-bold.ttf", 128);

    ClickableBox boxes[1];
    InputField fields[1];
    {
        Vec2 guiBoxOrigin = { 100.0f, 100.0f };
        Vec2 guiBoxSize = { 100.0f, 80.0f };
        boxes[0] = CreateClickableBox(guiBoxOrigin, guiBoxSize);
        Vec2 fieldOrigin = { 250.0f, 100.0f };
        Vec2 fieldSize = { 400.0f, 32.0f };
        fields[0] = CreateInputField(fieldOrigin, fieldSize);
    }

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    // Set up input handling.
    glfwSetKeyCallback(window, &KeyCallback);
    glfwSetMouseButtonCallback(window, &MouseCallback);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        { // Test draws (primitives & text)
            Vec3 kmPos = { 100.0f, 100.0f, 0.0f };
            Vec2 kmSize = { 400.0f, 400.0f };
            DrawTexturedRect(texturedRectGL,
                kmPos, Vec2::zero, kmSize, textureKM);

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
            const char* title = "PRINCE";
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

            UpdateClickableBoxes(boxes, 1, mousePos, clickState_);
            UpdateInputFields(fields, 1, mousePos, clickState_);
            DrawClickableBoxes(boxes, 1, rectGL);
            DrawInputFields(fields, 1, rectGL, textGL, cmSerif);
        }

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwTerminate();
    return 0;
}

#include "ogl_base.cpp"
#include "text.cpp"
#include "load_bmp.cpp"
#include "gui.cpp"

#include "glew.c"
//#undef internal
//#undef local_persist
//#undef global_var
//#include "lodepng.cpp"