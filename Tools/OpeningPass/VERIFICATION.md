# Opening scenario — 2026-09-15

## Implemented and verified
- Poly Haven CC0 wooden filing cabinet: separate body and six drawers, animated upper middle drawer. Both placed drawer actors reference the imported meshes.
- Independent CC0 DrawerOpen and DrawerClose recordings, distinct from existing doors.
- Flashlight charge percentage, charge bar, on/off state, spare count and replacement hint.
- Native pause/start menu with resume, quality preset, VSync, window mode, resolution, frame cap and quit. Graphics settings use GameUserSettings and are saved.
- Reception log -> first room maintenance record -> return outside main gate opening scenario.
- Electrical ambience and one-shot light failure assets assigned to the existing authored audio zones.
- Entrance gate query surfaces target barred leaves through gaps; no extra pawn collision.
- Matte paper replaces the incorrect atlas on the note.

## Evidence
Both TheLastShiftEditor and TheLastShift Win64 Development builds succeeded.
Saved/OpeningPass/menu-test.json: all 6 checks passed, including model and distinct recording references.
Saved/OpeningPass/legacy-door-test.json: all 8 checks passed in a clean PIE session, including refocusing the open gate and closing it.
Saved/OpeningPass/objective-test.json: all 5 checks passed.
Saved/OpeningPass/gameplay-test.json: battery pickup/replacement/drain, drawer animation, hiding entry/exit, and cabinet tests passed with the imported model. The historical note_read_and_put_away entry is a placeholder assertion and is not evidence of UI validation.
Native Slate screenshots inspected for HUD and menu layout, and the imported drawer's actual in-game appearance.

## Limits and remaining work
This is an opening scenario, not a completed AAA campaign. Remaining rooms need authored gameplay/story, an enemy/threat system, player interaction animations, save/load and broader performance/accessibility work. Resolution and window controls compile and call engine APIs but were not exercised in a packaged build. No cooked/distributable package has been produced; the standalone executable compilation does not constitute packaging. Objective transition tests use direct interactions/teleports, not a full manual walk through the level.
Map-wide material reference audit and representative views do not prove every visual artifact is fixed.
Road geometry was preserved. The abandoned rail-removal script was moved out of Tools to Saved/OpeningPass/Abandoned to prevent accidental reapplication.
