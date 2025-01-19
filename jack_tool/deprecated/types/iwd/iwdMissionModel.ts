import { GeoPosition } from './iwdVehicleModel';

export enum IwdMissionType {
  path = 'path',
}

type IwdMissionModel = {
  /**
   * Mission id.
   */
  id: string;
  /**
   * Mission type: path / ...
   */
  type: IwdMissionType;
  /**
   * Vehicle Id of which associates with mission.
   */
  vehicleId: string;
};

export interface IwdPathMissionModel extends IwdMissionModel {
  /**
   * Path missions waypoints: Array of GeoPosition {lat: number, lng: number}
   */
  paths: GeoPosition[];
}
