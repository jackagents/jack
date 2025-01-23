import React from 'react';
import * as protomaps from 'protomaps';
import { useMap } from 'react-leaflet';

export default function PMTilesLayer() {
  const map = useMap();

  React.useEffect(() => {
    const layer = protomaps.leafletLayer({
      url: 'http://localhost:3000/localmap.pmtiles',
    });
    layer.addTo(map);

    return () => {
      map.removeLayer(layer);
    };
  });

  return null;
}
