import {Client} from 'file:///C:/Users/Admin/mcp-servers/unreal-mcp/node_modules/@modelcontextprotocol/sdk/dist/esm/client/index.js';
import {StreamableHTTPClientTransport} from 'file:///C:/Users/Admin/mcp-servers/unreal-mcp/node_modules/@modelcontextprotocol/sdk/dist/esm/client/streamableHttp.js';
export const client = new Client({name:'thelastshift-opening-pass',version:'1'});
export async function connect(){await client.connect(new StreamableHTTPClientTransport(new URL('http://127.0.0.1:8000/mcp')));}
export async function call(toolset_name,tool_name,args={}){
 const r=await client.callTool({name:'call_tool',arguments:{toolset_name,tool_name,arguments:args}},undefined,{timeout:60000});
 if(r.isError) throw Error(JSON.stringify(r));
 return JSON.parse(r.content.find(x=>x.type==='text').text).returnValue;
}
export async function describe(toolset_name){const r=await client.callTool({name:'describe_toolset',arguments:{toolset_name}});return JSON.parse(r.content[0].text);}
