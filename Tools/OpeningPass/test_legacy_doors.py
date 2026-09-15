import unreal,time,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0);d=unreal.GameplayStatics.get_actor_of_class(w,unreal.ShiftDirector)
actors={a.get_actor_label():a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor)}
gate=actors['EntranceGate_E_Controller'];leaf=actors['EntranceGate_Left'];room=actors['P1_door3_SM'];pair=actors['Door_02_01']
checks=[];door_test={'step':0,'next':time.monotonic()+1}
def aim(target):pc.set_control_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.GameplayStatics.get_player_camera_manager(w,0).get_camera_location(),target))
def tick(dt):
 if time.monotonic()<door_test['next']:return
 i=door_test['step']
 try:
  if i==0:p.set_actor_location(unreal.Vector(-13240,2170,1342),False,True)
  elif i==1:aim(unreal.Vector(-13091.5,2170,1420))
  elif i==2:checks.append({'test':'gate_focus','passed':d.focused_actor==leaf});d.interact()
  elif i==3:
   checks.append({'test':'gate_open','passed':abs(leaf.get_actor_rotation().yaw+100)<1})
   p.set_actor_location(unreal.Vector(-13040,1900,1342),False,True)
  elif i==4:
   aim(leaf.get_actor_transform().transform_location(unreal.Vector(0,180,180)))
  elif i==5:
   checks.append({'test':'open_gate_focus','passed':d.focused_actor==leaf});d.interact()
  elif i==6:
   checks.append({'test':'gate_close','passed':abs(leaf.get_actor_rotation().yaw)<1})
   room.call_method('Interact')
  elif i==7:checks.append({'test':'north_room_direction','passed':abs(room.get_actor_rotation().yaw+90)<1});room.call_method('Interact')
  elif i==8:checks.append({'test':'room_returns_closed','passed':abs(room.get_actor_rotation().yaw)<1});pair.call_method('Interact')
  elif i==9:checks.append({'test':'paired_leaf_direction','passed':abs(pair.get_actor_rotation().yaw+90)<1});pair.call_method('Interact')
  elif i==10:
   checks.append({'test':'paired_leaf_returns_closed','passed':abs(pair.get_actor_rotation().yaw)<1})
   unreal.unregister_slate_post_tick_callback(door_test['handle'])
   with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/legacy-door-test.json','w') as f:json.dump(checks,f,indent=2)
   print('LEGACY_DOOR_TEST_COMPLETE',checks);return
  door_test['step']+=1;door_test['next']=time.monotonic()+2.1
 except Exception as e:
  unreal.unregister_slate_post_tick_callback(door_test['handle']);print('LEGACY_DOOR_TEST_ERROR',str(e))
door_test['handle']=unreal.register_slate_post_tick_callback(tick)
