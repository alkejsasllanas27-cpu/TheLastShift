import unreal
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
packages=list({a.get_package() for a in actors if a.get_actor_label().startswith('Horror_Blood')})
print('SAVE_BLOOD',unreal.EditorLoadingAndSavingUtils.save_packages(packages,True))
print('RELOAD_DOC',unreal.EditorLoadingAndSavingUtils.reload_packages.__doc__)
