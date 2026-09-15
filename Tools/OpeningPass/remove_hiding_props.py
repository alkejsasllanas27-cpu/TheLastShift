import unreal, json, shutil, pathlib
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world(), 'Stop PIE first'
actors = sub.get_all_level_actors()
targets = [a for a in actors if isinstance(a, unreal.ShiftInteractable) and
           (a.kind == unreal.ShiftObjectKind.HIDING_CABINET or a.get_actor_label() == 'Shift_FirstRoom_EmptyDrawer')]
backup = pathlib.Path('C:/Users/Admin/TheLastShift/Saved/OpeningPass/BeforeHidingRemoval')
backup.mkdir(parents=True, exist_ok=True)
source = pathlib.Path('C:/Users/Admin/TheLastShift/Content/FirstPerson/Lvl_FirstPerson.umap')
if not (backup / source.name).exists(): shutil.copy2(source, backup / source.name)
removed = []
with unreal.ScopedEditorTransaction('Remove unwanted hiding props'):
 for actor in targets:
  removed.append({'label': actor.get_actor_label(), 'location': str(actor.get_actor_location()), 'kind': str(actor.kind)})
  assert sub.destroy_actor(actor), 'Actor deletion failed'
 for actor in actors:
  if isinstance(actor, unreal.ShiftInteractable) and actor.get_actor_label() == 'Shift_NightShiftNote':
   actor.modify()
   actor.set_editor_property('note_text', unreal.Text('NIGHT SHIFT — 03:17\n\nThe backup circuit is failing. Keep a spare cell in the reception drawer.\n\nThe maintenance record is in the storage cabinet in the first room. Recover it and leave through the main gate.\n— M.'))
assert unreal.EditorLoadingAndSavingUtils.save_current_level()
remaining = [{'label': a.get_actor_label(), 'kind': str(a.kind)} for a in sub.get_all_level_actors() if isinstance(a, unreal.ShiftInteractable)]
assert not any(a['kind'] == str(unreal.ShiftObjectKind.HIDING_CABINET) or a['label'] == 'Shift_FirstRoom_EmptyDrawer' for a in remaining)
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/hiding-removal.json', 'w') as f:
 json.dump({'removed': removed, 'remaining': remaining}, f, indent=2)
print('HIDING_PROPS_REMOVED', removed)
