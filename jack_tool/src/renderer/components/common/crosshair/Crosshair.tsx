import L from 'leaflet';
import React from 'react';
import { useMap } from 'react-leaflet';
import { crosshairIcon } from 'misc/icons/common/cmIcons';
import './Crosshair.css';

function Crosshair() {
  const map = useMap();

  React.useEffect(() => {
    const crosshair = L.marker(map.getCenter(), {
      icon: crosshairIcon,
    });

    crosshair.addTo(map);

    map.on('move', () => {
      crosshair.setLatLng(map.getCenter());
    });

    return () => {
      map.removeLayer(crosshair);

      map.off('move', () => {
        crosshair.setLatLng(map.getCenter());
      });
    };
  }, []);

  return null;
}

export default React.memo(Crosshair);
