cmake_minimum_required(VERSION 3.23)

add_library(ZSharp)

target_sources(ZSharp
    PRIVATE
        Visualizer.natvis

        AABB.h
        Array.h
        Asset.h
        Bundle.h
        BundleGeneration.h
        Camera.h
        CommandLineParser.h
        Common.h
        CommonMath.h
        ConsoleVariable.h
        Constants.h
        DebugText.h
        Delegate.h
        DepthBuffer.h
        DevConsole.h
        FileString.h
        FixedArray.h
        Framebuffer.h
        GameInstance.h
        Graph.h
        HashFunctions.h
        HashTable.h
        Heap.h
        IndexBuffer.h
        IniFile.h
        InputManager.h
        ISerializable.h
        JPEG.h
        List.h
        Logger.h
        Mat2x3.h
        Mat3x3.h
        Mat4x4.h
        Mesh.h
        Model.h
        MP3.h
        OBJFile.h
        Pair.h
        PhysicsAlgorithms.h
        PhysicsObject.h
        PlatformAudio.h
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
        TexturePool.h
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
	
        AABB.cpp
        Asset.cpp
        Bundle.cpp
        BundleGeneration.cpp
        Camera.cpp
        CommandLineParser.cpp
        Common.cpp
        CommonMath.cpp
        ConsoleVariable.cpp
        DebugText.cpp
        DepthBuffer.cpp
        DevConsole.cpp
        FileString.cpp
        Framebuffer.cpp
        GameInstance.cpp
        HashFunctions.cpp
        IndexBuffer.cpp
        IniFile.cpp
        InputManager.cpp
        JPEG.cpp
        Logger.cpp
        Mat2x3.cpp
        Mat3x3.cpp
        Mat4x4.cpp
        Mesh.cpp
        Model.cpp
        MP3.cpp
        OBJFile.cpp
        PhysicsAlgorithms.cpp
        PhysicsObject.cpp
        PNG.cpp
        Quaternion.cpp
        Random.cpp
        Renderer.cpp
        Serializer.cpp
        ScopedTimer.cpp
        Texture.cpp
        TexturePool.cpp
        Triangle.cpp
        Vec3.cpp
        Vec4.cpp
        VertexBuffer.cpp
        Win32PlatformApplication.cpp
        Win32PlatformAudio.cpp
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

set(ZSharp_Compile_Options_Debug )

set(ZSharp_Compile_Options_Release )

set(ZSharp_Preprocessor_Defines HW_PLATFORM_X86)

if(WIN32)
  list(APPEND ZSharp_Preprocessor_Defines 
    PLATFORM_WINDOWS
    _CRT_SECURE_NO_WARNINGS
	_HAS_EXCEPTIONS=0
    WIN32_LEAN_AND_MEAN
    PLATFORM_MAX_PATH=260
  )

  list(APPEND ZSharp_Compile_Options_Debug
    /Od
    /Ob0
    /source-charset:utf-8
    /std:c++latest
    /D_UNICODE
    /GR-
    /MP
    /U_MBCS
    /W4
    /WX
    /Zi
    /diagnostics:caret
  )

  list(APPEND ZSharp_Compile_Options_Release
    /O2
    /Ob2
    /source-charset:utf-8
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
    /Zi
    /diagnostics:caret
  )
endif(WIN32)

target_compile_definitions(ZSharp PRIVATE 
  ${ZSharp_Preprocessor_Defines}
)

if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_compile_options(ZSharp PRIVATE 
			${ZSharp_Compile_Options_Debug})

	elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		target_compile_options(ZSharp PRIVATE 
			${ZSharp_Compile_Options_Release})

	else()
		message(FATAL_ERROR "Must specify -DCMAKE_BUILD_TYPE=Debug/Release when using Ninja.")
	endif()

else()
    target_compile_options(ZSharp PRIVATE 
	    "$<$<CONFIG:DEBUG>:${ZSharp_Compile_Options_Debug}>"
    )

    target_compile_options(ZSharp PRIVATE 
	    "$<$<CONFIG:RELEASE>:${ZSharp_Compile_Options_Release}>"
    )
endif()
