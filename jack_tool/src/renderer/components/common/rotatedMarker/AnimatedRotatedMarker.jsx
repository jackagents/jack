import { useEffect } from 'react';
import L from 'leaflet';
import { useLeafletContext } from '@react-leaflet/core';
import 'leaflet-rotatedmarker';
import { useInterval } from 'hooks/common/useInterval';

let angle = -1;
let _marker;

export function AnimatedRotatedMarker(props) {
  const context = useLeafletContext();

  const {
    position,
    icon,
    pane,
    isIndefiniteRotate,
    rotationAngle,
    rotatedFrenqucy,
  } = props;

  useEffect(() => {
    const marker = L.marker(position, {
      icon,
      pane: pane || 'markerPane',
    });

    _marker = marker;

    if (angle < 0) {
      // Set angle only once
      angle = rotationAngle || 0;
    }

    if (rotationAngle) {
      // Set rotation if arg is passed in
      marker.setRotationAngle(rotationAngle);
    } else {
      // If no argument will use angle
      marker.setRotationAngle(angle);
    }

    const container = context.layerContainer || context.map;

    container.addLayer(marker);

    return () => {
      container.removeLayer(marker);
    };
  });

  useInterval(() => {
    if (isIndefiniteRotate) {
      angle += 0.5;
      _marker.setRotationAngle(angle);
      if (angle >= 360) angle = 0;
    }
  }, 1 / (rotatedFrenqucy || 30));

  return null;
}
