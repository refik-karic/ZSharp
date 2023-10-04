# ZSharp
The core implementation of the 3D Software Renderer.

## High level features
* General Data Structures
  * Resizable Array, HashMap, Graph, Stack, List, Pair, Mutable Short/Wide String, Red/Black Tree, Min/Max Heap
* 3D Data Structures
  * Vec3/4, Mat2x3/3x3/4x4, Quaternion, AABB
* Platform Agnostic APIs
  * Buffered and Memory Mapped I/O
  * Aligned memory allocation
  * Capability querying (resolution, HW specs)
  * Timing
  * Dynamic dispatch SIMD
* Rendering interface
  * Frame buffer
  * Depth buffer
  * Camera
  * Vertex/Index buffers
  * World
* Programmer creature comforts
  * Scoped timers
  * Debug visualizers
  * INI files
  * Asserts
* Asset parsing and usage
  * OBJ, PNG, MP3, INI
