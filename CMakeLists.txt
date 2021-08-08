add_library(ZSharp)

set(ASSETS
	db_schema.json
	pyramid.json
	pyramids.json
	triangles.json
	ZSharpInit.ini
)

set(ASSET_PATHS
	${CMAKE_SOURCE_DIR}/ZSharp/db_schema.json
	${CMAKE_SOURCE_DIR}/ZSharp/pyramid.json
	${CMAKE_SOURCE_DIR}/ZSharp/pyramids.json
	${CMAKE_SOURCE_DIR}/ZSharp/triangles.json
	${CMAKE_SOURCE_DIR}/ZSharp/ZSharpInit.ini
)

set_property(
	TARGET ZSharp
	PROPERTY ZSHARP_ASSETS ${ASSET_PATHS}
)

target_sources(ZSharp
    PRIVATE
        AssetLoader.h
        Camera.h
        Common.h
        Constants.h
        FileUtilities.h
        Framebuffer.h
        GlobalEdgeTable.h
        IInputListener.h
        IndexBuffer.h
        InputManager.h
        IntelIntrinsics.h
        JsonObject.h
        JsonScanner.h
        Mat2x3.h
        Mat4x4.h
        Mesh.h
        Model.h
        Renderer.h
        Scanner.h
        Token.h
        Triangle.h
        UtilMath.h
        Vec3.h
        Vec4.h
        VertexBuffer.h
        ZAlgorithm.h
        ZColor.h
        ZConfig.h
        ZDrawing.h
	
        AssetLoader.cpp
        FileUtilities.cpp
        Framebuffer.cpp
        GlobalEdgeTable.cpp
        IndexBuffer.cpp
        InputManager.cpp
        IntelIntrinsics.cpp
        JsonObject.cpp
	      JsonScanner.cpp
        Renderer.cpp
        Scanner.cpp
	      Token.cpp
        UtilMath.cpp
        ZColor.cpp
        ZConfig.cpp
        ZDrawing.cpp
		
	${JSON_ASSETS}
)

set(ZSharp_Compile_Options 
  /favor:INTEL64
  /fp:fast
	/guard:cf-
  /permissive-
	/sdl-
  /source-charset:utf-8
	/std:c++latest
	/D_UNICODE
  /Gw
  /Gy
  /GL
	/GR-
	/GS-
	/MP
	/U_MBCS
  /W4
	/WX
)

target_compile_options(ZSharp PRIVATE 
	${ZSharp_Compile_Options}
)
