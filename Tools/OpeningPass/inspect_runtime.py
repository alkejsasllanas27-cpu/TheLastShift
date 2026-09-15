import unreal,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
cam=unreal.GameplayStatics.get_player_camera_manager(w,0);l=cam.get_camera_location();r=cam.get_camera_rotation()
h=unreal.SystemLibrary.line_trace_single_by_profile(w,l,l+unreal.MathLibrary.get_forward_vector(r)*240,'Pawn',True,[p],unreal.DrawDebugTrace.NONE,True)
out={'camera':str(l),'rotation':str(r),'player':str(p.get_actor_location()),'hit':str(h.to_tuple()[9]) if h else None,'objects':[]}
for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftInteractable):out['objects'].append({'label':a.get_actor_label(),'position':str(a.get_actor_location()),'open':a.open_alpha,'available':a.is_available()})
fl=p.get_component_by_class(unreal.FlashlightComponent);out['flashlight']={'charge':fl.battery_charge,'on':fl.is_flashlight_on(),'tick':fl.is_component_tick_enabled()}
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/runtime.json','w') as f:json.dump(out,f,indent=2)
