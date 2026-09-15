import unreal,json
out={}
for clsname in [n for n in dir(unreal) if n.startswith('GeometryScript_') and any(s in n for s in ['Selection','Material','Edit','Query'])]:
 cls=getattr(unreal,clsname,None)
 if cls:out[clsname]={n:getattr(cls,n).__doc__ for n in dir(cls) if any(s in n for s in ['copy_mesh','select_mesh','selection','delete_triangles'])}
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/geometry-api.json','w') as f:json.dump(out,f,indent=2)
