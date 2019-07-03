# Adept Engine

A flexible C++ Game engine focusing on multi-GPU rendering 

This is currently VERY work in progress

[Roadmap](https://trello.com/b/tiTV3Fbs/adept-engine-roadmap)

[Dev Blog](https://andrewcjp.wordpress.com/)


Flexible RHI (Render Hardware Interface) supporting:

* 	DirectX 12 
* 	Vulkan (WIP)

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
* Render node system for scalable multi-GPU rendering

Multi-GPU Features:

* Multi-GPU Shadow mapping
* Asynchronous Multi-GPU Shadow Mapping
* Split frame rendering (SFR)
* SFR with Multi-GPU Shadow mapping

Engine Features:

* Custom Build Tool 
* Custom header tool (WIP)
* Module System
* Platform Abstraction Layer.

Physics Features:

* Physx Support
* Custom TD Physics Engine

AI Features:

* D* Lite for path finding
* Navigation mesh generation 
* Asynchronous path finding request system(WIP)
* Behavior tree system