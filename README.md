# Realistic voxel scene rendering in real time
## Compilation
This program is targeted for `g++-11` so I recommend using that compiler. It may work with `g++-10` also. 

CMake downloads most of the necessary dependencies, except for the following:
* Vulkan:
```
    wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
    sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-focal.list https://packages.lunarg.com/vulkan/lunarg-vulkan-focal.list
    sudo apt update
    sudo apt install vulkan-sdk
```
* GLFW3: package `libglfw3-dev`

The compilation is done via cmake:
```
cd project_path # change directory to project root
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=release -DCMAKE_CXX_COMPILER="path_to_g++-11"
cmake --build build --target all
```
        
## Usage
First you need to prepare a configuration file which should look like this:
```toml
[rendering.compute]
local_size_x = 8
local_size_y = 8

[resources]
path_models = "path_to_project/assets/vox"
path_shaders = "path_to_project/src/shaders"

[ui.imgui]
path_icons = "path_to_project/assets/icons"

[ui.window]
height = 1200
width = 800
```

You need to replace `path_to_project` with your local path to the project.

The program accepts the following arguments:
```
-h --help       show this help message and exit
--scene_edit    Run scene editing renderer
-v --verbose    Verbose logging
-l --log        Enable console logging.
-d --debug      Enable debug logging.
--log_dir       Custom directory for log files.
--config        Custom TOML config file.
```

Most likely you want to start the project like this:
`realistic_voxel_rendering --config "your path to config"`

or

`realistic_voxel_rendering --scene_edit --config "your path to config"`

Alternatively you may just place the config file named as `config.toml` to the same directory as the binary, and it'll find it without the command line argument.

Most likely you want to first run the program in the `scene_edit` mode in order to prepare a scene, save its configuration using main menu -> `File` -> `Save scene`.
Then start the program without the `scene_edit` argument and load the scene using main menu -> `File` -> `Load scene`. Next render the 
## Controls
* Camera movement in space:
    * W - move forward
    * A - move left
    * S - move backward
    * D - move right
    * Q - move down
    * E - move up
* Camera direction:
    * Hold right mouse button outside any UI elements, this'll allow you to control camera direction.
    
### UI elements
Most of the elements contain tooltips, so feel free to hover over them if you're unsure of what they do.

### Main panel
* File - scene/model loading from filesystem
* View - show/hide windows
* Tools - windows for file format conversion

### Render settings
Controls for light position and phong lighting parameters.

### Info
Window showing FPS information, and some basic info on how long each part of the rendering loop takes. It also contains camera/movement controls and information about currently rendered scene.

### Shader controls
Controls to visualize probes/BVH nodes.

### Probe grid controls
Elements to setup light field probe grid. Enable LF visualisation in `Shader controls` window before using this one.

### Debug images
Window showing how many iterations were needed to calculate intersection for given pixel using sparse voxel octree ray marching

### Debug
A window for debugging controls, you can just close it if it pops up.

### Models
A window containing list of models found in `model_path` provided in the config file on the left. You can use drag and drop, or the arrow button to copy them to the right
listbox, which will load the model. Upon clicking on any model on the right the `Detail` part of the window will show you information about this model, and it will also
allow you to control its transformation matrix.

### Probes info
Visualisation of probe atlas. You need to click on the `Render probes in the next pass` button for the probes to prepare themselves. The texture rendered in the window allows you
to either visualize all the data stored within each probe, or it can show you how the scene looks if rendered via probe ray tracing.