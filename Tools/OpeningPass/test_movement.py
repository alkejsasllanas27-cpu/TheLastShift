import unreal,json,time
worlds=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
world=worlds
pawn=unreal.GameplayStatics.get_player_character(world,0)
assert pawn,'PIE player missing'
movement=pawn.get_component_by_class(unreal.CharacterMovementComponent)
controllers=[a for a in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.Actor) if a.get_actor_label() in ['ReceptionDoor_E_Controller','EntranceGate_E_Controller']]
for a in controllers:a.set_editor_property('IsOpen',True)
cases=[(-6500,2210,1342,1,300,-5150),(-5150,2210,1467,-1,300,-6500),(-6500,2210,1342,1,600,-5150),(-5150,2210,1467,-1,600,-6500)]
state={'index':-1,'results':[],'last':time.monotonic()}
def begin():
    state['index']+=1
    if state['index']>=len(cases):
        unreal.unregister_slate_post_tick_callback(state['handle'])
        with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/movement-test.json','w') as f:json.dump(state['results'],f,indent=2)
        print('REAL_MOVEMENT_TEST_COMPLETE',state['results']);return False
    x,y,z,d,s,target=cases[state['index']]
    pawn.set_actor_location(unreal.Vector(x,y,z),False,True)
    movement.stop_movement_immediately();movement.set_editor_property('max_walk_speed',s)
    state['elapsed']=0;state['last']=time.monotonic();return True
def tick(dt):
    now=time.monotonic();elapsed=now-state['last'];state['last']=now
    state['elapsed']+=elapsed
    x,y,z,d,s,target=cases[state['index']]
    pawn.add_movement_input(unreal.Vector(d,0,0),1,True)
    loc=pawn.get_actor_location()
    passed=loc.x>=target if d>0 else loc.x<=target
    if passed or state['elapsed']>12:
        state['results'].append({'case':state['index'],'speed':s,'direction':d,'passed':passed,'end':[loc.x,loc.y,loc.z],'seconds':state['elapsed']})
        begin()
begin()
state['handle']=unreal.register_slate_post_tick_callback(tick)
print('REAL_MOVEMENT_TEST_STARTED')
