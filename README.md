# l_system_octet

A simple implementation of stochastic L-Systems, with a rendering mode using a spring-damper system.

## Compiling and Running.

Download or clone the latest version of Octet Framework (https://github.com/andy-thomason/octet). 
Then clone this repository into a folder in the octet/src/examples directory, or in a custom directory at the same level.
To compile follow the same procedure used for compiling the other Octet examples.

## Keyboard commands:
* **Z** Decreases the rendering scale.
* **X** Increases the rendering scale.
* **A** Decreases the delta-angle of the L-System. 
* **S** Increases the delta-angle of the L-System. 
* **Up** Switches to the next file in the list. 
* **Down** Switches to the previous file in the list. 
* **Right** Switches to the next configuration file.
* **Left** Switches to the previous configuration file.
* **Space** Reloads the current file.
* **Numbers 1-8** Changes the order of the current L-System to the same number.
* **Tab** Reload the current configuration file.
* **E** Exports a PostScript file with the current rendering (named *render.eps*).
* **R** Toggles between the springy renderer and the simple renderer.
* **B** Toggles automatic rescaling of the rendering when the delta-angle is modified.
