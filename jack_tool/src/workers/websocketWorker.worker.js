const { BSON } = require('bson');
const websocket = require('websocket');
const { parentPort, workerData } = require('worker_threads');

const WebSockeClient = websocket.client;
const eventData = [];

async function getEventData(previousRecordIndex) {
  setImmediate(() => {
    const result = eventData.slice(
      previousRecordIndex + 1,
      eventData.length - 1
    );
    parentPort?.postMessage({ type: 'getEventData', data: result });
  });
}

function initWebsocketThread(address, storeMessages) {
  let timeZero = -1;
  const client = new WebSockeClient();

  client.on('connectFailed', (error) => {
    parentPort?.postMessage({ type: 'connectFailed', data: error });
  });

  client.on('connect', (connection) => {
    parentPort?.postMessage({ type: 'connect', data: null });

    connection.on('error', (error) => {
      parentPort?.postMessage({ type: 'error', data: error });
      client.abort();
    });

    connection.on('close', () => {
      parentPort?.postMessage({ type: 'close', data: null });
    });

    connection.on('message', (message) => {
      if (message.type === 'binary') {
        // Through direct cbdi => websocket => client
        const data = BSON.deserialize(message.binaryData);

        if (timeZero < 0) {
          timeZero = data.timestampUs;
        }

        data.timestampUs -= timeZero;

        if (storeMessages) {
          eventData.push(data);
        }

        parentPort?.postMessage({ type: 'message', data });
      }
    });
  });

  client.connect(address);
}

const { address } = workerData;

parentPort?.on('message', (message) => {
  switch (message.command) {
    case 'getData':
      getEventData(message.previousRecordIndex);
      break;

    case 'initWebsocket':
      initWebsocketThread(address, message.storeMessages);
      break;

    default:
      break;
  }
});
