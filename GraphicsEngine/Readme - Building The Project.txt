Prerequisites:
CMake Version 3.12.1 or better (if not included in Engine/CmakeLocal folder)
Visual Studio 15 or 17
(VS17 15.8 or better is required for experimental unity support)

Building the Project:
1. Run EngineBuildTool.exe located in the root directory of the engine.
2. Open EngineSolution.sln
3. Ensure all projects are built before first run.

Note: Module projects will not build when running if the "Only build Startup Projects and dependencies on Run" is enabled.

To Disable Unity build run the BuildTool with -nounity option