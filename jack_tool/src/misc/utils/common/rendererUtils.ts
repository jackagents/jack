import L, { LatLng, Polygon } from 'leaflet';
import * as turf from '@turf/turf';

export function capitalize(str: string): string {
  return str.charAt(0).toUpperCase() + str.slice(1);
}

/**
 * Takes in a latlng point, and a polygon.
 * Returns true if point is in polygon
 * @param point L.LatLng
 * @param poly L.Polygon
 * @returns true if point is in polygon
 */
export function pointInPolygon(point: LatLng, poly: Polygon) {
  const polyPoints = poly.getLatLngs();

  const x = point.lat;
  const y = point.lng;

  let inside = false;
  // eslint-disable-next-line no-plusplus
  for (let i = 0; i < polyPoints.length; i++) {
    const element: any = polyPoints[i];
    // eslint-disable-next-line no-plusplus
    for (let j = 0, k = element.length - 1; j < element.length; k = j++) {
      const xj = element[j].lat;
      const yj = element[j].lng;
      const xk = element[k].lat;
      const yk = element[k].lng;

      const isIntersect = yj > y !== yk > y && x < ((xk - xj) * (y - yj)) / (yk - yj) + xj;
      if (isIntersect) inside = !inside;
    }
  }

  return inside;
}

/**
 * Takes in 2 lat and lng sets, and calculate the distance between them.
 * @param lat1 latitude 1 number
 * @param lat2 longitude 1 number
 * @param lon1 latitude 2 number
 * @param lon2 longitude 2 number
 * @returns
 */
export function distance(lat1: number, lat2: number, lon1: number, lon2: number) {
  // The math module contains a function
  // named toRadians which converts from
  // degrees to radians.
  const radLon1 = (lon1 * Math.PI) / 180;
  const radLon2 = (lon2 * Math.PI) / 180;
  const radLat1 = (lat1 * Math.PI) / 180;
  const radLat2 = (lat2 * Math.PI) / 180;

  // Haversine formula
  const dlon = radLon2 - radLon1;
  const dlat = radLat2 - radLat1;
  const a = Math.sin(dlat / 2) ** 2 + Math.cos(radLat1) * Math.cos(radLat2) * Math.sin(dlon / 2) ** 2;

  const c = 2 * Math.asin(Math.sqrt(a));

  // Radius of earth in kilometers. Use 3956
  // for miles
  const r = 6371;

  // calculate the result
  return c * r;
}

export function getMetersPerPixel(centerLat: number, zoomLevel: number) {
  return (40075016.686 * Math.abs(Math.cos((centerLat * Math.PI) / 180))) / 2 ** (zoomLevel + 8);
}

/**
 * Find max number in an array of numbers
 * @param arr array of numbers
 * @returns max number
 */
export function getMax(arr: number[]) {
  let len = arr.length;
  let max = -Infinity;

  while (len--) {
    max = arr[len] > max ? arr[len] : max;
  }
  return max;
}

/**
 * Find min number in an array of numbers
 * @param arr array of numbers
 * @returns min number
 */
export function getMin(arr: number[]) {
  let len = arr.length;
  let min = Infinity;

  while (len--) {
    min = arr[len] < min ? arr[len] : min;
  }
  return min;
}

/**
 * Takes in cbdi belief sets and return position beliefs if found, otherwise return undefined
 * @param beliefSets cbdi belief sets
 * @returns L.LatLng or undefined
 */
// export function getPositionBelief(beliefSets: BeliefSets | undefined) {
//   if (!beliefSets) return undefined;
//   // Hardcoded offset
//   const offSet = {
//     lat: -30.4,
//     lng: 135.4,
//   };

//   const beliefs = new Map(beliefSets);

//   if (!beliefs.has(BELIEF_SET_KEYS.POSITION)) {
//     return undefined;
//   }

//   const positionBeliefs: BeliefMap = new Map(
//     beliefs.get(BELIEF_SET_KEYS.POSITION) as BeliefArray[]
//   );

//   const lat = positionBeliefs.get('positiony') / 100 + offSet.lat;
//   const lng = positionBeliefs.get('positionx') / 100 + offSet.lng;

//   return new LatLng(lat, lng);
// }

/**
 * Find point of interest using latlng position. Returns undefined if not found
 * @param pointsOfInterest array of PointOfInterest
 * @param position L.LatLng
 * @returns point of interest or undefined
 */
