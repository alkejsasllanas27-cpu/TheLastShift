import unreal,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
rows=[]
for x in [-13100,-10144,-9150,-9060,-8101,-6290]:
 for z in [1260,1300,1360,1400]:
  h=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(x-100,2242,z),unreal.Vector(x+100,2242,z),'Pawn',True,[],unreal.DrawDebugTrace.NONE,True)
  if h:
   t=h.to_tuple();c=t[10];face=t[15]
   mat=c.get_material_from_collision_face_index(face) if c else None
   rows.append({'x':x,'z':z,'actor':t[9].get_actor_label() if t[9] else None,'face':face,'material':str(mat),'point':str(t[5])})
print('SECTION_API',[(n,getattr(unreal.StaticMeshEditorSubsystem,n).__doc__) for n in dir(unreal.StaticMeshEditorSubsystem) if 'section' in n])
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/road-obstacles.json','w') as f:json.dump(rows,f,indent=2)
