import unreal,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
rows=[]
for x in [-4800,-4600,-4440,-4265,-4110,-3900]:
 for y in [1350,1530,1600,1750,1960,2450,2600]:
  h=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(x,y,1400),unreal.Vector(x,y,500),'Pawn',True,[],unreal.DrawDebugTrace.NONE,True)
  if h:
   t=h.to_tuple();rows.append([x,y,round(t[5].z,1),t[9].get_actor_label() if t[9] else None])
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/floors.json','w') as f:json.dump(rows,f)
