import unreal,time,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0)
lamp=p.get_component_by_class(unreal.FlashlightComponent)
lamp.set_flashlight_on(True)
batt={'time':time.monotonic(),'samples':[]}
def sample(dt):
 batt['samples'].append({'charge':lamp.battery_charge,'on':lamp.is_flashlight_on(),'tick':lamp.is_component_tick_enabled(),'dt':dt,'worldtime':unreal.GameplayStatics.get_time_seconds(w)})
 if len(batt['samples'])>=15:
  unreal.unregister_slate_post_tick_callback(batt['handle'])
  with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/battery-drain.json','w') as f:json.dump(batt['samples'],f,indent=2)
batt['handle']=unreal.register_slate_post_tick_callback(sample)
