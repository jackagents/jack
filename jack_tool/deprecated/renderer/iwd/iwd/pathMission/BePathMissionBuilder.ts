import { ipcMain, IpcMainInvokeEvent } from 'electron';
import { v4 } from 'uuid';
import { iwdRequest, iwdResponse } from 'misc/events/iwd/iwdEvents';
import { GeoPosition } from 'misc/types/iwd/iwdVehicleModel';
import { IwdMissionType } from 'misc/types/iwd/iwdMissionModel';
import { BePathMission } from 'root/deprecated/renderer/iwd/iwd/pathMission/BePathMission';

interface Props {
  window: Electron.BrowserWindow;
}

export default class BePathMissionBuilder {
  private _window: Electron.BrowserWindow;

  private _pathMissions: BePathMission[];

  constructor(props: Props) {
    this._window = props.window;
    this._pathMissions = [];
  }

  /**
   * Start listening to vehicle builder's events
   */
  start = () => {
    this.deregisterEventHandlers();
    this.registerEventHandlers();
  };

  /**
   * Add vehicle to builder's array
   * @param pathMission
   */
  add = (pathMission: BePathMission) => {
    this._pathMissions.push(pathMission);
  };

  /**
   * Callback on create new path mission.
   * @param event IpcMainInvokeEvent
   * @param vehicleIdData vehicle id string
   * @param waypointsData waypoints string
   */
  private _createNewPathMission = (
    _event: IpcMainInvokeEvent,
    vehicleIdData: string,
    waypointsData: string
  ) => {
    // parse waypoints data
    const waypoints = JSON.parse(waypointsData) as GeoPosition[];

    // init new path mission
    const newPathMission = new BePathMission({
      id: v4(),
      type: IwdMissionType.path,
      vehicleId: vehicleIdData,
      paths: waypoints,
    });

    // get modal and strigify
    const data = JSON.stringify(newPathMission.model);

    // add to path mission list
    this.add(newPathMission);

    // Send to renderer
    this._window.webContents.send(iwdResponse.mission.newPathMission, data);

    // Send to main BeVehicle class
    ipcMain.emit('iwdResponse.mission.newPathMission', data);
  };

  /**
   * Callback on client request for path mission.
   * @param _event IpcMainInvokeEvent
   * @param missionId string
   */
  private _onGetPathMissionRequested = (
    _event: IpcMainInvokeEvent,
    missionId: string
  ) => {
    // get mission from list
    const mission = this._pathMissions.find((x) => x.model.id === missionId);

    // if found, send mission model to associated vehicle only
    if (mission) {
      this._window.webContents.send(
        `${iwdResponse.mission.getPathMission}_${mission.model.vehicleId}`,
        JSON.stringify(mission.model)
      );
    }
  };

  /**
   * Register event handlers with callbacks
   */
  registerEventHandlers = () => {
    ipcMain.handle(
      iwdRequest.mission.newPathMission,
      this._createNewPathMission
    );
    ipcMain.handle(
      iwdRequest.mission.getPathMission,
      this._onGetPathMissionRequested
    );
  };

  deregisterEventHandlers = () => {
    ipcMain.removeHandler(iwdRequest.mission.newPathMission);
    ipcMain.removeHandler(iwdRequest.mission.getPathMission);
  };
}
