import unreal,json
sub=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem);w=sub.get_game_world() or sub.get_editor_world();p=unreal.GameplayStatics.get_player_pawn(w,0)
rows=[]
for y in range(1500,3001,100):
 row={'y':y,'hits':[]}
 for x in range(-6200,-5399,100):
  r=[]
  for top,bottom in [(2600,1800),(1700,1200)]:
   h=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(x,y,top),unreal.Vector(x,y,bottom),'Pawn',True,[p] if p else [],unreal.DrawDebugTrace.NONE,True)
   r.append(round(h.to_tuple()[5].z) if h else None)
  row['hits'].append({'x':x,'upper':r[0],'lower':r[1]})
 rows.append(row)
open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/balcony-grid.json','w').write(json.dumps(rows,indent=2))
