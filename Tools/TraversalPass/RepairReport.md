# The Last Shift — traversal and material repair

## Changes

- Added a 5 × 5 viewport-centered aim dot to `UShiftPromptWidget`, with a dark outline and no mouse input interception.
- Bound the existing Hospital02 ladder to `AShiftLadder` using the `UseLevelLadderMesh` tag. Its imported visual geometry remains in the hospital mesh.
- Measured the actual capsule path and set stand-off to 40 cm so the player clears the floor edges. The normal 34 cm radius / 96 cm half-height capsule and collision remain enabled.
- Ladder base: `(11473, -1848, 385.1)`, yaw `-90`, climb height `1295.3`, top clearance `55`, exit offset `(-110, 0, 0)`.
- Reparented `blood1m`, `blood2m`, and `blood3m` to `/Game/ShiftAssets/Materials/M_BloodOverlay`, preserving their original textures. The red channel masks the black texture background at a 0.008 threshold. Removed the instances' translucent blend override.

## Controls

Face the ladder and press **E**. Use **W** to climb and **S** to descend. The character steps off automatically at either end; **E** can also step off at an endpoint.

## Validation

- Full `TheLastShiftEditor Win64 Development` build succeeded.
- Actual game character mounted, climbed from the lower floor to the upper landing, and returned to the lower floor.
- Confirmed normal director focus and interaction, collision enabled, walking restored, and movement input restored after dismount.
- Confirmed mid-climb cancellation cannot drop the player off the ladder, followed by successful descent.
- Confirmed the HUD exists in the viewport and `AimDot` uses centered anchors, centered alignment, and a 5 × 5 size.
- Compared the original bathroom camera before and after: the broad black polygons are gone; blood silhouettes remain.
- No dirty map or content packages before the final editor restart.

Validation uses the existing single-player character and this ladder. It does not claim a traversal audit of every stair or structure in the map.
