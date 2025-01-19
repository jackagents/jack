import { useEffect } from 'react';
import { useLeafletContext } from '@react-leaflet/core';
import { AntPath as LeafletAntPath } from 'leaflet-ant-path';
import L from 'leaflet';

export function AntPath(props) {
  const context = useLeafletContext();
  const { positions, options, showWaypoint, waypointIcon } = props;

  useEffect(() => {
    const antPolyline = new LeafletAntPath(positions, options);

    const container = context.layerContainer || context.map;
    const markers = [];
    if (showWaypoint) {
      positions.forEach((position) => {
        let marker;
        if (waypointIcon) {
          marker = L.marker(position, { icon: waypointIcon });
        } else {
          marker = L.marker(position);
        }
        marker.addTo(container);
        markers.push(marker);
      });
    }

    antPolyline.addTo(container);

    return () => {
      container.removeLayer(antPolyline);
      markers.forEach((marker) => {
        container.removeLayer(marker);
      });
    };
  });

  return null;
}
