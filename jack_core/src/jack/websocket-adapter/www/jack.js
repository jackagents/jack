var ws = null;
var msgs = [];
var Tstart = 0;
var Topen = 0;
var Terror = 0;
var Tclose = 0;
var Tmessage = 0;
var agents = {};

export function setupJACK(element) {
    element.addEventListener('DOMContentLoaded', startup, false);
}

// render the model into the view
function renderModel() {

    var h = document.getElementById('ws_message');
    h.innerHTML = "---------";
    for (const [key, value] of Object.entries(agents)) {
        h.insertAdjacentHTML('afterbegin', "<div><p>" + key + ": " + JSON.stringify(value) + "</p></div>");
    }
}

function send(msg) {
    console.log("Will send msg of length:" + msg.length + "Starts with:" + msg.substring(0, 40))
    if (ws == null || ws.readyState !== 1) {
        console.log("queueing message:" + msg);
        msgs.push(msg);
    } else {
        ws.send(msg);
    }
}
function startup() {
    Tstart = performance.now();
    Topen = 0;
    Terror = 0;
    Tclose = 0;
    Tmessage = 0;
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
    if (ws === null || (ws && ws.readyState === 3)) {

        console.log('protocol ', window.location.protocol)
        console.log('host ', window.location.host)
        let host = '127.0.0.1:9001';

        ws = new WebSocket(
            ((window.location.protocol === "https:") ? "wss://" : "ws://") + host + "/api"
        );
        ws.onopen = function (event) {
            Topen = performance.now();
            console.log('onopen', event)
            document.getElementById("ws_open").innerHTML =
                "onopen " + Date() + "." + (Topen - Tstart) + " protocol:" + ws.protocol;
            while (msgs.length > 0) {
                ws.send(msgs.pop())
            }
        };
        ws.onerror = function (event) {
            Terror = performance.now();
            console.log('onerror', event)
            document.getElementById("ws_error").innerHTML = "onerror " + Date() + "." + (Terror - Tstart) + " - " + event;
        };
        ws.onclose = function (event) {
            Tclose = performance.now();
            console.log('onclose', event)
            document.getElementById("ws_close").innerHTML = "onclose " + Date() + "." + (Tclose - Tstart) + " - " + event;
        };
        ws.addEventListener('message', function (event) {
            Tmessage = performance.now()


            let obj = JSON.parse(event.data);

            console.log(obj);

            // if this is for an agent
            if (obj.recipient.type === 'AGENT') {

                // find the agent in the list
                var agent_name = obj.recipient.name;
                var agent_id = obj.recipient.id;
                var agent = { id: agent_id, beliefsets: {} };

                if (agent_name in agents) {
                    agent = Object.assign({}, agents[agent_name]);
                }

                if (obj.body_type === 'Percept') {

                    var bs = { [obj.body.value.name]: obj.body.value.data };

                    // splice in the current bs
                    if (obj.body.beliefSet in agent.beliefsets) {
                        bs = Object.assign(bs, agent.beliefsets[obj.body.beliefSet]);
                    }

                    agent.beliefsets[obj.body.beliefSet] = bs;
                }

                agents[agent_name] = agent;
            }

            //console.log(agents);
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

    // send a stop message to each agent
    for(const [key, val] of Object.entries(agents)) {
        let msg = {
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
        }

        // start agent
        send(JSON.stringify(msg));
    }
}

function start_agents() {

    // send a stop message to each agent
    for(const [key, val] of Object.entries(agents)) {
        let msg = {
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
        }

        // start agent
        send(JSON.stringify(msg));
    }
}

function test_send2() {
    send("stop")
}
function test_send3() {
    send("start")
}
function test_send4() {
    send("stop")
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
    var h = document.getElementById('ws_message');
    while (h.firstChild) {
        h.firstChild.remove();
    }
    msgs = [];
}
function send_Ping() {
    send('a'.repeat(1672));
}

// document.addEventListener('DOMContentLoaded', startup, false);
