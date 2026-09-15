import unreal
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 if a.get_actor_label() not in ['Hospital01','P1_door4_SM']:continue
 for c in a.get_components_by_class(unreal.StaticMeshComponent):
  if not c.static_mesh:continue
  for i in range(c.get_num_materials()):
   if not c.get_material(i):print('MISSING_SLOT',a.get_actor_label(),c.static_mesh.get_path_name(),i,str(c.static_mesh.static_materials[i]))
