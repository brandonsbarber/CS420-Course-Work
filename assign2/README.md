#Assignment 2: Roller Coaster#

##Compilation:##
`make`

The compiled `pic` library must be one directory level above this. For instance, if your directory path for this is `Code/assign2`, then you must have it in `Code/pic`.

##Running##

```./directoryName track.txt```

**Note:** This works for a single track specified in track.txt as per instructions.

##Key Commands##
###Rendering###
* **1:** Line
* **2:** Track
* **p:** Save Screenshot
* **o:** Toggle between path following and camera orbiting
* **i:** Toggle constant rendering (defaults to false)
###Navigation###
When not in path mode:

* Left-click and move to rotate
* Left-click with Control pressed to translate
* Left-click with Shift pressed to scale

My program loads in a set of points for building a Catmull-Rom roller coaster. By hitting `o` to follow the track, you can take a ride on the roller coaster. With track mode enabled, the car will follow a continuous line around the track as detailed in Sloan method of normal generation. This causes the viewer to have a continuous and smooth trip around the track.

A beautifully texture-mapped ground and sky box exist to add a little bit of color to this world. Upon reaching the end of a track, the camera will reset to the beginning and start all over, though following a different track color this time. This is done by simply not resetting the normals from the previous run, so they influence the next run of the track.

If you want to record this, please make sure that you set constant rendering to true by pressing `i`.

**Note:** To easily view this markdown, simple paste in http://www.ctrlshift.net/project/markdowneditor/
