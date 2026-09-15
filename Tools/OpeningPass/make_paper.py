import unreal
m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_NotePaper','/Game/ShiftAssets/Drawer',unreal.Material,unreal.MaterialFactoryNew())
if not m:m=unreal.load_asset('/Game/ShiftAssets/Drawer/M_NotePaper')
e=unreal.MaterialEditingLibrary;e.delete_all_material_expressions(m)
n=e.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(.73,.68,.55,1));e.connect_material_property(n,'',unreal.MaterialProperty.MP_BASE_COLOR)
r=e.create_material_expression(m,unreal.MaterialExpressionConstant);r.set_editor_property('r',.93);e.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
e.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
print('PAPER_MATERIAL_SAVED')
