import unreal
# North-side rooms open away from the corridor (Y increases into these rooms).
angles={'P1_door3_SM':-90.0,'P1_door3_SM001':-90.0,'P1_door3_SM002':-90.0,'P1_door3_SM003':-90.0,'P1_door3_SM004':85.0}
packages=[]
with unreal.ScopedEditorTransaction('Opening: configure individual room door swing'):
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        if a.get_actor_label() not in angles:continue
        a.modify();a.set_editor_property('OpenAngle',angles[a.get_actor_label()]);packages.append(a.get_package())
print('SAVE_DOOR_ANGLES',unreal.EditorLoadingAndSavingUtils.save_packages(list(set(packages)),True))
