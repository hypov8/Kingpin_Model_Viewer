# Kingpin Model Viewer

bassed on quake2 .md2 and kingpin .mdx model viewers. 
combined into one program. so kingpin supported files can be viewed in a single program.

original source from
http://www.milkshape3d.com/ms3d/


#version 1.1.4
- added vertex viewer to assist adding sprites
- fixed pause animation. loading a new model would still use old frames
- additional model with less animations are now always drawn on last frame (red wireframe)
- fixed some texture searching issues
- added scrollwheel to zoom view (buggy. looses focus)

#version 1.1.5
- fixed missing skins causing a crash
- added searching for player skins eg.. head.mdx searches for head_001.tga

#version 1.1.6
- fixed light normals
- added option. show vertex normals
- added option to show mdx hitbox
- added menu reload textures
- added option to show grid
- added custom color to new dev items
- camera rotated. this shows player front view
- fixed missing file extension on export
- fixed lerp counter going negative (DWORD)
- updated to latest mxtk from github (+added kingpin file support)
- background image can be removed with setting color
- loading 6 models with startup string
- added startup switch. 
    `-0`=stat wireframe
    `-1`=start flat shade
    `-2`=smooth shaded
    `-3`=start textured




![UI1](/pics/kp_viewer_01.png)
![UI1](/pics/kp_viewer_02.png)
![UI1](/pics/kp_viewer_03.png)





