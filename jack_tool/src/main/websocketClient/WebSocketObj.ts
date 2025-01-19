import websocket from 'websocket';
import LOGGER from 'misc/addons/logger/LoggerSingleton';
import { BaseBuilder } from 'main/beBuilders/baseBuilder/BaseBuilder';
import { Event as NonFlatBufferEvent } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { ConnectStatus } from 'constant/common/cmConstants';
import { BSON } from 'bson';
import playbackManager from 'main/playbackManager/playBackManager';
import { response } from 'misc/events/common/cmEvents';
import validateCbdiMessage from './validator/validateCbdiMessage';

const WebSockeClient = websocket.client;

interface Props {
  /**
   * Electron.BrowserWindow
   */
  window: Electron.BrowserWindow;
  /**
   * Ip address
   */
  address: string;
  /**
   * builders
   */
  builders?: BaseBuilder[];
  /**
   * Callback on message
   */
  onMessage?: (data: NonFlatBufferEvent) => void;
  // onMessage?: (data: CBDIMessage) => void;
  /**
   * Callback on message
   */
  onConnected?: () => void;
  /**
   * Callback on message
   */
  onDisconneted?: () => void;
}

export default class WebSocketObj {
  private _window: Electron.BrowserWindow;

  private _address: string;

  /**
   * WebSocket Client in Node
   */
  private _ws: websocket.client;

  /**
   * Agent builder
   */
  private _builders: BaseBuilder[];

  /**
   * Websocket connection
   */
  private _wsConnection: websocket.connection | undefined;

  /**
   * Array of messages received from server
   */
  private _msgs: string[] = [];

  /**
   * NodeJS.Timeout for reconnection
   */
  private _reconnectTimeout: NodeJS.Timeout | null = null;

  /**
   * Client ws connection status
   */
  private _wsStatus: ConnectStatus = ConnectStatus.disconnected;

  /**
   * First message timestamp in microsecond 1sec = 1000000 microsecond
   */
  private timeZero: number;

  /**
   * Callback on message
   */
  private _onMessage?: (data: NonFlatBufferEvent) => void;

  /**
   * Callback on message
   */
  private _onConnected?: () => void;

  /**
   * Callback on message
   */
  private _onDisconnected?: () => void;

  // private wfs = fs.createWriteStream(
  //   path.join(process.cwd(), 'wsmess.log.json')
  // );

  // private wsWorker: Worker | null = null;

  /**
   * Constructor
   * @param props
   */
  constructor(props: Props) {
    this._window = props.window;

    this._address = props.address;

    this._builders = props.builders ?? [];

    this._ws = this._init();

    this.timeZero = -1;

    this._onMessage = props.onMessage;
    this._onConnected = props.onConnected;
    this._onDisconnected = props.onDisconneted;
  }

  /**
   * Start websocket.client and connect to url
   * @returns WebSockeClient
   */
  private _init() {
    const client = new WebSockeClient();

    client.on('connectFailed', () => {
      LOGGER.warn('Connect to server at', this._address, 'failed');

      // start auto reconnect timeout
      this._reconnectTimeout = setTimeout(() => {
        LOGGER.warn('Attempt reconnect');
        this.start();
      }, 1000);
    });

    client.on('connect', (connection) => {
      this._wsStatus = ConnectStatus.connected;

      // clear reconnect timeout
      if (this._reconnectTimeout) {
        clearTimeout(this._reconnectTimeout);
      }

      if (this._onConnected) {
        this._onConnected();

        playbackManager.goLive();
        playbackManager.runLoop((time) => {
          this._window.webContents.send(response.playback.tick, time);
        });
      }

      LOGGER.info('WebSocket Client Connected');

      this._wsConnection = connection;

      while (this._msgs.length > 0) {
        const msg = this._msgs.pop();

        if (msg) {
          connection.send(msg);
        }
      }

      connection.on('error', (error) => {
        LOGGER.error(`Connection Error: ${error.toString()}`);
        client.abort();
      });

      connection.on('close', () => {
        if (this._onDisconnected) {
          this._onDisconnected();
        }

        // clear when websocket is disconnected
        this.stop();
        this.cleanup();
        this.clearBuilders();
        playbackManager.clear();
        playbackManager.stop();

        LOGGER.info('echo-protocol Connection Closed');

        // reconnect
        // if (this._wsStatus === ConnectStatus.connected) {
        //   this._wsStatus = ConnectStatus.connecting;
        //   this._reconnectTimeout = setTimeout(() => {
        //     this.start();
        //   }, 1000);
        // }
      });

      connection.on('message', (message) => {
        if (message.type === 'binary') {
          // Through direct cbdi => websocket => client
          const data = BSON.deserialize(message.binaryData) as NonFlatBufferEvent;

          // This can be expensive as it checks the schema of message
          // Should only run this in DEVELOPMENT
          if (process.env.NODE_ENV === 'development') {
            validateCbdiMessage(data);
          }

          if (this.timeZero < 0) {
            this.timeZero = data.timestampUs;
          }

          data.timestampUs -= this.timeZero;

          // this.wfs.write(`${JSON.stringify(data)}\n`);

          playbackManager.feedLiveEvent(data, this._window);

          if (this._onMessage) {
            this._onMessage(data);
          }

          // Check debug step available, maybe costly for every message check
          if (playbackManager.isLive() && playbackManager.isTracing() && playbackManager.isBreakPoint(data)) {
            playbackManager.stopByBreakPoint(playbackManager.lastRecordsIndex());
          }
        }
      });
    });

    return client;
  }

