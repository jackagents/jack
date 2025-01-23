import './style.css'
import { setupJACK } from './jack.js'

document.querySelector('#app').innerHTML = `
<p id="start"></p>
<div>
  <button id="btn_restart" type="button">Restart Connection</button>
  <button id="btn_close" type="button">Close Connection</button>
  <button id="btn_send1" type="button">Stop Agents</button>
  <button id="btn_send2" type="button">Start Agents</button>
  <button id="btn_send3" type="button">x</button>
  <button id="btn_send4" type="button">x</button>
  <button id="btn_send5" type="button">x</button>
  <button id="btn_send6" type="button">x</button>
  <button id="btn_clear" type="button">Clear</button>
</div-->
<p id="ws_open">Not open</p>
<p id="ws_close">Not closed</p>
<p id="ws_error">No Error</p>
<div id="ws_message"></div>
`

setupJACK(document)
