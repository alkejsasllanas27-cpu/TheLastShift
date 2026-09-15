import unreal,time
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0);d=unreal.GameplayStatics.get_actor_of_class(w,unreal.ShiftDirector)
drawer=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftInteractable) if a.get_actor_label()=='Shift_ReceptionSupplyDrawer')
p.set_actor_location(unreal.Vector(-4700,2080,1465),False,True)
p.get_component_by_class(unreal.FlashlightComponent).set_flashlight_on(True)
drawer_preview={'next':time.monotonic()+1}
def aim_preview(dt):
 if time.monotonic()<drawer_preview['next']:return
 cam=unreal.GameplayStatics.get_player_camera_manager(w,0).get_camera_location();pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(cam,drawer.get_actor_location()+unreal.Vector(0,10,38)))
 if not drawer.open:drawer.interact(d)
 unreal.unregister_slate_post_tick_callback(drawer_preview['handle'])
drawer_preview['handle']=unreal.register_slate_post_tick_callback(aim_preview)
