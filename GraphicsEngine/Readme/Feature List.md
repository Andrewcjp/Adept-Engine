# Feature List

Flexible RHI (Render Hardware Interface) supporting:
* 	DirectX 12 
* 	Vulkan (WIP)

Feature Support table:

Feature| DirectX 12 | Vulkan
---------|----------|--------
 Render pipeline support  | Complete       | Basic
 Render Graph system      | Complete| Untested
 Raytracing  | Basic | Planned
 Linked Multi-GPU| Planned | Planned
 Unlinked Multi-GPU | Complete | None
VR Support | Complete | Planned


Rendering Features:

* Render Graph system for flexible renderer design
* PBR Lighting Pipeline
* Shadow mapping (static and dynamic)
* GPU Particle System (WIP)
* UI system (WIP)
* Post Processing Stack (WIP)
* Raytracing (DXR) Support (WIP)
* Light Culling (WIP)
* VR Support (SteamVR) (WIP)
* Animation system (WIP)

Multi-GPU Features:

* Multi-GPU Shadow mapping
* Asynchronous Multi-GPU Shadow Mapping
* Split frame rendering (SFR)
* SFR with Multi-GPU Shadow mapping

Engine Features:

* Custom Build Tool
* Custom header tool (WIP)
* Module System
* Platform Abstraction Layer supporting:
    * Windows
    * Linux (Planned)
    

Physics Features:

* Physx 3.X Support
* Custom TD Physics Engine

AI Features:

* D* Lite for path finding
* Navigation mesh generation 
* Asynchronous path finding request system(WIP)
* Behavior tree system