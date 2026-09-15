import unreal,json
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
cdo=unreal.get_default_object(unreal.load_class(None,'/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter.BP_FirstPersonCharacter_C'))
cap=cdo.get_component_by_class(unreal.CapsuleComponent)
radius=cap.get_unscaled_capsule_radius();hh=cap.get_unscaled_capsule_half_height()
step=cdo.get_component_by_class(unreal.CharacterMovementComponent).get_editor_property('max_step_height')
ignore=[a for a in actors if a.get_actor_label() in ['EntranceGate_Left','EntranceGate_Right','ReceptionDoor_Left','ReceptionDoor_Right']]
def data(h):
    if not h:return None
    t=h.to_tuple();p=t[5];n=t[7]
    return {'actor':t[9].get_actor_label() if t[9] else None,'point':[p.x,p.y,p.z],'normal':[n.x,n.y,n.z],'penetrating':t[1]}
rows=[]
for y in [2160,2242,2320]:
    floor=1246
    for x in range(-13700,-4900,20):
        hit=unreal.SystemLibrary.line_trace_single_by_profile(world,unreal.Vector(x,y,floor+step+5),unreal.Vector(x,y,floor-100),'Pawn',False,ignore,unreal.DrawDebugTrace.NONE,True)
        ground=data(hit)
        if ground:floor=ground['point'][2]
        h=unreal.SystemLibrary.capsule_trace_single_by_profile(world,unreal.Vector(x,y,floor+hh+3),unreal.Vector(x+20,y,floor+hh+3),radius,hh,'Pawn',False,ignore,unreal.DrawDebugTrace.NONE,True)
        block=data(h)
        if block:rows.append({'x':x,'y':y,'floor':floor,'block':block})
out={'radius':radius,'half_height':hh,'step_height':step,'ignored_open_doors':[a.get_actor_label() for a in ignore],'blockers':rows}
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/walk-probe.json','w') as f:json.dump(out,f,indent=2)
print('WALK_PROBE_COMPLETE',len(rows))
