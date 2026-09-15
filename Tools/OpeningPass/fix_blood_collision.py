import unreal,json
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
changed=[]
with unreal.ScopedEditorTransaction('Opening: remove collision from decorative blood planes'):
    for actor in actors:
        if not actor.get_actor_label().startswith('Horror_Blood'):continue
        for c in actor.get_components_by_class(unreal.StaticMeshComponent):
            if not c.static_mesh or c.static_mesh.get_path_name()!='/Engine/BasicShapes/Plane.Plane':continue
            actor.modify();c.modify()
            c.set_collision_profile_name('NoCollision')
            c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
            changed.append(actor.get_actor_label())
print('BLOOD_COLLISION_FIXED',changed)
print('PACKAGE_API',[n for n in dir(unreal.EditorLoadingAndSavingUtils) if 'package' in n])
print('ACTOR_PACKAGE_API',[n for n in dir(actors[0]) if 'package' in n or 'outer' in n])
