cmake_minimum_required(VERSION 3.23)

add_library(ZSharp)

target_sources(ZSharp
    PRIVATE
        Visualizer.natvis

        Array.h
        Asset.h
        Bundle.h
        Camera.h
        Common.h
        CommonMath.h
        Constants.h
        Delegate.h
        FileString.h
        FixedArray.h
        Framebuffer.h
        GameInstance.h
        GlobalEdgeTable.h
        HashFunctions.h
        HashTable.h
        IndexBuffer.h
        IniFile.h
        InputManager.h
        ISerializable.h
        List.h
        Logger.h
        Mat2x3.h
        Mat3x3.h
        Mat4x4.h
        Mesh.h
        Model.h
        OBJFile.h
        OBJLoader.h
        Pair.h
        PlatformDebug.h
        PlatformDefines.h
        PlatformFile.h
        PlatformHAL.h
        PlatformIntrinsics.h
        PlatformMisc.h
        PlatformMemory.h
        PlatformTime.h
        PNG.h
        Quaternion.h
        Random.h
        Renderer.h
        ScopedTimer.h
        Serializer.h
        ShadingMode.h
        Stack.h
        Texture.h
        Tree.h
        Triangle.h
        Vec3.h
        Vec4.h
        VertexBuffer.h
        Win32PlatformHeaders.h
        Win32PlatformApplication.h
        World.h
        WorldObject.h
        ZAlgorithm.h
        ZAssert.h
        ZBaseTypes.h
        ZColor.h
        ZConfig.h
        ZDrawing.h
        ZFile.h
        ZString.h
	
        Asset.cpp
        Bundle.cpp
        Camera.cpp
        Common.cpp
        CommonMath.cpp
        FileString.cpp
        Framebuffer.cpp
        GameInstance.cpp
        GlobalEdgeTable.cpp
        HashFunctions.cpp
        IndexBuffer.cpp
        IniFile.cpp
        InputManager.cpp
        Logger.cpp
        Mat2x3.cpp
        Mat3x3.cpp
        Mat4x4.cpp
        Mesh.cpp
        Model.cpp
        OBJFile.cpp
        OBJLoader.cpp
        PNG.cpp
        Quaternion.cpp
        Random.cpp
        Renderer.cpp
        Serializer.cpp
        ScopedTimer.cpp
        Texture.cpp
        Triangle.cpp
        Vec3.cpp
        Vec4.cpp
        VertexBuffer.cpp
        Win32PlatformApplication.cpp
        Win32PlatformDebug.cpp
        Win32PlatformFile.cpp
        Win32PlatformHAL.cpp
        Win32PlatformIntrinsics.cpp
        Win32PlatformMisc.cpp
        Win32PlatformMemory.cpp
        Win32PlatformTime.cpp
        World.cpp
        WorldObject.cpp
        ZAlgorithm.cpp
        ZColor.cpp
        ZConfig.cpp
        ZDrawing.cpp
        ZFile.cpp
        ZString.cpp
)

set(ZSharp_Compile_Options_Debug 
  /source-charset:utf-8
  /std:c17
  /std:c++latest
  /D_UNICODE
  /GR-
  /MP
  /U_MBCS
  /W4
  /WX
)

set(ZSharp_Compile_Options_Release 
  /source-charset:utf-8
  /std:c17
  /std:c++latest
  /D_UNICODE
  /GR-
  /MP
  /U_MBCS
  /W4
  /WX
  /sdl-
  /GL
  /GS-
)

set(ZSharp_Preprocessor_Defines )

if(WIN32)
  list(APPEND ZSharp_Preprocessor_Defines 
    PLATFORM_WINDOWS
    _CRT_SECURE_NO_WARNINGS
	_HAS_EXCEPTIONS=0
    WIN32_LEAN_AND_MEAN
  )
endif(WIN32)

list(APPEND ZSharp_Preprocessor_Defines 
  HW_PLATFORM_X86
)

target_compile_definitions(ZSharp PRIVATE 
  ${ZSharp_Preprocessor_Defines}
)

target_compile_options(ZSharp PRIVATE 
	"$<$<CONFIG:DEBUG>:${ZSharp_Compile_Options_Debug}>"
)

target_compile_options(ZSharp PRIVATE 
	"$<$<CONFIG:RELEASE>:${ZSharp_Compile_Options_Release}>"
)
