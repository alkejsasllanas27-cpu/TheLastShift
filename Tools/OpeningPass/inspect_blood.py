import unreal,json
a=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
out=[]
for actor in a:
    if 'Blood' not in actor.get_actor_label():continue
    comps=[]
    for c in actor.get_components_by_class(unreal.PrimitiveComponent):
        comps.append({'path':c.get_path_name(),'profile':str(c.get_collision_profile_name()),'collision':str(c.get_collision_enabled()),'mesh':c.static_mesh.get_path_name() if isinstance(c,unreal.StaticMeshComponent) and c.static_mesh else None})
    out.append({'label':actor.get_actor_label(),'path':actor.get_path_name(),'components':comps})
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/blood.json','w') as f:json.dump(out,f,indent=2)
