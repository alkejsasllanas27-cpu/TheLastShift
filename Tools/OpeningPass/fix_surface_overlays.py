import unreal
for n in [83,84]:
 m=unreal.load_asset('/Game/HospitalExterior2/92m_'+str(n));m.modify()
 m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
 m.set_editor_property('opacity_mask_clip_value',0.02)
 expr=unreal.find_object(None,m.get_path_name()+':MaterialExpressionTextureSample_0')
 unreal.MaterialEditingLibrary.connect_material_property(expr,'G',unreal.MaterialProperty.MP_OPACITY_MASK)
 unreal.MaterialEditingLibrary.recompile_material(m)
print('SURFACE_OVERLAYS_PREVIEW_READY')
