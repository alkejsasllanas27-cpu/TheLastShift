import unreal,json
rows=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if 'BP_InteractableDoor_C' not in a.get_class().get_name():continue
    c=a.get_editor_property('DoorMesh');m=c.static_mesh
    o,e=a.get_actor_bounds(False);h=a.get_editor_property('HingeOffsetLocal')
    rows.append({'label':a.get_actor_label(),'path':a.get_path_name(),'mesh':m.get_path_name() if m else None,'center':[o.x,o.y,o.z],'extent':[e.x,e.y,e.z],'hinge':[h.x,h.y,h.z],'angle':a.get_editor_property('OpenAngle')})
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/door-geometry.json','w') as f:json.dump(rows,f,indent=2)
