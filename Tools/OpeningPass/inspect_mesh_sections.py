import unreal,json
m=unreal.load_asset('/Game/EntranceRepairs/SM_ExteriorPassage')
s=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
print('LOD_API',[(n,getattr(s,n).__doc__) for n in dir(s) if 'material' in n or 'lod_count' in n])
print('MESH_SECTIONS',m.get_num_sections(0),[(i,str(x.material_slot_name),x.material_interface.get_name() if x.material_interface else None) for i,x in enumerate(m.static_materials)])
