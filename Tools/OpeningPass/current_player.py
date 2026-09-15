import unreal,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
p=unreal.GameplayStatics.get_player_character(w,0)
out={}
if p:
 l=p.get_actor_location();r=p.get_control_rotation();out={'location':[l.x,l.y,l.z],'rotation':[r.pitch,r.yaw,r.roll]}
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/current-player.json','w') as f:json.dump(out,f)
