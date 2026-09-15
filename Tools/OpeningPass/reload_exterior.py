import unreal
m=unreal.load_asset('/Game/EntranceRepairs/SM_ExteriorPassage')
print('RESTORE_GARDEN_RAILS',unreal.EditorLoadingAndSavingUtils.reload_packages([m.get_outermost()],unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE))
