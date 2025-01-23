import React, { useEffect } from 'react';
import L from 'leaflet';
import { useLeafletContext } from '@react-leaflet/core';
import 'leaflet-rotatedmarker';

interface Props extends L.MarkerOptions {
  position: L.LatLngExpression;
  toolTipOptions?: L.TooltipOptions;
  eventHandlers?: L.LeafletEventHandlerFnMap;
}

function isKeyOfEventHandlers(key: string, eventHandlers: L.LeafletEventHandlerFnMap): key is keyof L.LeafletEventHandlerFnMap {
  return key in eventHandlers;
}

export const StaticRotatedMarker = React.forwardRef<L.Marker, Props>((props, ref) => {
  const context = useLeafletContext();

  const { position, rotationAngle, icon, title, toolTipOptions, pane, interactive, draggable, eventHandlers, zIndexOffset } = props;

  let direction: L.Direction;
  let permanent: boolean;
  let offset: L.PointExpression;
  let sticky: boolean;
  let opacity: number;
  let className: string;

  if (toolTipOptions) {
    direction = toolTipOptions.direction || 'center';
    permanent = toolTipOptions.permanent || true;
    offset = toolTipOptions.offset || [0, -20];
    opacity = toolTipOptions.opacity || 0.7;
    sticky = toolTipOptions.sticky || false;
    className = toolTipOptions.className || 'leaflet-tooltip-own';
  } else {
    direction = 'center';
    permanent = true;
    offset = [0, -20];
    opacity = 0.7;
    sticky = false;
    className = 'leaflet-tooltip-own';
  }

  useEffect(() => {
    const marker = L.marker(position, {
      icon,
      pane: pane || 'markerPane',
      interactive: interactive || false,
      draggable: draggable || false,
      zIndexOffset,
    });

    // Attach the marker to the ref
    if (ref && 'current' in ref) {
      // eslint-disable-next-line no-param-reassign
      ref.current = marker;
    }

    // Attach event handlers
    if (eventHandlers) {
      Object.keys(eventHandlers).forEach((key) => {
        if (isKeyOfEventHandlers(key, eventHandlers)) {
          const func: any = eventHandlers[key]; // TODO: Properly typed the func
          if (func) {
            marker.on(key, (e) => {
              if ('originalEvent' in e) {
                (e.originalEvent as any)?.preventDefault();
              }

              func(e);
            });
          }
        }
      });
    }

    if (title) {
      marker
        .bindTooltip(title, {
          direction,
          permanent,
          sticky,
          offset,
          opacity,
          className,
        })
        .openTooltip();
    }

    if (rotationAngle) {
      // Set rotation if arg is passed in
      marker.setRotationAngle(rotationAngle);
    }

    const container = context.layerContainer || context.map;

    container.addLayer(marker);

    return () => {
      container.removeLayer(marker);
    };
  });

  return null;
});
