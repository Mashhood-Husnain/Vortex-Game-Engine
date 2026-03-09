=====================================================================================================================================================
PENDING ISSUES:
=====================================================================================================================================================

[BUG] [LOW] Window Fullscreen Issue:
----------------------------------------------------------------
[DESC]:
when window return back from fullscreen, the window reduces in
height and y position moves down with each 'F' press
[STATUS] [PENDING]


[BUG] [LOW] Window Tearing Issue:
----------------------------------------------------------------
[DESC]:
Window tears when in fullscreen mode
[STATUS] [PENDING]


[BUG] [MEDIUM] Player-Camera Anchor:
----------------------------------------------------------------
[DESC]:
when camera is not specified for a player, the camera must not
be anchored when 'R' is pressed
[STATUS] [PENDING]


[REFACTOR] [LOW] Player Logic:
----------------------------------------------------------------
[DESC]:
it's a fucking mess
[STATUS] [PENDING]


=====================================================================================================================================================
SOLVED ISSUES:
=====================================================================================================================================================

[FIXED] Camera Pointing Issue:
----------------------------------------------------------------
[DESC]:
whenever resizing the window, the camera points somewhere else
from where the camera was looking before
[CAUSE]:
i was not updating the camera vectors
[SOL]:
update the camera vectors whenever there is a window size change
[STATUS] [SOLVED]


[FIXED] Window Hanging Issue:
----------------------------------------------------------------
[DESC]: 
window hangs or is 'transparent' while loading the assets
[SOL]:
i deactivate the window until all of the assets are loaded and
then activate it again
[STATUS] [SOLVED]


[FIXED] issue with particle and model depth conflict:
----------------------------------------------------------------
[DESC]:
particle seem to dissapear behind the model when looking from a
certain angle and the particles seem to move from their original
position they are being emitted from
[CAUSE]:
turns out it was just a simple problem where projection matrix
was hardcoded instead of using camera.getProjectionMatrix()
[SOL]:
replaced the hardcoded code with the function call
[STATUS] [SOLVED]
