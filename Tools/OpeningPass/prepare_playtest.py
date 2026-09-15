import unreal
pkgs=[unreal.load_asset('/Game/HospitalExterior2/92m_'+str(n)).get_outermost() for n in [83,84]]
print('RELOAD_REVERTED_PREVIEWS',unreal.EditorLoadingAndSavingUtils.reload_packages(pkgs,unreal.ReloadPackagesInteractionMode.ASSUME_POSITIVE))
