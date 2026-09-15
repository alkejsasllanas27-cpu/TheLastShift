import unreal
for n in [42,45,66,88]:
    path='/Game/HospitalExterior2/92m_'+str(n)
    m=unreal.load_asset(path);m.modify()
    m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_MASKED)
    m.set_editor_property('two_sided',True)
    m.set_editor_property('opacity_mask_clip_value',0.3333)
    expr=unreal.find_object(None,m.get_path_name()+':MaterialExpressionTextureSample_0')
    assert expr
    unreal.MaterialEditingLibrary.connect_material_property(expr,'A',unreal.MaterialProperty.MP_OPACITY_MASK)
    unreal.MaterialEditingLibrary.recompile_material(m)
print('FOLIAGE_PREVIEW_READY')
