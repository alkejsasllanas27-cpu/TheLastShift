import unreal,json
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world(),'Stop PIE before audit'
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
doors=[];materials={};props=[];sounds=[]
for a in actors:
 if 'BP_InteractableDoor_C' in a.get_class().get_name():
  c=a.get_editor_property('DoorMesh');m=c.static_mesh
  if m:
   b=m.get_bounding_box();ct=c.get_world_transform();wc=unreal.MathLibrary.transform_location(ct,(b.min+b.max)*.5);h=unreal.MathLibrary.transform_location(a.get_actor_transform(),a.get_editor_property('HingeOffsetLocal'))
   doors.append({'label':a.get_actor_label(),'path':a.get_path_name(),'center':[wc.x,wc.y,wc.z],'hinge':[h.x,h.y,h.z],'angle':a.get_editor_property('OpenAngle'),'scale':str(a.get_actor_scale3d())})
 for c in a.get_components_by_class(unreal.StaticMeshComponent):
  if c.static_mesh:
   for i in range(c.get_num_materials()):
    m=c.get_material(i);path=m.get_path_name() if m else 'MISSING'
    materials.setdefault(path,[]).append(a.get_actor_label())
reg=unreal.AssetRegistryHelpers.get_asset_registry()
for d in reg.get_assets_by_path('/Game',True):
 n=str(d.asset_name);cl=str(d.asset_class_path.asset_name)
 if cl in ['SoundWave','SoundCue']:sounds.append(str(d.package_name))
 if cl=='StaticMesh' and any(s in n.lower() for s in ['cabinet','drawer','wardrobe','battery','desk','locker','table','note']):props.append(str(d.package_name))
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/final-audit.json','w') as f:json.dump({'doors':doors,'materials':materials,'props':props,'sounds':sounds},f,indent=2)
print('FINAL_AUDIT',len(doors),len(materials),len(props),len(sounds))