// export const findPoIByPosition = (
//   pointsOfInterest: PointOfInterestType[],
//   position: LatLng
// ) => {
//   const key = `${position.lat}, ${position.lng}`;
//   const poi = pointsOfInterest.find(
//     (x) => `${x.position.lat}, ${x.position.lng}` === key
//   );
//   return poi;
// };

function pad(num: number) {
  return num < 10 ? `0${num.toString()}` : num.toString();
}

function ddToDms(coordinate: number, posSymbol: string, negSymbol: string) {
  const dd = Math.abs(coordinate);
  const d = Math.floor(dd);
  const m = Math.floor((dd - d) * 60);
  const s = Math.round((dd - d - m / 60) * 3600 * 100) / 100;
  const directionSymbol = dd === coordinate ? posSymbol : negSymbol;
  return `${pad(d)}&deg; ${pad(m)}' ${pad(s)}" ${directionSymbol}`;
}

/**
 * Calculate measurements for an array of points
 * @param latlngs L.latlng
 * @returns an object with calculated area, length...
 */
export default function calc(latlngs: [number, number][]) {
  const last = latlngs[latlngs.length - 1];
  const path = latlngs.map((latlng) => new LatLng(latlng[0], latlng[1]));

  const polyline = L.polyline(path);
  const polygon = L.polygon(path);
  const meters = turf.length(polyline.toGeoJSON(), { units: 'kilometers' }) * 1000;
  const sqMeters = turf.area(polygon.toGeoJSON());

  return {
    lastCoord: {
      dd: {
        x: last[1],
        y: last[0],
      },
      dms: {
        x: ddToDms(last[1], 'E', 'W'),
        y: ddToDms(last[0], 'N', 'S'),
      },
    },
    length: meters,
    area: sqMeters,
  };
}

/**
 * convert timestamp to date (year+month+day+hour+minute+second)
 * @param unix_timestamp timestamp
 * @returns full date string (year+month+day+hour+minute+second)
 */
export function convertTimestampToDate(unix_timestamp: number) {
  const date = new Date(unix_timestamp / 1000);
  const years = date.getFullYear();
  const months = `0${date.getMonth() + 1}`;
  const days = `0${date.getDay()}`;
  const hours = date.getHours();
  const minutes = `0${date.getMinutes()}`;
  const seconds = `0${date.getSeconds()}`;

  const formattedDate = `${years}-${months.slice(-2)}-${days.slice(-2)} ${hours}:${minutes.slice(-2)}:${seconds.slice(-2)}`;

  return formattedDate;
}

export function formatMicrosecondOffset(microseconds: number) {
  const milliseconds = Math.floor(microseconds / 1000);
  const seconds = Math.floor(milliseconds / 1000);
  const minutes = Math.floor(seconds / 60);

  const remainingMilliseconds = milliseconds % 1000;
  const remainingSeconds = seconds % 60;

  const formattedOffset = `${minutes} minutes ${remainingSeconds} seconds ${remainingMilliseconds} milliseconds`;

  return formattedOffset;
}

/**
 * Convert seconds to mm:ss
 * @param seconds
 * @param doubleDigit (Optional) Default is true
 * @returns
 */
export function secondsToMinutes(seconds: number, doubleDigit = true): string {
  const minutes = Math.floor(seconds / 60);
  const remainingSeconds = seconds % 60;

  const minutesString = minutes < 10 && doubleDigit ? `0${minutes}` : `${minutes}`;

  const secondsString = remainingSeconds < 10 ? `0${remainingSeconds}` : `${remainingSeconds}`;

  return `${minutesString}:${secondsString}`;
}

/**
 * An util to convert miliseconds into HH:MM:SS format
 * @param milliseconds
 * @returns
 */
export function millisecondsToHHMMSS(milliseconds: number) {
  // Convert milliseconds to seconds
  const totalSeconds = Math.floor(milliseconds / 1000);

  // Calculate hours, minutes, and seconds
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;

  // Format the result
  const formattedTime = `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;

  return formattedTime;
}

/**
 * Takes two GeoJSON polygons and return the intersect polygon. If none exist, returns null.
 */
export function getIntersectPolygon(polygon1: GeoJSON.Polygon, polygon2: GeoJSON.Polygon) {
  const turfPolygon1 = turf.polygon(polygon1.coordinates);
  const turfPolygon2 = turf.polygon(polygon2.coordinates);
  return turf.intersect(turf.featureCollection([turfPolygon1, turfPolygon2]));
}
