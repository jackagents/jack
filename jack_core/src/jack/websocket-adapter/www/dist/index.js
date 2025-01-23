(function polyfill() {
  const relList = document.createElement("link").relList;
  if (relList && relList.supports && relList.supports("modulepreload")) {
    return;
  }
  for (const link of document.querySelectorAll('link[rel="modulepreload"]')) {
    processPreload(link);
  }
  new MutationObserver((mutations) => {
    for (const mutation of mutations) {
      if (mutation.type !== "childList") {
        continue;
      }
      for (const node of mutation.addedNodes) {
        if (node.tagName === "LINK" && node.rel === "modulepreload")
          processPreload(node);
      }
    }
  }).observe(document, { childList: true, subtree: true });
  function getFetchOpts(script) {
    const fetchOpts = {};
    if (script.integrity)
      fetchOpts.integrity = script.integrity;
    if (script.referrerpolicy)
      fetchOpts.referrerPolicy = script.referrerpolicy;
    if (script.crossorigin === "use-credentials")
      fetchOpts.credentials = "include";
    else if (script.crossorigin === "anonymous")
      fetchOpts.credentials = "omit";
    else
      fetchOpts.credentials = "same-origin";
    return fetchOpts;
  }
  function processPreload(link) {
    if (link.ep)
      return;
    link.ep = true;
    const fetchOpts = getFetchOpts(link);
    fetch(link.href, fetchOpts);
  }
})();
const style = "";
var ws = null;
var msgs = [];
var Tstart = 0;
var Topen = 0;
var Terror = 0;
var Tclose = 0;
var agents = {};
function setupJACK(element) {
  element.addEventListener("DOMContentLoaded", startup, false);
}
function renderModel() {
  var h = document.getElementById("ws_message");
  h.innerHTML = "---------";
  for (const [key, value] of Object.entries(agents)) {
    h.insertAdjacentHTML("afterbegin", "<div><p>" + key + ": " + JSON.stringify(value) + "</p></div>");
  }
}
function send(msg2) {
  console.log("Will send msg of length:" + msg2.length + "Starts with:" + msg2.substring(0, 40));
  if (ws == null || ws.readyState !== 1) {
    console.log("queueing message:" + msg2);
    msgs.push(msg2);
  } else {
    ws.send(msg2);
  }
}
function startup() {
  Tstart = performance.now();
  Topen = 0;
  Terror = 0;
  Tclose = 0;
  document.getElementById("btn_restart").addEventListener("click", test_restart);
  document.getElementById("btn_close").addEventListener("click", test_close);
  document.getElementById("btn_send1").addEventListener("click", stop_agents);
  document.getElementById("btn_send2").addEventListener("click", start_agents);
  document.getElementById("btn_send3").addEventListener("click", test_send3);
  document.getElementById("btn_send4").addEventListener("click", test_send4);
  document.getElementById("btn_send5").addEventListener("click", test_send5);
  document.getElementById("btn_send6").addEventListener("click", test_send6);
  document.getElementById("btn_clear").addEventListener("click", test_clear);
  document.getElementById("start").innerHTML = "Started " + Date();
  if (ws === null || ws && ws.readyState === 3) {
    console.log("protocol ", window.location.protocol);
    console.log("host ", window.location.host);
    let host = "127.0.0.1:9001";
    ws = new WebSocket(
      (window.location.protocol === "https:" ? "wss://" : "ws://") + host + "/api"
    );
    ws.onopen = function(event) {
      Topen = performance.now();
      console.log("onopen", event);
      document.getElementById("ws_open").innerHTML = "onopen " + Date() + "." + (Topen - Tstart) + " protocol:" + ws.protocol;
      while (msgs.length > 0) {
        ws.send(msgs.pop());
      }
    };
    ws.onerror = function(event) {
      Terror = performance.now();
      console.log("onerror", event);
      document.getElementById("ws_error").innerHTML = "onerror " + Date() + "." + (Terror - Tstart) + " - " + event;
    };
    ws.onclose = function(event) {
      Tclose = performance.now();
      console.log("onclose", event);
      document.getElementById("ws_close").innerHTML = "onclose " + Date() + "." + (Tclose - Tstart) + " - " + event;
    };
    ws.addEventListener("message", function(event) {
      performance.now();
      let obj = JSON.parse(event.data);
      console.log(obj);
      if (obj.recipient.type === "AGENT") {
        var agent_name = obj.recipient.name;
        var agent_id = obj.recipient.id;
        var agent = { id: agent_id, beliefsets: {} };
        if (agent_name in agents) {
          agent = Object.assign({}, agents[agent_name]);
        }
        if (obj.body_type === "Percept") {
          var bs = { [obj.body.value.name]: obj.body.value.data };
          if (obj.body.beliefSet in agent.beliefsets) {
            bs = Object.assign(bs, agent.beliefsets[obj.body.beliefSet]);
          }
          agent.beliefsets[obj.body.beliefSet] = bs;
        }
        agents[agent_name] = agent;
      }
      renderModel();
    });
  }
}
function test_restart() {
  console.log("restart websocket");
  if (msgs.length > 0) {
    console.log("Dropping unsent messages: " + msgs.length);
    msg = [];
  }
  startup();
}
function test_close() {
  console.log("close websocket");
  ws.close();
}
function stop_agents() {
  for (const [key, val] of Object.entries(agents)) {
    let msg2 = {
      senderNode: {
        id: "90fddbd2aec8c0382818041763604c8d",
        name: "DeliveryRobots",
        type: "NODE"
      },
      sender: {
        id: "90fddbd2aec8c0382818041763604c8d",
        name: "DeliveryRobots",
        type: "NODE"
      },
      recipient: {
        id: val.id,
        name: key,
        type: "AGENT"
      },
      body_type: "Control",
      eventId: "1234567890dbf7c0c14ef7c78b300de1",
      body: { command: "STOP" }
    };
    send(JSON.stringify(msg2));
  }
}
function start_agents() {
  for (const [key, val] of Object.entries(agents)) {
    let msg2 = {
      senderNode: {
        id: "90fddbd2aec8c0382818041763604c8d",
        name: "DeliveryRobots",
        type: "NODE"
      },
      sender: {
        id: "90fddbd2aec8c0382818041763604c8d",
        name: "DeliveryRobots",
        type: "NODE"
      },
      recipient: {
        id: val.id,
        name: key,
        type: "AGENT"
      },
      body_type: "Control",
      eventId: "1234567890dbf7c0c14ef7c78b300de1",
      body: { command: "START" }
    };
    send(JSON.stringify(msg2));
  }
}
function test_send3() {
  send("start");
}
function test_send4() {
  send("stop");
}
function test_send5() {
  send_ReqAnalysisEntityAnnotationsForEntities();
  send_ReqActivityByIdList();
}
function test_send6() {
  send_ReqActivityByIdList();
  send_ReqAnalysisEntityAnnotationsForEntities();
}
function test_clear() {
  console.log("clearing messages");
  var h = document.getElementById("ws_message");
  while (h.firstChild) {
    h.firstChild.remove();
  }
  msgs = [];
}
document.querySelector("#app").innerHTML = `
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
`;
setupJACK(document);
