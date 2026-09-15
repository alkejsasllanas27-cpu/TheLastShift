import unreal
sub=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
angles={'Shift_ReceptionSupplyDrawer':90,'Shift_NightShiftNote':90,'Shift_FirstRoom_HidingCabinet':90,'Shift_FirstRoom_EmptyDrawer':180,'Shift_FirstRoom_StorageCabinet':90}
packages=[]
for a in sub.get_all_level_actors():
 if a.get_actor_label() in angles:
  a.modify();a.set_actor_rotation(unreal.Rotator(pitch=0,yaw=angles[a.get_actor_label()],roll=0),False);packages.append(a.get_package())
assert unreal.EditorLoadingAndSavingUtils.save_packages(list(set(packages)),True)
print('PROP_ROTATIONS_VERIFIED')
