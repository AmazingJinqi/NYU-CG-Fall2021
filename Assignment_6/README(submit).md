Assignment 6
=============

### Environment

- MacOS 11.6
- CMake 3.21.2
- GNU Make 3.81
- Apple clang 13.0.0 (clang-1300.0.29.3)

### Mode

There are 6 modes in the program : `INSERTION`, `TRANSLATION`, `ROTATION_SCALE`, `DELETE`, `COLOR` and `ANIMATION`. 

The reason I use `ROTATION_SCALE` mode for rotation and scale instead of using `TRANSLATION` mode is that there is a
conflict between the descriptions of ex1 and ex2 after the mouse is released (for ex1 the triangle is not selected but
for ex2 it is still selected). So professor told me to add a new mode for it.

### Keybindings

- <kbd>i</kbd>: enable `INSERTION` mode
- <kbd>o</kbd>: enable `TRANSLATION` mode
- <kbd>p</kbd>: enable `DELETE` mode
- <kbd>u</kbd>: enable `ROTATION_SCALE` mode
- <kbd>h</kbd>: rotate by 10 degree clockwise
- <kbd>j</kbd>: rotate by 10 degree counter-clockwise
- <kbd>k</kbd>: scaled up by 25%
- <kbd>l</kbd>: scaled down by 25%
- <kbd>c</kbd>: enable `COLOR` mode
- <kbd>1</kbd> to <kbd>9</kbd>: change to different colors
- <kbd>=</kbd>: increase the zoom by 20% zooming in in the center of the screen
- <kbd>-</kbd>: decrease the zoom by 20% zooming out in the center of the screen
- <kbd>w</kbd>, <kbd>a</kbd>, <kbd>s</kbd>, and <kbd>d</kbd>: translate the entire scene, respectively down, right, up and left by 20% of the window size
- <kbd>m</kbd>: enable `ANIMATION` mode and create a new frame of animation
- <kbd>n</kbd>: clear the frames of the animation, keeping only the first frame
- <kbd>z</kbd>: play the animation using linear interpolation
- <kbd>x</kbd>: play the animation using Bézier curve