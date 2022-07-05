add_library(ZSharp)

target_sources(ZSharp
    PRIVATE
        Array.h
        AssetLoader.h
        Camera.h
        Common.h
        Constants.h
	    FileString.h
	    FixedArray.h
        Framebuffer.h
        GlobalEdgeTable.h
        IInputListener.h
        IndexBuffer.h
        InputManager.h
        IntelIntrinsics.h
        ISerializable.h
        Mat2x3.h
        Mat3x3.h
        Mat4x4.h
        Mesh.h
        Model.h
        Quaternion.h
        Renderer.h
        Triangle.h
        UtilMath.h
        Vec3.h
        Vec4.h
        VertexBuffer.h
        World.h
        ZAlgorithm.h
        ZAssert.h
        ZBaseTypes.h
        ZColor.h
        ZConfig.h
        ZDrawing.h
        ZString.h
        OBJFile.h
	
        AssetLoader.cpp
        Camera.cpp
	    FileString.cpp
        Framebuffer.cpp
        GlobalEdgeTable.cpp
        IndexBuffer.cpp
        InputManager.cpp
        IntelIntrinsics.cpp
        Mat2x3.cpp
        Mat3x3.cpp
        Mat4x4.cpp
        Mesh.cpp
        Model.cpp
        OBJFile.cpp
        Quaternion.cpp
        Renderer.cpp
        Triangle.cpp
        UtilMath.cpp
        Vec3.cpp
        Vec4.cpp
        VertexBuffer.cpp
        World.cpp
        ZAlgorithm.cpp
        ZColor.cpp
        ZConfig.cpp
        ZDrawing.cpp
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

target_compile_definitions(ZSharp PRIVATE 
  FORCE_AVX512
  FORCE_SSE
)

target_compile_options(ZSharp PRIVATE 
	"$<$<CONFIG:DEBUG>:${ZSharp_Compile_Options_Debug}>"
)

target_compile_options(ZSharp PRIVATE 
	"$<$<CONFIG:RELEASE>:${ZSharp_Compile_Options_Release}>"
)
