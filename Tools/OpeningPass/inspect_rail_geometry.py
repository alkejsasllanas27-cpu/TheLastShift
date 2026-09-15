import unreal,json
m=unreal.load_asset('/Game/EntranceRepairs/SM_ExteriorPassage')
dm=unreal.DynamicMesh()
dm,outcome=unreal.GeometryScript_AssetUtils.copy_mesh_from_static_mesh(m,dm,unreal.GeometryScriptCopyMeshFromAssetOptions(),unreal.GeometryScriptMeshReadLOD())
assert outcome==unreal.GeometryScriptOutcomePins.SUCCESS
S=unreal.GeometryScript_MeshSelection
rows=[]
for x in [-10144,-8105]:
 box=unreal.Box(unreal.Vector(2242-180-2211,-(x+90+6186),1220-1290),unreal.Vector(2242+180-2211,-(x-90+6186),1410-1290))
 _,sel=S.select_mesh_elements_in_box(dm,box,min_num_triangle_points=1)
 for mat in [12,13]:
  _,ms=S.select_mesh_elements_by_material_id(dm,mat)
  intersection=S.combine_mesh_selections(sel,ms,unreal.GeometryScriptCombineSelectionMode.INTERSECTION)
  _,ids,_=S.convert_mesh_selection_to_index_array(dm,intersection)
  _,bounds,empty=unreal.GeometryScript_MeshSelectionQueries.get_mesh_selection_bounding_box(dm,intersection)
  rows.append({'x':x,'mat':mat,'count':len(ids),'bounds':str(bounds),'ids':list(ids)})
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/rail-geometry.json','w') as f:json.dump(rows,f,indent=2)
print('RAIL_SELECTION_READY')
