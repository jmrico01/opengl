import sys
import os
import shutil
import platform
import string
import hashlib

def GetScriptPath():
    path = os.path.realpath(__file__)
    lastSep = string.rfind(path, os.sep)
    return path[:lastSep]

def GetLastSlashPath(path):
    """
    Returns the path before the last separating slash in path.
    (The path of the directory containing the given path.
    This is the equivalent of "path/..", but without the "..")
    """
    lastSep = string.rfind(path, os.sep)
    return path[:lastSep]

def NormalizePathSlashes(pathDict):
    for name in pathDict:
        pathDict[name] = pathDict[name].replace("/", os.sep)

# Important directory & file paths
paths = { "root": GetLastSlashPath(GetScriptPath()) }

paths["build"]          = paths["root"] + "/build"
paths["data"]           = paths["root"] + "/data"
paths["external"]       = paths["root"] + "/external"
paths["src"]            = paths["root"] + "/src"

paths["build-data"]     = paths["build"] + "/data"
paths["build-shaders"]  = paths["build"] + "/shaders"
paths["src-shaders"]    = paths["src"] + "/shaders"


paths["main-cpp"] = paths["src"] + "/main.cpp"

paths["include-glew"]       = paths["external"] + "/glew-2.1.0/include"
paths["include-glfw"]       = paths["external"] + "/glfw-3.2.1/include"
paths["include-freetype"]   = paths["external"] + "/freetype-2.8.1/include"
paths["include-lodepng"]    = paths["external"] + "/lodepng/include"

paths["lib-glfw-win-d"] = paths["external"] + "/glfw-3.2.1/lib/win/debug"
paths["lib-glfw-win-r"] = paths["external"] + "/glfw-3.2.1/lib/win/release"
paths["lib-glfw-linux"] = paths["external"] + "/glfw-3.2.1/lib/linux"
paths["lib-ft-win-d"]   = paths["external"] + "/freetype-2.8.1/lib/win/debug"
paths["lib-ft-win-r"]   = paths["external"] + "/freetype-2.8.1/lib/win/release"
paths["lib-ft-linux"]   = paths["external"] + "/freetype-2.8.1/lib/linux"

paths["external-src"]   = paths["external"] + "/src"
paths["external-build"] = paths["external"] + "/build"

paths["src-hashes"]     = paths["build"] + "/src_hashes"
paths["src-hashes-old"] = paths["build"] + "/src_hashes_old"

NormalizePathSlashes(paths)

def WinCompileDebug():
    macros = "/D_CRT_SECURE_NO_WARNINGS"
    compilerFlags = " ".join([
        "/MTd",     # CRT static link (debug)
        "/nologo",  # disables the "Microsoft C/C++ Optimizing Compiler" message
        "/Gm-",     # disable incremental build things
        "/GR-",     # disable type information
        "/EHa-",    # disable exception handling
        "/EHsc",    # handle stdlib errors
        "/Od",      # no optimization
        "/Oi",      # ...except, optimize compiler intrinsics (do I need this?)
        "/Z7"       # minimal "old school" debug information
    ])
    compilerWarningFlags = " ".join([
        "/WX",      # treat warnings as errors
        "/W4",      # level 4 warnings

        # disable the following warnings:
        "/wd4100",  # unused function arguments
        "/wd4189",  # unused initialized local variable
        "/wd4201",  # nonstandard extension used: nameless struct/union
        "/wd4505",  # unreferenced local function has been removed
    ])
    includePaths = " ".join([
        "/I" + paths["include-glew"],
        "/I" + paths["include-glfw"],
        "/I" + paths["include-freetype"],
        "/I" + paths["include-lodepng"]
    ])

    linkerFlags = " ".join([
        "/incremental:no",  # disable incremental linking
        "/opt:ref"          # get rid of extraneous linkages
    ])
    libPaths = " ".join([
        "/LIBPATH:" + paths["lib-glfw-win-d"],
        "/LIBPATH:" + paths["lib-ft-win-d"]
    ])
    libs = " ".join([
        "user32.lib",
        "shell32.lib",
        "gdi32.lib",
        "opengl32.lib",
        "glfw3.lib",
        "freetype281MTd.lib"
    ])

    compileCommand = " ".join([
        "cl",
        macros, compilerFlags, compilerWarningFlags, includePaths,
        "/Feopengl.exe", "/Fmopengl.map", paths["main-cpp"],
        "/link", linkerFlags, libPaths, libs])
    
    loadCompiler = "call \"C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\vcvarsall.bat\" x64"
    os.system(" & ".join([
        "pushd " + paths["build"],
        loadCompiler,
        compileCommand,
        "popd"
    ]))

