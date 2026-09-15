import unreal
if 'st' in globals() and 'handle' in st:
 try:unreal.unregister_slate_post_tick_callback(st['handle'])
 except Exception:pass
