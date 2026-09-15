import unreal
fixes=[('/Game/EntranceRepairs/SM_Part1EntranceOpening',6,'/Game/AbandonedHospital/Meshes/6m'),('/Game/AbandonedHospital/Meshes/door4_SM',1,'/Game/AbandonedHospital/Meshes/door4m2')]
for meshpath,slot,matpath in fixes:
 m=unreal.load_asset(meshpath);mat=unreal.load_asset(matpath);assert mat
 assert m.static_materials[slot].material_interface is None
 m.modify();m.set_material(slot,mat)
 assert unreal.EditorAssetLibrary.save_loaded_asset(m)
print('MISSING_MATERIALS_RELINKED',len(fixes))