def WinCompileRelease():
    macros = "/D_CRT_SECURE_NO_WARNINGS"
    compilerFlags = " ".join([
        "/MT",     # CRT static link (release)
        "/nologo",  # disables the "Microsoft C/C++ Optimizing Compiler" message
        "/Gm-",     # disable incremental build things
        "/GR-",     # disable type information
        "/EHa-",    # disable exception handling
        "/EHsc",    # handle stdlib errors
        "/Ox",      # full optimization
    ])
    compilerWarningFlags = " ".join([
        "/WX",      # treat warnings as errors
        "/W4",      # level 4 warnings

        # disable the following warnings:
        "/wd4100",  # unused function arguments
        "/wd4189",  # unused initialized local variable
        "/wd4201",  # nonstandard extension used: nameless struct/union
        "/wd4505",  # unreferenced local function has been removed
    ])
    includePaths = " ".join([
        "/I" + paths["include-glew"],
        "/I" + paths["include-glfw"],
        "/I" + paths["include-freetype"],
        "/I" + paths["include-lodepng"]
    ])

    linkerFlags = " ".join([
        "/incremental:no",  # disable incremental linking
        "/opt:ref"          # get rid of extraneous linkages
    ])
    libPaths = " ".join([
        "/LIBPATH:" + paths["lib-glfw-win-r"],
        "/LIBPATH:" + paths["lib-ft-win-r"]
    ])
    libs = " ".join([
        "user32.lib",
        "shell32.lib",
        "gdi32.lib",
        "opengl32.lib",
        "glfw3.lib",
        "freetype281MT.lib"
    ])

    compileCommand = " ".join([
        "cl",
        macros, compilerFlags, compilerWarningFlags, includePaths,
        "/Feopengl.exe", paths["main-cpp"],
        "/link", linkerFlags, libPaths, libs])
    
    loadCompiler = "call \"C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\vcvarsall.bat\" x64"
    os.system(" & ".join([
        "pushd " + paths["build"],
        loadCompiler,
        compileCommand,
        "popd"
    ]))

def LinuxCompileDebug():
    compilerFlags = " ".join([
        "-std=c++11",       # use C++11 standard
        "-ggdb3",           # generate level 3 (max) GDB debug info.
        "-fno-rtti",        # disable run-time type info
        "-fno-exceptions"   # disable C++ exceptions (ew)
    ])
    compilerWarningFlags = " ".join([
        "-Werror",  # treat warnings as errors
        "-Wall",    # enable all warnings

        # disable the following warnings:
        "-Wno-char-subscripts" # using char as an array subscript
    ])
    includePaths = " ".join([
        "-I" + paths["include-glew"],
        "-I" + paths["include-glfw"],
        "-I" + paths["include-freetype"],
        "-I" + paths["include-lodepng"]
    ])

    libPaths = " ".join([
        "-L" + paths["lib-glfw-linux"],
        "-L" + paths["lib-ft-linux"]
    ])
    libs = " ".join([
        # main external libs
        "-lglfw3",
        "-lfreetype",

        # GLFW3 dependencies
        "-lGL",
        "-lm",
        "-lXrandr",
        "-lXi",
        "-lX11",
        "-lXxf86vm",
        "-lXcursor",
        "-lXinerama",
        "-lpthread",
        "-ldl",

        # FreeType dependencies
        "-lz",
        "-lpng"
    ])

    compileCommand = " ".join([
        "g++",
        compilerFlags, compilerWarningFlags, includePaths,
        paths["main-cpp"], "-o opengl",
        libPaths, libs
    ])

    os.system("bash -c \"" + " ; ".join([
        "pushd " + paths["build"] + " > /dev/null",
        compileCommand,
        "popd > /dev/null"
    ]) + "\"")

