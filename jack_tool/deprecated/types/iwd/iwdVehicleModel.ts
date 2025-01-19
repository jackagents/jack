// import { GeoPosition } from 'types/iwd/iwdTypes';

export interface GeoPosition {
  lat: number;
  lng: number;
}

export enum IwdVehicleType {
  friendly = 'friendly',
  hostile = 'hostile',
  neutral = 'neutral',
}

export enum IwdVehicleOperationStatus {
  activated,
  inactivated,
}

export enum IwdVehicleDriveMode {
  auto,
  manual,
  safetyLock,
}

export type IwdPhysicalEntityModel = {
  /**
   * A uuid string.
   */
  id: string;
  /**
   * A string
   */
  name: string;
  /**
   * A GeoPosition {lat: number, lng: number} object.
   */
  position: GeoPosition;
  /**
   * Contains planned and history paths (Array of GeoPosition).
   */
  path: {
    planned: GeoPosition[];
    history: GeoPosition[];
  };
  /**
   * Last changed timestamp
   */
  lastChanged: string;
  /**
   * Base position: GeoPosition {lat: number, lng: number} object.
   */
  basePosition: GeoPosition;
  /**
   * Rotation degree
   */
  direction: number;
  /**
   * Speed: number
   */
  speed: number;
  /**
   * (Optional) Target position: GeoPosition {lat: number, lng: numer} object.
   */
  targetPosition?: GeoPosition;
  /**
   * (Optional) Mission id string.
   */
  missionId?: string;
};

export enum IwdVehicleStatus {
  idle = 'idle',
  moving = 'moving',
  paused = 'paused',
}

export interface IwdVehicleModel extends IwdPhysicalEntityModel {
  /**
   * Battery level
   */
  battery: number;
  /**
   * Hostile / Neutral / Friendly
   */
  type: IwdVehicleType;
  /**
   * Activated / Inactivated
   */
  operationStatus: IwdVehicleOperationStatus;
  /**
   * Idle / Moving / Paused
   */
  status: IwdVehicleStatus;
  /**
   * auto / manual / safety lock
   */
  driveMode: IwdVehicleDriveMode;
}
