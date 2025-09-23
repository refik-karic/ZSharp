cmake_minimum_required(VERSION 3.23)

project(ZSharp VERSION 1.0)

add_library(ZSharp)

set(ZSharp_Header_Files 
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
    FrontEnd.h
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
    MoveHelpers.h
    MP3.h
    OBJFile.h
    Pair.h
    PhysicsAlgorithms.h
    PhysicsObject.h
    PlatformAtomic.h
    PlatformAudio.h
    PlatformApplication.h
    PlatformDebug.h
    PlatformDefines.h
    PlatformFile.h
    PlatformHAL.h
    PlatformIntrinsics.h
    PlatformMisc.h
    PlatformMemory.h
    PlatformTime.h
    PlatformThread.h
    PNG.h
    Quaternion.h
    Random.h
    Renderer.h
    ScopedTimer.h
    Serializer.h
    ShaderDefinition.h
    Span.h
    Stack.h
    Texture.h
    TexturePool.h
    ThreadPool.h
    Tree.h
    Triangle.h
    Trie.h
    UIBase.h
    UIButton.h
    UIContainer.h
    UIElement.h
    UIFrame.h
    UIGrid.h
    UIImage.h
    UILabel.h
    UILinearPanel.h
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
)

set(ZSharp_Source_Files 
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
    FrontEnd.cpp
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
    ShaderDefinition.cpp
    ScopedTimer.cpp
    Texture.cpp
    TexturePool.cpp
    ThreadPool.cpp
    Triangle.cpp
    UIBase.cpp
    UIButton.cpp
    UIContainer.cpp
    UIElement.cpp
    UIFrame.cpp
    UIGrid.cpp
    UIImage.cpp
    UILabel.cpp
    UILinearPanel.cpp
    Vec3.cpp
    Vec4.cpp
    VertexBuffer.cpp
    Win32PlatformApplication.cpp
    Win32PlatformAtomic.cpp
    Win32PlatformAudio.cpp
    Win32PlatformDebug.cpp
    Win32PlatformFile.cpp
    Win32PlatformHAL.cpp
    Win32PlatformIntrinsics.cpp
    Win32PlatformMisc.cpp
    Win32PlatformMemory.cpp
    Win32PlatformTime.cpp
    Win32PlatformThread.cpp
    World.cpp
    WorldObject.cpp
    ZAlgorithm.cpp
    ZColor.cpp
    ZConfig.cpp
    ZDrawing.cpp
    ZFile.cpp
    ZString.cpp
)

target_sources(ZSharp
    PRIVATE
        Visualizer.natvis
        ${ZSharp_Header_Files}
        ${ZSharp_Source_Files}
)

set(ZSharp_Compile_Options_Debug )

set(ZSharp_Compile_Options_MinSizeRel )

set(ZSharp_Compile_Options_RelWithDebInfo )

set(ZSharp_Compile_Options_Release )

set(ZSharp_Preprocessor_Defines HW_PLATFORM_X86)

message("Generating with compiler ${CMAKE_CXX_COMPILER_ID}")

if(WIN32)
  list(APPEND ZSharp_Preprocessor_Defines 
    PLATFORM_WINDOWS
    _CRT_SECURE_NO_WARNINGS
	_HAS_EXCEPTIONS=0
    WIN32_LEAN_AND_MEAN
    PLATFORM_MAX_PATH=260
    BUILD_TYPE="${CMAKE_BUILD_TYPE}"
  )

  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    list(APPEND ZSharp_Compile_Options_Debug
        /Od
        /Ob0
        /Oi
        /source-charset:utf-8
        /std:c++latest
        /D_UNICODE
        /GR-
        /U_MBCS
        /W4
        /WX
        /Zi
        /diagnostics:caret
        -Wno-sign-compare
        -Wno-deprecated-copy-with-user-provided-copy
        -mavx2
        -mfma
    )

    list(APPEND ZSharp_Compile_Options_MinSizeRel
        /O1
        /Ob1
        /source-charset:utf-8
        /std:c++latest
        /D_UNICODE
        /GR-
        /U_MBCS
        /W4
        /WX
        /Zi
        /diagnostics:caret
        -Wno-sign-compare
        -Wno-deprecated-copy-with-user-provided-copy
        -mavx2
        -mfma
    )

    list(APPEND ZSharp_Compile_Options_RelWithDebInfo
        /O2
        /Ob1
        /source-charset:utf-8
        /std:c++latest
        /D_UNICODE
        /GR-
        /U_MBCS
        /W4
        /WX
        /Zi
        /diagnostics:caret
        -Wno-sign-compare
        -Wno-deprecated-copy-with-user-provided-copy
        -mavx2
        -mfma
    )

    list(APPEND ZSharp_Compile_Options_Release
        /Os
        /Ob2
        /source-charset:utf-8
        /std:c++latest
        /D_UNICODE
        /GR-
        /U_MBCS
        /W4
        /WX
        /sdl-
        /GS-
        /Zi
        /diagnostics:caret
        /Gw
		/Gy
        -flto
        -fvirtual-function-elimination
        -Wno-sign-compare
        -Wno-deprecated-copy-with-user-provided-copy
        -mavx2
        -mfma
    )
  else()
    list(APPEND ZSharp_Compile_Options_Debug
        /Od
        /Ob0
        /Oi
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

    list(APPEND ZSharp_Compile_Options_MinSizeRel
        /O1
        /Ob1
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

    list(APPEND ZSharp_Compile_Options_RelWithDebInfo
        /O2
        /Ob1
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
        /Os
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
		/Gw
		/Gy
        /diagnostics:caret
    )
  endif()

  target_compile_definitions(ZSharp PRIVATE 
    ${ZSharp_Preprocessor_Defines})

endif(WIN32)

if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_compile_options(ZSharp PRIVATE 
			${ZSharp_Compile_Options_Debug})

    elseif("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
		target_compile_options(ZSharp PRIVATE 
			${ZSharp_Compile_Options_MinSizeRel})

    elseif("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
		target_compile_options(ZSharp PRIVATE 
			${ZSharp_Compile_Options_RelWithDebInfo})

	elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		target_compile_options(ZSharp PRIVATE 
			${ZSharp_Compile_Options_Release})

	else()
		message(FATAL_ERROR "Must specify -DCMAKE_BUILD_TYPE=Debug|MinSizeRel|RelWithDebInfo|Release when using Ninja.")
	endif()

    source_group("Headers" FILES ${ZSharp_Header_Files})
	source_group("Source" FILES ${ZSharp_Source_Files})

else()

    target_compile_options(ZSharp PRIVATE 
	    "$<$<CONFIG:DEBUG>:${ZSharp_Compile_Options_Debug}>")

    target_compile_options(ZSharp PRIVATE 
	    "$<$<CONFIG:MINSIZEREL>:${ZSharp_Compile_Options_MinSizeRel}>")

    target_compile_options(ZSharp PRIVATE 
	    "$<$<CONFIG:RELWITHDEBINFO>:${ZSharp_Compile_Options_RelWithDebInfo}>")

    target_compile_options(ZSharp PRIVATE 
	    "$<$<CONFIG:RELEASE>:${ZSharp_Compile_Options_Release}>")

endif()
