// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

import { URL } from 'url';
import path from 'path';
import { BDILogIntentionsModel } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { MAXIMUM_INTENTIONS } from 'constant/common/cmConstants';

/**
 * Boilerplate util
 * @param htmlFileName
 * @returns
 */
export function resolveHtmlPath(htmlFileName: string) {
  if (process.env.NODE_ENV === 'development') {
    const port = process.env.PORT || 1212;
    const url = new URL(`http://localhost:${port}`);
    url.pathname = htmlFileName;
    return url.href;
  }
  return `file://${path.resolve(__dirname, '../renderer/', htmlFileName)}`;
}

/**
 * Takes in a javascript object and return an array of functions name of the object
 * @param obj a js object
 * @returns list of functions name of the object
 */
export const getMethods = (obj: any) => {
  const properties = new Set();
  let currentObj = obj;
  do {
    Object.getOwnPropertyNames(currentObj).map((item: any) =>
      properties.add(item)
    );
    // eslint-disable-next-line no-cond-assign
  } while ((currentObj = Object.getPrototypeOf(currentObj)));
  return [...properties.keys()].filter(
    (item: any) => typeof obj[item] === 'function'
  );
};

/**
 * Compare the two arrays. Takes in two arrays and return an object of two arrays which contain the different elements
 * @param arr1 first array
 * @param arr2 second array
 * @returns
 */
export const getDiff = (arr1: any[], arr2: any[]) => {
  const diff1 = arr1.filter((item: any) => {
    return arr2.indexOf(item) === -1;
  });

  const diff2 = arr2.filter((item: any) => {
    return arr1.indexOf(item) === -1;
  });

  return { diff1, diff2 };
};

/**
 * Returns the current timestamp in string
 * @returns current timestamp in string
 */
export const getCurrentDateStr = () => {
  return Date.parse(new Date().toString()).toString();
};

export const filterIntentionModel = (
  model: BDILogIntentionsModel,
  max = MAXIMUM_INTENTIONS
) => {
  // Only return by max amount
  const keys = Object.keys(model).splice(0, max);

  const filtered: BDILogIntentionsModel = {};

  keys.forEach((key) => {
    filtered[key] = model[key];
  });

  return filtered;
};

export const isProxyObj = (obj: Record<string, any>) =>
  !!obj && typeof obj === 'object' && typeof obj.get === 'function';

export const getPropValue = (obj: Record<string, any>) =>
  isProxyObj(obj) ? obj.get() : obj;

export const formatDateToCustomString = (date: Date) => {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0');
  const day = String(date.getDate()).padStart(2, '0');
  const hours = String(date.getHours()).padStart(2, '0');
  const minutes = String(date.getMinutes()).padStart(2, '0');
  const seconds = String(date.getSeconds()).padStart(2, '0');

  return `${year}-${month}-${day}-${hours}-${minutes}-${seconds}`;
};
