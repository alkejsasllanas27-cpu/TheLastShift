import unreal,json
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
assert not w.get_game_world(),'Stop PIE before editing'
mirror={'Door_02_01','Door_02_05','P1_door6_SM','P1_door3_SM010'}
north={'P1_door3_SM','P1_door3_SM001','P1_door3_SM002','P1_door3_SM003'}
saved=[];report=[]
with unreal.ScopedEditorTransaction('Fix room swing and paired-door outer hinges'):
 for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  label=a.get_actor_label()
  if label not in mirror|north|{'P1_door3_SM004'}:continue
  a.modify()
  if label in mirror:
   c=a.get_editor_property('DoorMesh');b=c.static_mesh.get_bounding_box()
   center=unreal.MathLibrary.transform_location(c.get_world_transform(),(b.min+b.max)*.5)
   h=unreal.MathLibrary.transform_location(a.get_actor_transform(),a.get_editor_property('HingeOffsetLocal'))
   a.set_editor_property('HingeOffsetLocal',unreal.MathLibrary.inverse_transform_location(a.get_actor_transform(),center*2-h))
  a.set_editor_property('OpenAngle',85.0 if label=='P1_door3_SM004' else -90.0)
  saved.append(a.get_package());report.append(label)
assert len(report)==9
print('FINAL_DOOR_FIX',report,unreal.EditorLoadingAndSavingUtils.save_packages(list(set(saved)),True))
