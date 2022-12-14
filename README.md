# Vortex Game Engine [![License](https://img.shields.io/github/license/JShuk-7/Vortex.svg)](https://github.com/JShuk-7/Vortex/blob/master/LICENSE)

![Vortex Logo](Vortex-Editor/Resources/Images/VortexLogo.png)

Vortex is an early-stage interactive application and rendering engine for Windows. Vortex is primarily a 2D engine with potential support for 3D comming in the near future. With that being said, you can still create a 3D looking scene with the features of the engine.

---

## Getting Started

Visual Studio 2022 is recommended, Vortex has not been tested with other development environments.

<ins>**1. Downloading the repository:**<ins>

Start by cloning the repository with the following command: </br>

(NOTE: if the repo was previously cloned non-recursively, you can use 'git submodule update --init' to retrieve the necessary submodules)

```bash
git clone --recursive "https://github.com/Jshuk-7/Vortex"
```

<ins>**2. Building the engine:**</ins>

Since there are currently no released binaries available, you must build the engine from source.

Run the [Win32-GenProjects.bat](https://github.com/JShuk-7/Vortex/blob/master/scripts/Win32-GenProjects.bat) file found in `scripts` folder. This will create a Visual Studio 2022 solution file that you can use to build the engine.

If you make changes to the engine, or if you want to regenerate project files, rerun the [Win32-GenProjects.bat](https://github.com/JShuk-7/Vortex/blob/master/scripts/Win32-GenProjects.bat) file found in the `scripts` folder.

---

## What to Expect in the Future

-   High-fidelity Physically-Based 3D rendering
-   Support for Mac, Linux, Android and IOS
-   Fully featured Vulkan Renderer
-   Integrated 3rd party 3D physics engine
-   Procedural terrain and world generation
-   Artificial Intelligence
-   Tree Graph editor system

---

## Preview
- A simple platformer game
![Platformer game](Vortex-Editor/Resources/Images/Engine/PlatformerGame.png)

- Imported 3D model with skybox
![3D model with skybox](Vortex-Editor/Resources/Images/Engine/3DModel.png)

- Sponza model in full chrome
![Sponza in chrome](Vortex-Editor/Resources/Images/Engine/SponzaChrome.png)

- FPS Demo with Nvidia PhysX
![FPS Demo game](Vortex-Editor/Resources/Images/Engine/FPSDemo.png)