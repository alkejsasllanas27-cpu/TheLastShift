import unreal,json,time,math
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
p=unreal.GameplayStatics.get_player_character(w,0)
assert p
mov=p.get_component_by_class(unreal.CharacterMovementComponent)
for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor):
 if a.get_actor_label() in ['ReceptionDoor_E_Controller','EntranceGate_E_Controller']:a.set_editor_property('IsOpen',True)
route=[(-13600,2242),(-9800,2242),(-9650,2460),(-8500,2460),(-7800,2242),(-6500,2210),(-5150,2210)]
st={'case':-1,'results':[]}
def save():
 with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/full-approach-test.json','w') as f:json.dump(st['results'],f,indent=2)
def begin():
 st['case']+=1
 if st['case']==4:
  mov.stop_movement_immediately();unreal.unregister_slate_post_tick_callback(st['handle']);print('FULL_APPROACH_COMPLETE');return
 st['points']=route if st['case']%2==0 else list(reversed(route));st['waypoint']=1
 x,y=st['points'][0];p.set_actor_location(unreal.Vector(x,y,1342 if st['case']%2==0 else 1467),False,True)
 mov.stop_movement_immediately();mov.set_editor_property('max_walk_speed',300 if st['case']<2 else 600)
 st['start']=time.monotonic();st['progress']=time.monotonic();st['last']=p.get_actor_location()
def tick(dt):
 loc=p.get_actor_location();x,y=st['points'][st['waypoint']];dx=x-loc.x;dy=y-loc.y;dist=math.hypot(dx,dy)
 if (loc-st['last']).length()>15:st['progress']=time.monotonic();st['last']=loc
 passed=False
 if dist<35:
  st['waypoint']+=1
  if st['waypoint']==len(st['points']):passed=True
 if passed or time.monotonic()-st['progress']>5 or time.monotonic()-st['start']>50:
  st['results'].append({'case':st['case'],'passed':passed,'waypoint':st['waypoint'],'end':[loc.x,loc.y,loc.z],'seconds':time.monotonic()-st['start']});save();begin();return
 p.add_movement_input(unreal.Vector(dx/dist,dy/dist,0),1,True)
begin();st['handle']=unreal.register_slate_post_tick_callback(tick)
print('FULL_APPROACH_STARTED')
