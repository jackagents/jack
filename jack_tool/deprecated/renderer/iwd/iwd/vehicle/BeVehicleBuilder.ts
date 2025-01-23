import { ipcMain, IpcMainInvokeEvent } from 'electron';
import { v4 } from 'uuid';
import { IwdPathMissionModel } from 'types/iwd/iwdMissionModel';
import { iwdRequest, iwdResponse } from 'misc/events/iwd/iwdEvents';
import {
  GeoPosition,
  IwdVehicleDriveMode,
  IwdVehicleModel,
  IwdVehicleOperationStatus,
  IwdVehicleStatus,
  IwdVehicleType,
} from 'misc/types/iwd/iwdVehicleModel';
import BeVehicle from 'root/deprecated/renderer/iwd/iwd/vehicle/BeVehicle';
import { getCurrentDateStr } from 'main/util';

interface Props {
  window: Electron.BrowserWindow;
}

export default class BeVehicleBuilder {
  private _window: Electron.BrowserWindow;

  private _vehicles: BeVehicle[];

  constructor(props: Props) {
    this._window = props.window;
    this._vehicles = [];
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
   * @param vehicle
   */
  add = (vehicle: BeVehicle) => {
    this._vehicles.push(vehicle);
  };

  /**
   * Callback on create vehicle
   * @param _event
   * @param latlng
   */
  private _onCreateVehicle = (_event: IpcMainInvokeEvent, latlng: string) => {
    const position = JSON.parse(latlng) as GeoPosition;

    const vehicle = this._createVehicle(position);

    this.add(vehicle);

    this._window.webContents.send(
      iwdResponse.vehicle.create,
      JSON.stringify(vehicle)
    );
  };

  /**
   * Update vehicle by using vehicle param id
   * @param _e
   * @param data
   */
  private _onUpdateVehicle = (_e: IpcMainInvokeEvent, data: string) => {
    const model = JSON.parse(data) as IwdVehicleModel;
    const vehicle = this._getVehicleFromId(model.id);

    vehicle?.updateModel(model);

    this._window.webContents.send(
      iwdResponse.vehicle.info,
      JSON.stringify(vehicle?.getModel())
    );
  };

  /**
   * Callback on update position
   * @param _e IpcMainInvokeEvent
   * @param id vehicleId: string
   * @param data stringified GeoPosition
   */
  private _onUpdatePosition = (
    _e: IpcMainInvokeEvent,
    id: string,
    data: string
  ) => {
    const latlng = JSON.parse(data) as GeoPosition;

    const vehicle = this._getVehicleFromId(id);

    vehicle?.updatePosition(latlng);
  };

  /**
   * Callback on client requests vehicle position.
   * @param _e IpcMainInvokeEvent
   * @param id vehicleId: string
   */
  private _onRequestVehiclePosition = (_e: IpcMainInvokeEvent, id: string) => {
    const vehicle = this._getVehicleFromId(id);
    this._window.webContents.send(
      iwdResponse.vehicle.requestCurrentVehiclePosition,
      JSON.stringify(vehicle?.getPosition()),
      vehicle?.getStatus()
    );
  };

  /**
   * Callback on client requests to update status.
   * @param _e IpcMainInvokeEvent
   * @param id vehicleId: string
   * @param status vehicle status: Idle/Moving/Paused...
   */
  private _onUpdateVehicleStatus = (
    _e: IpcMainInvokeEvent,
    id: string,
    status: IwdVehicleStatus
  ) => {
    const vehicle = this._getVehicleFromId(id);

    // update vehicle status in backend
    vehicle?.updateStatus(status);

    // update status for ui
    this._window.webContents.send(iwdResponse.vehicle.updateStatus, id, status);

    // update status for front end builder animation
    this._window.webContents.send(
      iwdResponse.vehicle.update,
      JSON.stringify(vehicle)
    );
  };

  /**
   * Callback on client requests vehicle info.
   * @param _e IpcMainInvokeEvent
   * @param id vehicleId: string
   */
  private _onRequestVehicleInfo = (_e: IpcMainInvokeEvent, id: string) => {
    const vehicle = this._getVehicleFromId(id);

    this._window.webContents.send(
      iwdResponse.vehicle.info,
      JSON.stringify(vehicle?.getModel())
    );
  };

  /**
   * Get vehicle from id
   * @param id vehicleId: string
   * @returns vehicle
   */
  private _getVehicleFromId = (id: string) => {
    return this._vehicles.find((x) => x.getId() === id);
  };

  /**
   * Create vehicle
   * @param position
   * @returns
   */
  private _createVehicle = (position: GeoPosition) => {
    return new BeVehicle({
      id: v4(),
      basePosition: position,
      battery: 100,
      direction: 0,
      driveMode: IwdVehicleDriveMode.auto,
      name: `vehicle_${this._vehicles.length}`,
      operationStatus: IwdVehicleOperationStatus.activated,
      path: {
        planned: [],
        history: [],
      },
      position,
      type: IwdVehicleType.friendly,
      lastChanged: getCurrentDateStr(),
      speed: 1,
      status: IwdVehicleStatus.idle,
    });
  };

  /**
   * Callback when user choose rebase a vehicle.
   * @param event Electron.IpcMainInvokeEvent
   * @param iwdVehicle  string
   */
  private _onRebaseVehicle = (
    _event: Electron.IpcMainInvokeEvent,
    iwdVehicle: string
  ) => {
    this._window.webContents.send(iwdResponse.vehicle.rebase, iwdVehicle);
  };

  private _onRebaseVehicleFinalised = (
    _event: Electron.IpcMainInvokeEvent,
    id: string,
    newBase: string
  ) => {
    const vehicle = this._vehicles.find((x) => x.getId() === id);
    vehicle?.updateBase(JSON.parse(newBase) as GeoPosition);

    // Send new vehicles array to front end
    this._window.webContents.send(
      iwdResponse.vehicle.update,
      JSON.stringify(vehicle)
    );
  };

  /**
   * Callback on cllient created new path mission.
   * @param _event IpcMainEvent
   * @param data PathMission model data string
   */
  private _onNewPathMissionCreated = (
    _event: Electron.IpcMainEvent,
    data: string
  ) => {
    const pathMission = JSON.parse(data) as IwdPathMissionModel;

    const vehicle = this._vehicles.find(
      (x) => x.getId() === pathMission.vehicleId
    );

    vehicle?.updateMissionId(pathMission.id);
  };

  /**
   * Register event handlers with callbacks
   */
  registerEventHandlers = () => {
    ipcMain.handle(iwdRequest.vehicle.create, this._onCreateVehicle);
    ipcMain.handle(iwdRequest.vehicle.update, this._onUpdateVehicle);
    ipcMain.handle(
      iwdRequest.vehicle.updateStatus,
      this._onUpdateVehicleStatus
    );
    ipcMain.handle(iwdRequest.vehicle.updatePosition, this._onUpdatePosition);
    ipcMain.handle(iwdRequest.vehicle.info, this._onRequestVehicleInfo);
    ipcMain.handle(
      iwdRequest.vehicle.requestCurrentVehiclePosition,
      this._onRequestVehiclePosition
    );
    ipcMain.handle(iwdRequest.vehicle.rebase, this._onRebaseVehicle);
    ipcMain.handle(
      iwdRequest.vehicle.rebaseFinalised,
      this._onRebaseVehicleFinalised
    );

    ipcMain.on(
      iwdResponse.mission.newPathMission,
      this._onNewPathMissionCreated
    );
  };

  deregisterEventHandlers = () => {
    ipcMain.removeHandler(iwdRequest.vehicle.create);
    ipcMain.removeHandler(iwdRequest.vehicle.update);
    ipcMain.removeHandler(iwdRequest.vehicle.updateStatus);
    ipcMain.removeHandler(iwdRequest.vehicle.updatePosition);
    ipcMain.removeHandler(iwdRequest.vehicle.info);
    ipcMain.removeHandler(iwdRequest.vehicle.requestCurrentVehiclePosition);
    ipcMain.removeHandler(iwdRequest.vehicle.rebase);
    ipcMain.removeHandler(iwdRequest.vehicle.rebaseFinalised);

    ipcMain.removeAllListeners(iwdResponse.mission.newPathMission);
  };
}
