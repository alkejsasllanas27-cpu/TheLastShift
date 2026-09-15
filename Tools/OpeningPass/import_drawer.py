import unreal, json
root='C:/Users/Admin/TheLastShift/Saved/AssetSources/'
tasks=[]
for filename,dest in [('VintageDrawer/drawer.fbx','/Game/ShiftAssets/Drawer'),('VintageDrawer/textures/vintage_wooden_drawer_01_diff_2k.jpg','/Game/ShiftAssets/Drawer'),('VintageDrawer/textures/vintage_wooden_drawer_01_arm_2k.jpg','/Game/ShiftAssets/Drawer'),('VintageDrawer/textures/vintage_wooden_drawer_01_nor_gl_2k.jpg','/Game/ShiftAssets/Drawer'),('DrawerOpen.wav','/Game/ShiftAssets/Audio'),('DrawerClose.wav','/Game/ShiftAssets/Audio')]:
 t=unreal.AssetImportTask();t.filename=root+filename;t.destination_path=dest;t.automated=True;t.save=True;t.replace_existing=True
 if filename.endswith('.fbx'):
  o=unreal.FbxImportUI();o.import_mesh=True;o.import_as_skeletal=False;o.import_materials=False;o.import_textures=False;o.automated_import_should_detect_type=False;o.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
  o.static_mesh_import_data.combine_meshes=False;o.static_mesh_import_data.transform_vertex_to_absolute=True;t.options=o
 tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
out=[]
for p in unreal.EditorAssetLibrary.list_assets('/Game/ShiftAssets'):
 a=unreal.load_asset(p);r={'path':p,'class':a.get_class().get_name()}
 if isinstance(a,unreal.StaticMesh):r['bounds']=str(a.get_bounds())
 out.append(r)
open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/imported-drawer.json','w').write(json.dumps(out,indent=2))
