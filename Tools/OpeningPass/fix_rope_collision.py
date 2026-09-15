import unreal
m=unreal.load_asset('/Game/EntranceRepairs/SM_ExteriorPassage')
s=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
changed=[]
with unreal.ScopedEditorTransaction('Opening: decorative ground rope collision'):
 m.modify()
 for lod in range(s.get_lod_count(m)):
  for section in range(m.get_num_sections(lod)):
   slot=s.get_lod_material_slot(m,lod,section)
   if m.static_materials[slot].material_interface.get_name()=='92m_86':
    changed.append([lod,section,s.is_section_collision_enabled(m,lod,section)])
    s.enable_section_collision(m,False,lod,section)
print('ROPE_SECTION_COLLISION_DISABLED',changed)
print('ROPE_MESH_SAVED',unreal.EditorAssetLibrary.save_loaded_asset(m))
