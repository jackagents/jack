import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { EDITOR_OVERVIEW_MODULECONCEPT } from 'misc/constant/cbdiEdit/cbdiEditConstant';
import { rad2Deg } from './mathUtils';

export const getKeyByValue = (obj: any, value: any) => Object.keys(obj).find((x) => obj[x] === value);

export const trimSpace = (text: string) => text.trim().replace(/\s/g, '');

export const trimAnySpaceAndLowerCase = (text: string) => trimSpace(text).toLowerCase();

export const areObjectsEqual = <T extends Object | null | undefined>(obj1: T, obj2: T): boolean => {
  // Early return for null or undefined inputs
  if (obj1 === null || obj2 === null) return obj1 === obj2;
  if (obj1 === undefined && obj2 === undefined) return true;

  // If neither is null or undefined, compare their contents
  if (typeof obj1 !== 'object' || obj1 === null || typeof obj2 !== 'object' || obj2 === null) {
    return obj1 === obj2;
  }
  return JSON.stringify(obj1) === JSON.stringify(obj2);
};

/**
 * check if two module concept equal
 * @param moduleConcept1
 * @param moduleConcept2
 * @returns
 */
export const areModuleConceptsEqual = (
  moduleConcept1: ModuleConcept | undefined | null,
  moduleConcept2: ModuleConcept | undefined | null,
): boolean => {
  if (moduleConcept1 === undefined || moduleConcept1 === null || moduleConcept2 === undefined || moduleConcept2 === null) {
    if (moduleConcept1 === moduleConcept2) {
      return true;
    }
    return false;
  }

  if (moduleConcept1.module === moduleConcept2.module && moduleConcept1.uuid === moduleConcept2.uuid) {
    return true;
  }
  return false;
};

/**
 * check if module concept is overview concept
 */
export const isModuleConceptOverview = (moduleConcept: ModuleConcept | undefined | null) => {
  if (moduleConcept === undefined || moduleConcept === null) {
    return false;
  }
  return moduleConcept.uuid === EDITOR_OVERVIEW_MODULECONCEPT.uuid && moduleConcept.name === EDITOR_OVERVIEW_MODULECONCEPT.name;
};

/**
 * check if moduleConcept is a sub module node
 * @param moduleConcept
 * @returns
 */
export const isModuleConceptModule = (moduleConcept: ModuleConcept | undefined | null) => {
  if (moduleConcept && moduleConcept.module !== '' && moduleConcept.name === moduleConcept.module && moduleConcept.module === moduleConcept.uuid) {
    return true;
  }
  return false;
};

/**
 * Get size of string data in kilobytes
 * @param str string
 * @returns size in kb
 */
export const getStringSizeInKb = (str: string) => {
  const encoder = new TextEncoder();
  const encodedBytes = encoder.encode(str);
  const sizeInKB = encodedBytes.length / 1024;
  return sizeInKB;
};

/**
 * Copy object
 * @param originalObj
 * @returns
 */
export function copyObj<T>(originalObj: T): T {
  return JSON.parse(JSON.stringify(originalObj));
}

/**
 * Deep merge 2 objects with second object overwrites first object
 * @param a first object
 * @param b second object
 * @returns merged object with second object overwrite first object
 */
export const deepMerge = (a: Record<string, any>, b: Record<string, any>) => {
  const mergedObject = copyObj(a);

  Object.keys(b).forEach((key) => {
    if (typeof b[key] === 'object' && b[key] !== null && !Array.isArray(b[key])) {
      if (!(key in mergedObject) || typeof mergedObject[key] !== 'object' || mergedObject[key] === null) {
        mergedObject[key] = {};
      }
      mergedObject[key] = deepMerge(mergedObject[key], b[key]);
    } else {
      mergedObject[key] = b[key];
    }
  });

  return mergedObject;
};

export const roundToDecimalPlaces = (value: number, decimalPlaces: number): number => {
  const factor = 10 ** decimalPlaces;
  return Math.round(value * factor) / factor;
};

export const convertNestedArrayToObjects = (json: any): any => {
  if (typeof json === 'object' && json !== null && json !== undefined) {
    if (Array.isArray(json)) {
      const newObj: any = {};
      json.forEach((item, index) => {
        newObj[index] = convertNestedArrayToObjects(item);
      });
      return newObj;
    }
    const newObj: any = {};
    Object.keys(json).forEach((key) => {
      newObj[key] = convertNestedArrayToObjects(json[key]);
    });

    return newObj;
  }
  // if value is boolean type, stringfy it
  if (typeof json === 'boolean') {
    return json.toString();
  }
  return json;
};

/**
 * Calculate heading (deg) from xyzw
 * @param param0
 * @returns
 */
export function quaternion2euler({ x, y, z, w }: { x: number; y: number; z: number; w: number }) {
  const sinyCosp = 2 * (w * z + x * y);
  const cosyCosp = 1 - 2 * (y * y + z * z);
  const yaw = Math.atan2(sinyCosp, cosyCosp);
  let result = ((rad2Deg(yaw) - 90.0) * -1.0) % 360.0;
  while (result < 0) {
    result += 360;
  }
  return result; // ENU to Compass/NED limit +/- 360
}

/**
 * Get direction string ('N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW') from heading (deg)
 * @param heading
 * @returns
 */
export function getDirection(heading: number): string {
  const directions = ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW'];
  // Normalize the heading to be within 0 to 360 degrees
  let normalizedHeading = heading % 360;
  if (normalizedHeading < 0) {
    normalizedHeading += 360;
  }

  // Calculate the index by dividing the normalized heading by 45 (since there are 8 directions)
  // and rounding to the nearest integer.
  let index = Math.round(normalizedHeading / 45);

  // Ensure the index is within the range of 0 to 7 (since there are 8 directions)
  index %= 8;

  return directions[index];
}

/**
 * parese number string to float/int
 * @param value
 * @returns
 */
export const parseNumber = (value: string): number => {
  const floatValue = parseFloat(value);

  // Check if the value is NaN (not a number)
  if (Number.isNaN(floatValue)) {
    return 0; // or handle the error as needed
  }

  // Check if the value is an integer
  if (floatValue === Math.floor(floatValue)) {
    return floatValue; // Return as an integer
  }

  // If not an integer, return as a floating-point number
  return floatValue;
};

/**
 * Remove  everything before the first period ('.') and the period itself from the input string.
 */
export const removeBeforeFirstDotAndDot = (input: string | undefined): string | undefined => {
  // Check if the input is not undefined and is indeed a string
  // If the input is undefined, return undefined
  if (input === undefined) {
    return input;
  }

  const result = input.replace(/^.+?\./, '');

  // Return the modified string
  return result;
};
