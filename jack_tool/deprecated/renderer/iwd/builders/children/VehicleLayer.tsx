import React from 'react';
import BeVehicle from 'main/beBuilders/iwd/vehicle/BeVehicle';
import VehicleMarker from './VehicleMarker';

interface Props {
  vehicles: BeVehicle[];
}

function VehicleLayer({ vehicles }: Props) {
  return (
    <div>
      {vehicles.flatMap((v) => {
        if (!v.model) return [];

        return <VehicleMarker key={v.model.id} data={v.model} />;
      })}
    </div>
  );
}
export default React.memo(VehicleLayer);
