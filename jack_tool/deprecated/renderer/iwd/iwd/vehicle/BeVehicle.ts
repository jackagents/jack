import {
  GeoPosition,
  IwdVehicleModel,
  IwdVehicleStatus,
} from 'misc/types/iwd/iwdVehicleModel';

export default class BeVehicle {
  model?: IwdVehicleModel;

  constructor(vehicle: IwdVehicleModel) {
    this.model = vehicle;
  }

  getModel = () => {
    return this.model;
  };

  updatePosition = (latlng: GeoPosition) => {
    if (this.model) {
      this.model.position = latlng;
    }
  };

  updateStatus = (status: IwdVehicleStatus) => {
    if (this.model) {
      this.model.status = status;
    }
  };

  updateModel = (model: IwdVehicleModel) => {
    this.model = model;
  };

  updateBase = (newBase: GeoPosition) => {
    if (this.model) {
      // Save new base position
      this.model.basePosition = newBase;

      // If vehicle is idling, move vehicle to base immediately.
      if (this.model.status === IwdVehicleStatus.idle) {
        this.model.position = newBase;
      }
    }
  };

  updateMissionId = (missionId: string) => {
    if (this.model) {
      this.model.missionId = missionId;
    }
  };

  getStatus = () => {
    return this.model?.status;
  };

  getPosition = () => {
    return this.model?.position;
  };

  getId = () => {
    return this.model?.id;
  };
}
