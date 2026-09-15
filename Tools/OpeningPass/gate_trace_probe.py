import unreal,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors={a.get_actor_label():a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
out={}
for label in ['EntranceGate_Left','EntranceGate_Right','EntranceGate_E_Controller']:
 a=actors[label];r={'location':str(a.get_actor_location()),'bounds':str(a.get_actor_bounds(False))}
 r['components']=[{'name':c.get_name(),'collision':str(c.get_collision_enabled()),'visibility':str(c.get_collision_response_to_channel(unreal.CollisionChannel.ECC_VISIBILITY))} for c in a.get_components_by_class(unreal.PrimitiveComponent)]
 out[label]=r
hit=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(-13240,2170,1420),unreal.Vector(-13000,2170,1420),'BlockAll',True,[],unreal.DrawDebugTrace.NONE,True)
out['hit']=str(hit.to_tuple()) if hit else None
open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/gate-probe.json','w').write(json.dumps(out,indent=2))
