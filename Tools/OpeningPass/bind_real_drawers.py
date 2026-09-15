import unreal,json
folder='/Game/ShiftAssets/Drawer/vintage_wooden_drawer_01_'
es=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);actors={a.get_actor_label():a for a in es.get_all_level_actors()};saved=[]
for label in ['Shift_ReceptionSupplyDrawer']:
 a=actors[label];a.set_editor_property('drawer_body',unreal.load_asset(folder+'body'));a.set_editor_property('drawer_meshes',[unreal.load_asset(folder+'drawer'+str(i).zfill(2)) for i in range(1,7)])
 a.set_editor_property('open_sound',unreal.load_asset('/Game/ShiftAssets/Audio/DrawerOpen'));a.set_editor_property('close_sound',unreal.load_asset('/Game/ShiftAssets/Audio/DrawerClose'));a.rebuild();saved.append(a)
b=actors['Shift_Battery_InReceptionDrawer'];b.set_actor_relative_location(unreal.Vector(0,0,30),False,True);saved.append(b)
n=actors['Shift_NightShiftNote'];v=n.get_actor_location();v.z=1423;n.set_actor_location(v,False,True);saved.append(n)
unreal.EditorLoadingAndSavingUtils.save_packages(list({a.get_package() for a in saved}),True)
print('REAL_DRAWERS_BOUND')
