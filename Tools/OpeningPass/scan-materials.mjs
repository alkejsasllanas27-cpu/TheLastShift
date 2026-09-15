import {connect,call,client} from './mcp.mjs';
import fs from 'node:fs';
const obj='editor_toolset.toolsets.object.ObjectTools', mat='editor_toolset.toolsets.material.MaterialTools';
await connect();
try {
 const mesh={refPath:'/Game/EntranceRepairs/SM_ExteriorPassage.SM_ExteriorPassage'};
 const slots=JSON.parse(await call(obj,'get_properties',{instance:mesh,properties:['staticMaterials']})).staticMaterials;
 const rows=[];
 for(const slot of slots){
  const material=slot.materialInterface;
  const settings=JSON.parse(await call(obj,'get_properties',{instance:material,properties:['blendMode','twoSided','opacityMaskClipValue']}));
  const expressions=await call(mat,'get_expressions',{material_or_function:material});
  const textures=[];
  for(const expression of expressions.filter(e=>e.refPath.includes('TextureSample'))){
   const p=JSON.parse(await call(obj,'get_properties',{instance:expression,properties:['texture']}));
   textures.push({expression,...p});
  }
  rows.push({material,...settings,textures});
 }
 fs.writeFileSync(new URL('materials.json',import.meta.url),JSON.stringify(rows,null,2));
 console.log(JSON.stringify(rows));
}finally{await client.close();}
