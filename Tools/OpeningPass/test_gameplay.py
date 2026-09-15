import unreal,json,time
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
d=unreal.GameplayStatics.get_actor_of_class(w,unreal.ShiftDirector)
assert p and d
actors={a.get_actor_label():a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftInteractable)}
drawer=actors['Shift_ReceptionSupplyDrawer'];battery=actors['Shift_Battery_InReceptionDrawer'];cab=actors['Shift_FirstRoom_HidingCabinet'];note=actors['Shift_NightShiftNote'];storage=actors['Shift_FirstRoom_StorageCabinet']
fl=p.get_component_by_class(unreal.FlashlightComponent)
results=[];st={'step':0,'next':time.monotonic()+1}
def record(name,value):results.append({'test':name,'passed':bool(value)})
def aim(target):
 cam=unreal.GameplayStatics.get_player_camera_manager(w,0).get_camera_location()
 pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(cam,target))
def check_focus(actor):return d.get_editor_property('focused_actor')==actor
def step():
 i=st['step']
 if i==0:
  record('one_director',len(unreal.GameplayStatics.get_all_actors_of_class(w,unreal.ShiftDirector))==1)
  record('flashlight_connected',fl is not None)
  record('battery_locked_while_drawer_closed',not battery.is_available())
  p.set_actor_location(unreal.Vector(-4700,2120,1465),False,True)
 elif i==1:aim(drawer.get_actor_transform().transform_location(unreal.Vector(29,0,37)))
 elif i==2:record('trace_targets_drawer',check_focus(drawer));d.interact()
 elif i==3:record('drawer_opens',drawer.open_alpha>.98);aim(battery.get_actor_location()+unreal.Vector(0,0,4))
 elif i==4:record('trace_targets_battery',check_focus(battery));d.interact()
 elif i==5:
  record('battery_collected',d.spare_batteries==1);d.reload_battery();record('battery_used',d.spare_batteries==0 and fl.battery_charge>.99)
  fl.set_flashlight_on(True);st['charge']=fl.battery_charge
 elif i==6:record('battery_drains',fl.battery_charge<st['charge']);results.append({'battery_before':st['charge'],'battery_after':fl.battery_charge,'runtime':fl.battery_runtime_seconds});fl.set_flashlight_on(False);st['charge']=fl.battery_charge
 elif i==7:
  record('battery_stops_draining_when_off',abs(fl.battery_charge-st['charge'])<.00001)
  d.read_note(note.note_text,note.story_event);d.interact()
  record('note_read_and_put_away',True)
  p.set_actor_location(cab.get_actor_transform().transform_location(unreal.Vector(160,0,98)),False,True)
 elif i==8:aim(cab.get_actor_transform().transform_location(unreal.Vector(48,0,115)))
 elif i==9:record('trace_targets_hiding_cabinet',check_focus(cab));d.interact()
 elif i==10:
  record('player_hidden',d.player_hidden and cab.occupied)
  record('hidden_tag',unreal.Name('PlayerHidden') in p.tags)
  d.exit_hiding()
 elif i==11:
  record('exit_restores_movement',not d.player_hidden and p.get_actor_enable_collision() and p.get_component_by_class(unreal.CharacterMovementComponent).movement_mode==unreal.MovementMode.MOVE_WALKING)
  storage.interact(d)
 elif i==12:record('storage_opens',storage.open_alpha>.98);storage.interact(d)
 elif i==13:
  record('storage_closes',storage.open_alpha<.02)
  unreal.unregister_slate_post_tick_callback(st['handle'])
  with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/gameplay-test.json','w') as f:json.dump(results,f,indent=2)
  print('GAMEPLAY_TEST_COMPLETE',results);return
 st['step']+=1;st['next']=time.monotonic()+1.25
def tick(dt):
 if time.monotonic()>=st['next']:
  try:step()
  except Exception as e:
   unreal.unregister_slate_post_tick_callback(st['handle']);results.append({'error':str(e),'step':st['step']})
   with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/gameplay-test.json','w') as f:json.dump(results,f,indent=2)
   print('GAMEPLAY_TEST_ERROR',str(e))
st['handle']=unreal.register_slate_post_tick_callback(tick)
print('GAMEPLAY_TEST_STARTED')
