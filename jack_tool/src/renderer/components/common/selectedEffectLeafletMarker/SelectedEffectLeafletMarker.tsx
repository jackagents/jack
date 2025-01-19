import React from 'react';
import L from 'leaflet';
import { Marker } from 'react-leaflet';

interface Props {
  position: L.LatLngExpression;
}
export default function SelectedEffectLeafletMarker({ position }: Props) {
  const TargetIcon = React.useMemo(() => {
    return L.divIcon({
      className: 'custom-icon',
      html: `<svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                <defs>
                    <filter id="shadow" x="-20%" y="-20%" width="140%" height="140%">
                        <feDropShadow dx="2" dy="2" stdDeviation="2" flood-color="black" flood-opacity="1"/>
                    </filter>
                </defs>
                <g id="SVGRepo_bgCarrier" stroke-width="0"></g>
                <g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round">
                    <path filter="url(#shadow)" d="M15.6602 2.84952H19.1516C20.2555 2.84952 21.1504 3.74444 21.1504 4.84839V8.3398" stroke="white" stroke-width="2.4" stroke-linecap="round" fill="none"></path>
                    <path filter="url(#shadow)" d="M2.84949 8.33981L2.84949 4.8484C2.84949 3.74446 3.74441 2.84953 4.84836 2.84953L8.33977 2.84953" stroke="white" stroke-width="2.4" stroke-linecap="round" fill="none"></path>
                    <path filter="url(#shadow)" d="M21.1505 15.6602L21.1505 19.1516C21.1505 20.2555 20.2556 21.1505 19.1516 21.1505L15.6602 21.1505" stroke="white" stroke-width="2.4" stroke-linecap="round" fill="none"></path>
                    <path filter="url(#shadow)" d="M8.33984 21.1505L4.84843 21.1505C3.74449 21.1505 2.84956 20.2555 2.84956 19.1516L2.84956 15.6602" stroke="white" stroke-width="2.4" stroke-linecap="round" fill="none"></path>
                </g>
            </svg>`,
      iconSize: [30, 30],
      iconAnchor: [15, 15],
    });
  }, []);

  return <Marker icon={TargetIcon} position={position} />;
}
