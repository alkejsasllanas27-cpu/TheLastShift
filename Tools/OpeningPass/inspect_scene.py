import unreal, json, os
root=unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
assert os.path.normcase(root.rstrip('/\\'))==os.path.normcase('C:/Users/Admin/TheLastShift')
out=[]
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for a in sub.get_all_level_actors():
    label=a.get_actor_label()
    if any(s in label.lower() for s in ['entrance','gate','door','hospital','playerstart','ground','passage','reception']):
        loc=a.get_actor_location();rot=a.get_actor_rotation();org,ext=a.get_actor_bounds(False)
        row={'label':label,'path':a.get_path_name(),'class':a.get_class().get_path_name(),'location':[loc.x,loc.y,loc.z],'rotation':[rot.pitch,rot.yaw,rot.roll],'bounds':[[org.x,org.y,org.z],[ext.x,ext.y,ext.z]],'components':[]}
        for c in a.get_components_by_class(unreal.PrimitiveComponent):
            p={'name':c.get_name(),'class':c.get_class().get_name(),'collision':str(c.get_collision_enabled()),'profile':str(c.get_collision_profile_name())}
            if isinstance(c,unreal.StaticMeshComponent):
                mesh=c.static_mesh;p['mesh']=mesh.get_path_name() if mesh else None
            row['components'].append(p)
        out.append(row)
os.makedirs(root+'/Saved/OpeningPass',exist_ok=True)
with open(root+'/Saved/OpeningPass/scene.json','w') as f:json.dump(out,f,indent=2)
print('OPENING_INSPECTION_COMPLETE',len(out))