def MacCompileDebug():
    compilerFlags = " ".join([
        "-std=c++11",       # use C++11 standard
        "-ggdb3",           # generate level 3 (max) GDB debug info.
        "-fno-rtti",        # disable run-time type info
        "-fno-exceptions"   # disable C++ exceptions (ew)
    ])
    compilerWarningFlags = " ".join([
        "-Werror",  # treat warnings as errors
        "-Wall",    # enable all warnings

        # disable the following warnings:
        "-Wno-missing-braces"  # Not sure why this complains
    ])
    includePaths = " ".join([
        "-I" + paths["include-glew"],
        "-I" + paths["include-glfw"],
        "-I" + paths["include-freetype"],
        "-I" + paths["include-lodepng"]
    ])

    libPaths = " ".join([
        "-L" + paths["lib-glfw-linux"],
        "-L" + paths["lib-ft-linux"]
    ])
    libs = " ".join([
        # main external libs
        "-lglfw3",
        "-lfreetype",

        # GLFW3 dependencies
        "-framework OpenGL",

        # FreeType dependencies
        "-lz",
        "-lpng"
    ])

    compileCommand = " ".join([
        "gcc",
        compilerFlags, compilerWarningFlags, includePaths,
        paths["main-cpp"], "-o opengl",
        libPaths, libs
    ])

    os.system("bash -c \"" + " ; ".join([
        "pushd " + paths["build"] + " > /dev/null",
        compileCommand,
        "popd > /dev/null"
    ]) + "\"")

def FileMD5(filePath):
    md5 = hashlib.md5()
    with open(filePath, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            md5.update(chunk)
    
    return md5.hexdigest()

def ComputeSrcHashes():
    with open(paths["src-hashes"], "w") as out:
        for root, _, files in os.walk(paths["src"]):
            for fileName in files:
                filePath = os.path.join(root, fileName)
                out.write(filePath + "\n")
                out.write(FileMD5(filePath) + "\n")

def CopyDir(srcPath, dstPath):
    # Re-create (clear) the directory
    if os.path.exists(dstPath):
        shutil.rmtree(dstPath)
    os.makedirs(dstPath)

    # Copy
    for fileName in os.listdir(srcPath):
        filePath = os.path.join(srcPath, fileName)
        if os.path.isfile(filePath):
            shutil.copy2(filePath, dstPath)
        elif os.path.isdir(filePath):
            shutil.copytree(filePath, dstPath + os.sep + fileName)

def Debug():
    ComputeSrcHashes()
    CopyDir(paths["data"], paths["build-data"])
    CopyDir(paths["src-shaders"], paths["build-shaders"])

    platformName = platform.system()
    if platformName == "Windows":
        WinCompileDebug()
    elif platformName == "Linux":
        LinuxCompileDebug()
    else:
        LinuxCompileDebug()
        print "Unsupported platform: " + platformName
        

def IfChanged():
    hashPath = paths["src-hashes"]
    oldHashPath = paths["src-hashes-old"]

    changed = False
    if os.path.exists(hashPath):
        if os.path.exists(oldHashPath):
            os.remove(oldHashPath)
        os.rename(hashPath, oldHashPath)
    else:
        changed = True

    ComputeSrcHashes()
    if not changed:
        if os.path.getsize(hashPath) != os.path.getsize(oldHashPath) \
        or open(hashPath, "r").read() != open(oldHashPath, "r").read():
            changed = True
    
    if changed:
        Debug()
    else:
        print "No changes. Nothing to compile."

def Release():
    CopyDir(paths["data"], paths["build-data"])
    CopyDir(paths["src-shaders"], paths["build-shaders"])

    platformName = platform.system()
    if platformName == "Windows":
        WinCompileRelease()
    elif platformName == "Linux":
        print "Release: UNIMPLEMENTED"
    else:
        print "Release: UNIMPLEMENTED"

def External():
    if not os.path.exists(paths["external-build"]):
        os.makedirs(paths["external-build"])
    
    for fileName in os.listdir(paths["external-src"]):
        filePath = os.path.join(paths["external-src"], fileName)
        print filePath

def Clean():
    for fileName in os.listdir(paths["build"]):
        filePath = os.path.join(paths["build"], fileName)
        try:
            if os.path.isfile(filePath):
                os.remove(filePath)
            elif os.path.isdir(filePath):
                shutil.rmtree(filePath)
        except Exception as e:
            # Handles file-in-use kinds of things.
            # ... exceptions are so ugly.
            print e

def Main():
    if not os.path.exists(paths["build"]):
        os.makedirs(paths["build"])

    arg1 = ""
    if len(sys.argv) > 1:
        arg1 = sys.argv[1]

    if arg1 == "" or arg1 == "debug":
        Debug()
    elif arg1 == "ifchanged":
        IfChanged()
    elif arg1 == "release":
        Release()
    elif arg1 == "external":
        External()
    elif arg1 == "clean":
        Clean()

Main()
