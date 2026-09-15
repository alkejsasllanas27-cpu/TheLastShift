import unreal,json,math
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
rows=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if 'BP_InteractableDoor_C' not in a.get_class().get_name():continue
    c=a.get_editor_property('DoorMesh');m=c.static_mesh
    if not m:continue
    box=m.get_bounding_box();lo=box.min;hi=box.max;center=(lo+hi)*0.5
    ct=c.get_world_transform();wc=unreal.MathLibrary.transform_location(ct,center)
    if wc.x>-1500 or wc.z<1300:continue
    hinge=unreal.MathLibrary.transform_location(a.get_actor_transform(),a.get_editor_property('HingeOffsetLocal'))
    axis='x' if hi.x-lo.x>hi.y-lo.y else 'y'
    p0=unreal.Vector(center.x,center.y,center.z);p1=unreal.Vector(center.x,center.y,center.z)
    setattr(p0,axis,getattr(lo,axis)+(getattr(hi,axis)-getattr(lo,axis))*.15)
    setattr(p1,axis,getattr(hi,axis)-(getattr(hi,axis)-getattr(lo,axis))*.15)
    ends=[unreal.MathLibrary.transform_location(ct,p)-hinge for p in [p0,p1]]
    results={}
    for sign in [-1,1]:
        hits=[]
        for angle in range(15,91,15):
            pts=[hinge+unreal.MathLibrary.rotate_angle_axis(p,angle*sign,unreal.Vector(0,0,1)) for p in ends]
            h=unreal.SystemLibrary.line_trace_single_by_profile(world,pts[0],pts[1],'Pawn',True,[a],unreal.DrawDebugTrace.NONE,True)
            if h:
                t=h.to_tuple();hits.append({'angle':angle*sign,'actor':t[9].get_actor_label() if t[9] else None})
        results[str(sign)]=hits
    rows.append({'label':a.get_actor_label(),'path':a.get_path_name(),'center':[wc.x,wc.y,wc.z],'current':a.get_editor_property('OpenAngle'),'swing_hits':results})
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/door-swing.json','w') as f:json.dump(rows,f,indent=2)
print('DOOR_SWING_PROBE',len(rows))
