import {connect,call,client} from './mcp.mjs';
import fs from 'node:fs';
const root='C:/Users/Admin/TheLastShift/';
const ids=[79,81,82,83,84,89,90];
await connect();try{
 fs.mkdirSync(root+'Tools/OpeningPass/cutouts',{recursive:true});
 for(const id of ids){
  const name='92m_'+id,asset='/Game/HospitalExterior2/'+name+'.'+name,material={refPath:asset};
  const backup=root+'Saved/OpeningPass/Before/'+name+'.uasset';
  if(!fs.existsSync(backup))fs.copyFileSync(root+'Content/HospitalExterior2/'+name+'.uasset',backup);
  const prior=await call('editor_toolset.toolsets.material.MaterialTools','get_property_input',{material,material_property:'MP_OpacityMask'});
  if(prior.expression!=='None')throw Error('Existing mask must be reviewed: '+name);
  await call('editor_toolset.toolsets.object.ObjectTools','set_properties',{instance:material,values:JSON.stringify({blendMode:'BLEND_Masked',twoSided:[79,89,90].includes(id),opacityMaskClipValue:0.3333})});
  await call('editor_toolset.toolsets.material.MaterialTools','connect_to_output',{expression:{refPath:asset+':MaterialExpressionTextureSample_0'},output_name:'A',material_property:'MP_OpacityMask'});
  await call('editor_toolset.toolsets.material.MaterialTools','recompile',{material_or_function:material});
  const img=await call('EditorToolset.EditorAppToolset','CaptureAssetImage',{assetPath:asset});
  fs.writeFileSync(root+'Tools/OpeningPass/cutouts/'+name+'.png',Buffer.from(img.data,'base64'));
  console.log(name+' preview ready');
 }
}finally{await client.close();}