  /**
   * Start connection
   */
  start(newAddress?: string) {
    if (newAddress && newAddress !== this._address) {
      this._address = newAddress;
    }

    this._ws.abort();
    this._wsStatus = ConnectStatus.connecting;

    if (this._reconnectTimeout) {
      clearTimeout(this._reconnectTimeout);
    }

    this._ws.connect(this._address);

    // https://gitlab.aosgrp.net/applications/aewcf/-/issues/975
    // if (this.wsWorker) {
    //   this.wsWorker.terminate();
    //   this.wsWorker = null;
    // }

    // this.wsWorker = new Worker(
    //   path.join(__dirname, '../../workers/websocketWorker.worker.js'),
    //   {
    //     workerData: {
    //       address: newAddress,
    //     },
    //   }
    // );

    // this.wsWorker.on('error', (err) => {
    //   LOGGER.error('ws thread error', err);
    // });

    // this.wsWorker.on('exit', (exitCode) => {
    //   LOGGER.warn('ws thread exit with exit code', exitCode);
    // });

    // this.wsWorker.on('messageerror', (err) => {
    //   LOGGER.error('ws thread message error', err);
    // });

    // this.wsWorker.on('message', (message: any) => {
    //   switch (message.type) {
    //     case 'connectFailed': {
    //       LOGGER.warn('Connect to server at', this._address, 'failed');

    //       // start auto reconnect timeout
    //       this._reconnectTimeout = setTimeout(() => {
    //         LOGGER.warn('Attempt reconnect');
    //         this.start();
    //       }, 1000);

    //       break;
    //     }
    //     case 'connect': {
    //       this._wsStatus = ConnectStatus.connected;

    //       // clear reconnect timeout
    //       if (this._reconnectTimeout) {
    //         clearTimeout(this._reconnectTimeout);
    //       }

    //       if (this._onConnected) {
    //         this._onConnected();

    //         playbackManager.goLive();
    //         playbackManager.runLoop((time) => {
    //           this._window.webContents.send(response.playback.tick, time);
    //         });
    //       }

    //       LOGGER.info('WebSocket Client Connected');

    //       break;
    //     }
    //     case 'error': {
    //       LOGGER.error(`Connection Error: ${message.error.toString()}`);
    //       break;
    //     }
    //     case 'close': {
    //       if (this._onDisconnected) {
    //         this._onDisconnected();
    //       }

    //       // clear when websocket is disconnected
    //       this.stop();
    //       this.cleanup();
    //       this.clearBuilders();
    //       playbackManager.clear();
    //       playbackManager.stop();

    //       LOGGER.info('echo-protocol Connection Closed');

    //       break;
    //     }
    //     case 'message': {
    //       playbackManager.feedLiveEvent(message.data, this._window);

    //       if (this._onMessage) {
    //         this._onMessage(message.data);
    //       }

    //       // Check debug step available, maybe costly for every message check
    //       if (
    //         playbackManager.isLive() &&
    //         playbackManager.isTracing() &&
    //         playbackManager.isBreakPoint(message.data)
    //       ) {
    //         playbackManager.stopByBreakPoint(
    //           playbackManager.lastRecordsIndex()
    //         );
    //       }

    //       break;
    //     }
    //     case 'getEventData': {
    //       ipcMain.emit('get-event-data-from-ws-thread', message.data);
    //       break;
    //     }

    //     default:
    //       break;
    //   }
    // });

    // this.wsWorker.on('exit', () => {
    //   this.wsWorker?.terminate();
    // });

    // this.wsWorker.postMessage({
    //   command: 'initWebsocket',
    //   storeMessages: playbackManager.isLiveDebug(),
    // });
  }

  /**
   * Stop connection
   */
  stop() {
    // https://gitlab.aosgrp.net/applications/aewcf/-/issues/975
    // if (this.wsWorker) {
    //   this.wsWorker.terminate();
    // }

    this._ws.abort();

    // stop auto reconnect timeout
    if (this._reconnectTimeout) {
      clearTimeout(this._reconnectTimeout);
    }

    // close if connection is opened
    if (this._wsStatus === ConnectStatus.connected && this._wsConnection) {
      this._wsConnection.close();
    }
    // execute disconnected if it is not open
    else if (this._onDisconnected) {
      this._onDisconnected();
    }

    this._wsStatus = ConnectStatus.disconnected;
  }

  /**
   * Send message
   */
  send(msg: string) {
    if (this._wsConnection) {
      LOGGER.info('going to send to websocket', msg);
      this._wsConnection.send(msg);
    } else {
      this._msgs.push(msg);
    }
  }

  /**
   * Get connection status
   * @returns boolean isConnected
   */
  isConnected() {
    return this._wsConnection?.connected ?? false;
  }

  /**
   * Clear builders
   */
  clearBuilders() {
    this._builders.forEach((builder) => {
      builder.clear();
    });
  }

  cleanup() {
    this.stop();
    this.clearBuilders();
    playbackManager.clear();
    this.timeZero = -1;
    // playbackManager.goLive();
  }

  /**
   * Get current connect status
   * @returns ConnectStatus status
   */
  status() {
    return this._wsStatus;
  }

  // https://gitlab.aosgrp.net/applications/aewcf/-/issues/975
  /**
   * Request data from ws thread
   */
  // getDataFromWsThread(previousRecordIndex: number) {
  //   LOGGER.info('Get data from index', previousRecordIndex);
  //   this.wsWorker?.postMessage({ command: 'getData', previousRecordIndex });
  // }
}
