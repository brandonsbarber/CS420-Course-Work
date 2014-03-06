Brandon Barber

##Compilation:##
```make```
The compiled `pic` library must be one directory level above this. For instance, if your directory path for this is `Code/assign1`, then you must have it in `Code/pic`.

##Running##

```./directoryName Images/imageName.jpg```

**Note:** This only takes a black and white image, but should take any size image.

##Key Commands##
###Rendering###
* **1:** Points
* **2:** Wireframe
* **3:** Solid
* **p:** Save Screenshot
###Navigation###
* Left-click and move to rotate
* Left-click with Control pressed to translate
* Left-click with Shift pressed to scale
###Other###
* **Q:** Raise overall elevation
* **A:** Lower overall elevation

My program loads in a black and white image and renders a height-mapped geometry from the contained color values. This mesh can be viewed in three different modes and can be rotated, scaled, and translated around.

In addition to these features, my code allows the user to manipulate the vertical scaling of the mesh while preserving the other dimensions. In doing this, the user can make the terrain steeper or flatter based on key strokes. The mesh is rendered with a red gradient based on the height-map data.

**Note:** To easily view this markdown, simple paste in http://www.ctrlshift.net/project/markdowneditor/