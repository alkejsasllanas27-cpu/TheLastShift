import {connect,call,client} from './mcp.mjs';
import fs from 'node:fs';
const root=new URL('./',import.meta.url);
const rows=JSON.parse(fs.readFileSync(new URL('materials.json',root)));
await connect();
try {
 fs.mkdirSync(new URL('thumbnails/',root),{recursive:true});
 for(const row of rows){
  const sample=row.textures[0]; if(!sample?.texture?.refPath)continue;
  const img=await call('EditorToolset.EditorAppToolset','CaptureAssetImage',{assetPath:sample.texture.refPath});
  fs.writeFileSync(new URL('thumbnails/'+row.material.refPath.split('/').at(-1).split('.')[0]+'.png',root),Buffer.from(img.data,'base64'));
 }
 console.log('Saved texture thumbnails',rows.length);
}finally{await client.close();}
