import unreal,json
rows=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if not any(s in a.get_actor_label().lower() for s in ['door','gate','cabinet','drawer','wardrobe']):continue
    l=a.get_actor_location();r=a.get_actor_rotation()
    rows.append({'label':a.get_actor_label(),'class':a.get_class().get_path_name(),'path':a.get_path_name(),'location':[l.x,l.y,l.z],'yaw':r.yaw,'methods':[n for n in dir(a) if any(s in n.lower() for s in ['interact','open','key','input'])]})
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/doors.json','w') as f:json.dump(rows,f,indent=2)
