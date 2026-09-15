import unreal,json
folder='/Game/ShiftAssets/Drawer/'
e=unreal.MaterialEditingLibrary
m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_VintageDrawer',folder,unreal.Material,unreal.MaterialFactoryNew())
if not m:m=unreal.load_asset(folder+'M_VintageDrawer')
e.delete_all_material_expressions(m)
for suffix,links in [('diff',[('',unreal.MaterialProperty.MP_BASE_COLOR)]),('nor_gl',[('',unreal.MaterialProperty.MP_NORMAL)]),('arm',[('R',unreal.MaterialProperty.MP_AMBIENT_OCCLUSION),('G',unreal.MaterialProperty.MP_ROUGHNESS),('B',unreal.MaterialProperty.MP_METALLIC)])]:
 t=unreal.load_asset(folder+'vintage_wooden_drawer_01_'+suffix+'_2k')
 if suffix=='nor_gl':t.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP);t.set_editor_property('srgb',False);t.set_editor_property('flip_green_channel',True)
 if suffix=='arm':t.set_editor_property('srgb',False);t.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_MASKS)
 n=e.create_material_expression(m,unreal.MaterialExpressionTextureSample);n.set_editor_property('texture',t)
 if suffix=='nor_gl':n.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
 if suffix=='arm':n.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
 for channel,prop in links:e.connect_material_property(n,channel,prop)
 unreal.EditorAssetLibrary.save_loaded_asset(t)
e.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
for path in unreal.EditorAssetLibrary.list_assets(folder):
 a=unreal.load_asset(path)
 if isinstance(a,unreal.StaticMesh):
  a.set_material(0,m);a.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);unreal.EditorAssetLibrary.save_loaded_asset(a)
tasks=[]
for name in ['DrawerOpen','DrawerClose']:
 t=unreal.AssetImportTask();t.filename='C:/Users/Admin/TheLastShift/Saved/AssetSources/'+name+'.wav';t.destination_path='/Game/ShiftAssets/Audio';t.automated=True;t.save=True;t.replace_existing=True;tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
print('DRAWER_ASSETS_COMPLETE')
