import unreal,json
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
existing={a.get_actor_label():a for a in sub.get_all_level_actors()}
created=[]
def floor(x,y):
 h=unreal.SystemLibrary.line_trace_single_by_profile(world,unreal.Vector(x,y,1390),unreal.Vector(x,y,1250),'Pawn',True,[],unreal.DrawDebugTrace.NONE,True)
 assert h,'No floor at placement'
 z=h.to_tuple()[5].z
 assert 1350<z<1390,'Unexpected floor'
 return z+1
def object(label,kind,x,y,yaw=0,z=None):
 if label in existing:
  a=existing[label];created.append(a);return a
 a=sub.spawn_actor_from_class(unreal.ShiftInteractable,unreal.Vector(x,y,floor(x,y) if z is None else z),unreal.Rotator(pitch=0,yaw=yaw,roll=0))
 a.set_actor_label(label);a.set_editor_property('kind',kind)
 if kind in [unreal.ShiftObjectKind.DRAWER,unreal.ShiftObjectKind.CABINET]:
  sound=unreal.load_asset('/Game/AbandonedHospitalPart2/Audio/DoorCreak');a.set_editor_property('open_sound',sound);a.set_editor_property('close_sound',sound)
 a.rebuild();created.append(a);return a
d=object('Shift_ReceptionSupplyDrawer',unreal.ShiftObjectKind.DRAWER,-4700,1960,90)
b=object('Shift_Battery_InReceptionDrawer',unreal.ShiftObjectKind.BATTERY,-4700,1960,0,d.get_actor_location().z+24)
b.set_editor_property('container',d)
b.attach_to_component(d.moving_root,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
b.set_actor_relative_location(unreal.Vector(0,0,24),False,False)
n=object('Shift_NightShiftNote',unreal.ShiftObjectKind.NOTE,-4700,1960,90,d.get_actor_location().z+59.3)
n.set_editor_property('note_text',unreal.Text('NIGHT SHIFT — 03:17\n\nThe backup circuit is failing. Keep a spare cell in the reception drawer.\n\nThe maintenance record is in the storage cabinet in the first room. Recover it and leave through the main gate.\n— M.'))
n.set_editor_property('story_event','NightShiftEvidence')
storage=object('Shift_FirstRoom_StorageCabinet',unreal.ShiftObjectKind.CABINET,-4000,1350,90)
spare=object('Shift_Battery_StorageCabinet',unreal.ShiftObjectKind.BATTERY,-4000,1350,0,storage.get_actor_location().z+5)
spare.set_editor_property('container',storage)
for label,x,y,event,on_enter in [('Shift_Audio_Entrance',-5450,2210,'HospitalArrival',True),('Shift_Audio_FirstRoom',-4265,1550,'NightShiftEvidence',False)]:
 a=existing.get(label) or sub.spawn_actor_from_class(unreal.ShiftAudioZone,unreal.Vector(x,y,1500));a.set_actor_label(label)
 a.set_editor_property('story_event',event);a.set_editor_property('trigger_story_on_enter',on_enter);created.append(a)
for a in created:a.modify()
assert unreal.EditorLoadingAndSavingUtils.save_packages(list({a.get_package() for a in created}),True)
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/gameplay-placement.json','w') as f:json.dump([{'label':a.get_actor_label(),'path':a.get_path_name(),'location':str(a.get_actor_location())} for a in created],f,indent=2)
print('GAMEPLAY_PLACED',len(created))
