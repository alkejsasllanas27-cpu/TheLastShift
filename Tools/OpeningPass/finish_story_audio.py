import unreal
tasks=[]
for name in ['ElectricalHum','LightFailure']:
 t=unreal.AssetImportTask();t.filename='C:/Users/Admin/TheLastShift/Saved/AssetSources/'+name+'.wav';t.destination_path='/Game/ShiftAssets/Audio';t.automated=True;t.save=True;tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
hum=unreal.load_asset('/Game/ShiftAssets/Audio/ElectricalHum');hum.set_editor_property('looping',True);unreal.EditorAssetLibrary.save_loaded_asset(hum)
failure=unreal.load_asset('/Game/ShiftAssets/Audio/LightFailure')
actors={a.get_actor_label():a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()};saved=[]
for name in ['Shift_Audio_Entrance','Shift_Audio_FirstRoom']:
 a=actors[name];a.set_editor_property('ambient_sound',hum);a.set_editor_property('ambient_volume',.07);a.set_editor_property('tension_sound',failure);a.set_editor_property('fade_seconds',.25);saved.append(a)
storage=actors['Shift_FirstRoom_StorageCabinet'];storage.set_editor_property('story_event','MaintenanceEvidence');saved.append(storage)
note=actors['Shift_NightShiftNote'];note.set_editor_property('note_text',unreal.Text('NIGHT SHIFT — 03:17\n\nThe backup circuit is failing. Keep a spare cell in the reception drawer.\n\nThe maintenance record is in the storage cabinet in the first room. Recover it and leave through the main gate.\n— M.'));saved.append(note)
unreal.EditorLoadingAndSavingUtils.save_packages(list({a.get_package() for a in saved}),True)
print('STORY_AUDIO_BOUND')
