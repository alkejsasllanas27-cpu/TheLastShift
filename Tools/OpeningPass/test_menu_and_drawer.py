import unreal,json,time
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();d=unreal.GameplayStatics.get_actor_of_class(w,unreal.ShiftDirector);p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
if not d.menu_open:d.toggle_menu()
m=d.get_menu();s=unreal.GameUserSettings.get_game_user_settings();out=[]
out.append({'test':'menu_pauses_game','passed':unreal.GameplayStatics.is_game_paused(w) and d.menu_open})
v=s.is_v_sync_enabled();m.call_method('VSync');out.append({'test':'vsync_applies','passed':s.is_v_sync_enabled()!=v});m.call_method('VSync')
old=s.get_frame_rate_limit();m.call_method('FrameLimit');out.append({'test':'frame_limit_applies','passed':s.get_frame_rate_limit()!=old});s.set_frame_rate_limit(old);s.apply_non_resolution_settings();s.save_settings()
m.call_method('Resume');out.append({'test':'resume_unpauses','passed':not unreal.GameplayStatics.is_game_paused(w) and not d.menu_open})
actors={a.get_actor_label():a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftInteractable)};drawer=actors['Shift_ReceptionSupplyDrawer']
out.append({'test':'real_drawer_meshes','passed':bool(drawer.drawer_body) and len(drawer.drawer_meshes)==6})
out.append({'test':'distinct_drawer_recordings','passed':drawer.open_sound!=drawer.close_sound and 'DrawerOpen' in drawer.open_sound.get_path_name() and 'DrawerClose' in drawer.close_sound.get_path_name()})
p.set_actor_location(unreal.Vector(-4700,2080,1465),False,True);p.get_component_by_class(unreal.FlashlightComponent).set_flashlight_on(True)
real_view={'next':time.monotonic()+1}
def shot(dt):
 if time.monotonic()<real_view['next']:return
 cam=unreal.GameplayStatics.get_player_camera_manager(w,0).get_camera_location();pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(cam,drawer.get_actor_location()+unreal.Vector(0,10,38)))
 unreal.unregister_slate_post_tick_callback(real_view['handle'])
real_view['handle']=unreal.register_slate_post_tick_callback(shot)
open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/menu-test.json','w').write(json.dumps(out,indent=2))
