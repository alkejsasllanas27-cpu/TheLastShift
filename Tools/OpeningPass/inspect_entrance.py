import unreal,json,os
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
rows=[]
for y in [2150,2242,2330]:
    for x in range(-13700,-4800,100):
        hit=unreal.SystemLibrary.line_trace_single_by_profile(world,unreal.Vector(x,y,1900),unreal.Vector(x,y,800),'Pawn',False,[],unreal.DrawDebugTrace.NONE,True)
        if hit:
            data=hit.to_tuple();p=data[5]
            rows.append({'x':x,'y':y,'z':p.z,'actor':data[9].get_actor_label() if data[9] else None,'component':data[10].get_name() if data[10] else None})
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/entrance-traces.json','w') as f:json.dump(rows,f,indent=2)
print('ENTRANCE_TRACES_COMPLETE',len(rows))
