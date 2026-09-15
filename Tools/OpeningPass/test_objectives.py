import unreal,json,time
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0);d=unreal.GameplayStatics.get_actor_of_class(w,unreal.ShiftDirector)
a={x.get_actor_label():x for x in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftInteractable)}
out=[{'test':'storage_story_is_bound','passed':str(a['Shift_FirstRoom_StorageCabinet'].story_event)=='MaintenanceEvidence'}]
d.trigger_story('NightShiftEvidence');out.append({'test':'reception_advances_objective','passed':d.objective_stage>=1})
d.trigger_story('MaintenanceEvidence');out.append({'test':'record_advances_objective','passed':d.objective_stage==2})
zones=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftAudioZone)
out.append({'test':'ambient_and_tension_assigned','passed':len(zones)>=2 and all(z.ambient_sound and z.tension_sound for z in zones)})
p.set_actor_location(unreal.Vector(-13400,2242,1342),False,True)
objective_test={'next':time.monotonic()+1}
def done(dt):
 if time.monotonic()<objective_test['next']:return
 out.append({'test':'exit_completes_scenario','passed':d.objective_stage==3})
 open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/objective-test.json','w').write(json.dumps(out,indent=2))
 unreal.unregister_slate_post_tick_callback(objective_test['handle'])
objective_test['handle']=unreal.register_slate_post_tick_callback(done)
