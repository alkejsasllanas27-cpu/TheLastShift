import unreal,json
sub=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
w=sub.get_game_world() or sub.get_editor_world()
actors=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.Actor)
out={'candidates':[],'surfaces':[]}
for a in actors:
 label=a.get_actor_label()
 if any(s in label.lower() for s in ['ladder','stair','roof','hospital']):out['candidates'].append({'label':label,'bounds':str(a.get_actor_bounds(True))})
p=unreal.GameplayStatics.get_player_pawn(w,0)
for x in [-5800,-5500,-5200,-4900,-4600]:
 for y in [900,1400,1900,2400,2900,3400]:
  h=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(x,y,4200),unreal.Vector(x,y,1200),'Pawn',True,[p] if p else [],unreal.DrawDebugTrace.NONE,True)
  if h:
   t=h.to_tuple();out['surfaces'].append({'x':x,'y':y,'point':str(t[5]),'normal':str(t[7]),'actor':t[9].get_actor_label() if t[9] else ''})
open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/ladder-site.json','w').write(json.dumps(out,indent=2))
