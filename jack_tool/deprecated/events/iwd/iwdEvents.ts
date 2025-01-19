export const iwdRequest = {
  vehicle: {
    create: 'req-create-vehicle',
    update: 'req-update-vehicle',
    updateStatus: 'req-update-vehicle-status',
    updatePosition: 'update-position',
    info: 'req-vehicle-info',
    requestCurrentVehiclePosition: 'req-request-current-vehicle-position',
    rebase: 'req-rebase',
    rebaseFinalised: 'req-rebase-finalised',
  },
  mission: {
    newPathMission: 'req-create-new-path-mission',
    getPathMission: 'req-get-path-mission',
  },
  websocket: {
    connect: 'iwd-request-connect-websocket',
    disconnect: 'iwd-request-disconnect-websocket',
    send: 'iwd-request-send-cbdi-message',
  },
};

export const iwdResponse = {
  vehicle: {
    create: 'res-create-vehicle',
    update: 'res-update-vehicle',
    updateStatus: 'res-update-vehicle-status',
    info: 'res-vehicle-info',
    requestCurrentVehiclePosition: 'res-request-current-vehicle-position',
    rebase: 'res-rebase',
    getVehicles: 'resp-get-vehicles',
  },
  mission: {
    newPathMission: 'res-create-new-path-mission',
    getPathMission: 'res-get-path-mission',
  },
  websocket: {
    connected: 'iwd-response-websocket-connected',
    disconnected: 'iwd-response-websocket-disconnected',
    percept: 'iwd-response-websocket-percept',
    message: 'iwd-response-websocket-message',
    control: 'iwd-response-websocket-control',
  },
};
