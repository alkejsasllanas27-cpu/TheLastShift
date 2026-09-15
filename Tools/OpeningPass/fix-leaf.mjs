import {connect,call,client} from './mcp.mjs';
import fs from 'node:fs';
const root='C:/Users/Admin/TheLastShift/';
const asset='/Game/HospitalExterior2/92m_85.92m_85';
fs.mkdirSync(root+'Saved/OpeningPass/Before',{recursive:true});
const backup=root+'Saved/OpeningPass/Before/92m_85.uasset';
if(!fs.existsSync(backup))fs.copyFileSync(root+'Content/HospitalExterior2/92m_85.uasset',backup);
await connect();
try{
 const mat={refPath:asset};
 await call('editor_toolset.toolsets.object.ObjectTools','set_properties',{instance:mat,values:JSON.stringify({blendMode:'BLEND_Masked',opacityMaskClipValue:0.3333,twoSided:true})});
 await call('editor_toolset.toolsets.material.MaterialTools','connect_to_output',{expression:{refPath:asset+':MaterialExpressionTextureSample_0'},output_name:'A',material_property:'MP_OpacityMask'});
 await call('editor_toolset.toolsets.material.MaterialTools','recompile',{material_or_function:mat});
 const img=await call('EditorToolset.EditorAppToolset','CaptureAssetImage',{assetPath:asset});
 fs.writeFileSync(root+'Tools/OpeningPass/leaf-material-after.png',Buffer.from(img.data,'base64'));
 console.log('Leaf material corrected in editor; awaiting visual verification before saving.');
}finally{await client.close();}
