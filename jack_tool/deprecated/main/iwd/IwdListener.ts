import { ipcMain, IpcMainInvokeEvent } from 'electron';
import BeVehicleBuilder from 'root/deprecated/renderer/iwd/iwd/vehicle/BeVehicleBuilder';
import { request, response } from 'misc/events/common/cmEvents';
import { iwdResponse } from 'misc/events/iwd/iwdEvents';
import BePathMissionBuilder from 'root/deprecated/renderer/iwd/iwd/pathMission/BePathMissionBuilder';
import WebSocketObj from 'main/websocketClient/WebSocketObj';
import BaseListener, { TListenerProps } from 'listeners/base/BaseListener';
import MODEL_BUILDER from 'main/beBuilders/cbdi/cbdiModelBuilder/cbdiModelBuilder';
// import { EventType } from 'types/cbdi/cbdiTypes';
import { EventType } from 'types/cbdi/auto-generated-types';
import LOG_BUILDER from 'main/beBuilders/cbdi/logBuilder/cbdiLogBuilder';
import INTENTION_BUILDER from 'main/beBuilders/cbdi/intentionBuilder/cbdiIntentionBuilder';

export default class IwdListener extends BaseListener {
  private _vehicleBuilder: BeVehicleBuilder;

  private _pathMissionBuilder: BePathMissionBuilder;

  private websocketObj: WebSocketObj;

  /* -------------------------------------------------------------------------- */
  /*                                 CONSTRUCTOR                                */
  /* -------------------------------------------------------------------------- */

  constructor(props: TListenerProps) {
    super(props);

    this._vehicleBuilder = new BeVehicleBuilder({ window: this._window });

    this._pathMissionBuilder = new BePathMissionBuilder({
      window: this._window,
    });

    this.websocketObj = new WebSocketObj({
      window: this._window,
      address: 'ws://127.0.0.1:1313',
      builders: [MODEL_BUILDER, LOG_BUILDER, INTENTION_BUILDER],
      onConnected: () => {
        this._window.webContents.send(response.websocket.connected);
      },
      onDisconneted: () => {
        this._window.webContents.send(response.websocket.disconnected);
      },
      onMessage: (data) => {
        // Send percept event to render position
        if (data.eventType === EventType.EventType_MESSAGE) {
          this._window.webContents.send(
            iwdResponse.websocket.message,
            JSON.stringify(data)
          );
        } else if (data.eventType === EventType.EventType_CONTROL) {
          this._window.webContents.send(
            iwdResponse.websocket.control,
            JSON.stringify(data)
          );
        } else {
          // LOGGER.warn('WS: Event is not handled to UI', JSON.stringify(data));
        }
      },
      // onMessage: (data) => {
      //   // Send percept event to render position
      //   if (data.eventType === EventType.MESSAGE) {
      //     this._window.webContents.send(
      //       iwdResponse.websocket.message,
      //       JSON.stringify(data)
      //     );
      //   } else if (data.eventType === EventType.CONTROL) {
      //     this._window.webContents.send(
      //       iwdResponse.websocket.control,
      //       JSON.stringify(data)
      //     );
      //   } else {
      //     // LOGGER.warn('WS: Event is not handled to UI', JSON.stringify(data));
      //   }
      // },
    });
  }

  /* -------------------------------------------------------------------------- */
  /*                                  OVERRIDE                                  */
  /* -------------------------------------------------------------------------- */

  override showAbout = (clientName: string) => {
    super.showAbout(clientName);
  };

  override start() {
    super.start();
    this.deregisterEventHandlers();
    this.registerEventHandlers();

    this._vehicleBuilder.start();

    this._pathMissionBuilder.start();

    return this;
  }

  protected override onWindowClose = (): void => {
    throw new Error('Not implemented');
  };

  protected override onWindowReadyToShow = (): void => {
    throw new Error('Not implemented');
  };

  /* -------------------------------------------------------------------------- */
  /*                                   PRIVATE                                  */
  /* -------------------------------------------------------------------------- */

  /**
   * Callback when user right click on marker
   * @param _event IpcMainInvokeEvent
   * @param type type of clicked marker
   * @param cursor stringified position of cursor, including latlng and screen coordinates
   * @param object (optional) object can be an agent or point of interest...
   */
  private onContextmenuOnMarker = (
    _event: IpcMainInvokeEvent,
    type: number,
    cursor: string,
    object?: string
  ) => {
    this._window.webContents.send(
      response.project.contextmenu,
      type,
      cursor,
      object
    );
  };

  /**
   * Callback when user creates point of interest. Takes Electron event, lat number, lng number and send them to client
   * @param _event Electron.IpcMainInvokeEvent
   * @param lat latitude
   * @param lng longitude
   */
  private onCreatePointOfInterest = (
    _event: Electron.IpcMainInvokeEvent,
    lat: number,
    lng: number
  ) => {
    this._window.webContents.send(
      response.project.createPointOfInterest,
      lat,
      lng
    );
  };

  /**
   * Callback when user creates zone. Takes in Electron event, stringified zone array and forward it to client
   * @param event Electron.IpcMainInvokeEvent
   * @param zone Stringified zone array
   */
  private onCreateZone = (
    _event: Electron.IpcMainInvokeEvent,
    zone: string
  ) => {
    this._window.webContents.send(response.project.createZone, zone);
  };

  /**
   * Callback when user creates zone. Takes in Electron event, stringified zone and forward it to client
   * @param event Electron.IpcMainInvokeEvent
   * @param zone (optional) String zone
   */
  private onUndoZone = (_event: Electron.IpcMainInvokeEvent, zone?: string) => {
    this._window.webContents.send(response.project.undoZone, zone);
  };

  /* -------------------------------------------------------------------------- */
  /*                                   PUBLIC                                   */
  /* -------------------------------------------------------------------------- */

  /**
   * Register event handlers with callbacks
   */
  registerEventHandlers() {
    super.registerEventHandlers();
    ipcMain.handle(request.project.contextmenu, this.onContextmenuOnMarker);
    ipcMain.handle(
      request.project.createPointOfInterest,
      this.onCreatePointOfInterest
    );
    ipcMain.handle(request.project.createZone, this.onCreateZone);
    ipcMain.handle(request.project.undoZone, this.onUndoZone);
  }

  /**
   * Deregister event handlers with callbacks
   */
  deregisterEventHandlers = () => {
    super.deregisterEventHandlers();
    ipcMain.removeHandler(request.project.contextmenu);
    ipcMain.removeHandler(request.project.createPointOfInterest);
    ipcMain.removeHandler(request.project.createZone);
    ipcMain.removeHandler(request.project.undoZone);
  };
}
