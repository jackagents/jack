import { LatLng } from 'leaflet';
import { waypointDivIcon } from 'misc/icons/common/cmIcons';
import React from 'react';
import { Marker } from 'react-leaflet';

interface Props {
  rebaseLocation?: LatLng;
}

function RebaseLayer({ rebaseLocation }: Props) {
  return (
    <>
      {rebaseLocation && (
        <Marker icon={waypointDivIcon} position={rebaseLocation} />
      )}
    </>
  );
}

export default React.memo(RebaseLayer);
