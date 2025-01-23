import L from 'leaflet';
import imgs from 'misc/images';

// import pin from '../../../assets/icons/pin.png';
// import batteryImg from '../../../assets/icons/battery.png';
// import rechargeImg from '../../../assets/icons/chargeStation.png';
// import missileImg from '../../../assets/icons/missile.png';
// import activeAgentImg from '../../../assets/icons/active_agent.png';
// import crosshairImg from '../../../assets/icons/crosshair.png';

// export const arrowIcon = L.icon({
//   iconUrl: arrow,
//   iconSize: [30, 30],
//   iconAnchor: [15, 40],
// });

// export const pinIcon = L.icon({
//   iconUrl: pin,
//   iconSize: [10, 10],
// });

// export const batteryIcon = L.icon({
//   iconUrl: batteryImg,
//   iconSize: [20, 20],
//   iconAnchor: [10, -15],
// });

// export const rechargeStation = L.icon({
//   iconUrl: rechargeImg,
//   iconSize: [30, 30],
//   iconAnchor: [15, 15],
// });

// export const missileIcon = L.icon({
//   iconUrl: missileImg,
//   iconSize: [40, 40],
//   iconAnchor: [20, 20],
// });

export const activeAgentIcon = L.icon({
  iconUrl: imgs.activeAgent,
  iconSize: [30, 48],
  iconAnchor: [15, 24],
});

export const crosshairIcon = L.icon({
  iconUrl: imgs.crosshairImg,
  iconSize: [40, 40],
  iconAnchor: [20, 20],
  className: 'cross-hair-icon',
});

export const iconDefault = L.icon({
  iconUrl: imgs.markerIconImg,
  iconRetinaUrl: imgs.retinaMarkerIconImg,
  shadowUrl: imgs.markerShadowImg,
  iconSize: [25, 41],
  iconAnchor: [12.5, 41],
  popupAnchor: [1, -34],
  tooltipAnchor: [16, -28],
  shadowSize: [41, 41],
});

export const planeIcon = L.icon({
  iconUrl: imgs.planeImg,
  iconSize: [40, 40],
  iconAnchor: [20, 20],
});

export const waypointDivIcon = L.divIcon({
  className: 'rebase-icon',
  html: `
    <svg width="20" height="20">
        <circle cx="10" cy="10" r="10" fill="orange" fill-opacity="0.6" />
        <circle cx="10" cy="10" r="3" fill="red" fill-opacity="0.8"/>
    </svg>`,
  iconAnchor: [10, 10],
});
