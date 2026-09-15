import unreal
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
h=unreal.SystemLibrary.line_trace_single_by_profile(w,unreal.Vector(-13000,2242,1900),unreal.Vector(-13000,2242,800),'Pawn',False,[],unreal.DrawDebugTrace.NONE,True)
print('HIT_DIR',dir(h))
print('HIT_TUPLE',h.to_tuple())
print('BREAK',[(n, [x for x in dir(getattr(unreal,n)) if 'break_hit' in x]) for n in ['SystemLibrary','GameplayStatics','MathLibrary']])
