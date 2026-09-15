import unreal,json
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/final-audit.json') as f:paths=json.load(f)['materials']
out=[]
for path,actors in paths.items():
 m=unreal.load_asset(path) if path!='MISSING' else None
 row={'material':path,'instances':len(actors),'actors':list(set(actors))[:4],'textures':[],'missing_samples':[]}
 if m:
  row['class']=m.get_class().get_name()
  if isinstance(m,unreal.Material):
   row['blend']=str(m.get_editor_property('blend_mode'))
   for i in range(32):
    e=unreal.find_object(None,m.get_path_name()+':MaterialExpressionTextureSample_'+str(i))
    if e:
     t=e.get_editor_property('texture')
     if t:row['textures'].append(t.get_path_name())
     else:row['missing_samples'].append(e.get_name())
 out.append(row)
with open('C:/Users/Admin/TheLastShift/Saved/OpeningPass/material-audit.json','w') as f:json.dump(out,f,indent=2)
print('MATERIAL_AUDIT_COMPLETE',len(out),'MISSING',sum(1 for r in out if r['material']=='MISSING' or r['missing_samples']))
