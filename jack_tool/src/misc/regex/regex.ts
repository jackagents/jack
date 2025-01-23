// Negative and Posivite float 3 digits, 15 float
export const LatLngFloatStringReg = new RegExp(/^-?\d{0,3}(\.\d{0,15})?$/);

// Match -90 to 90 string and allow floating point (for input 1 by 1 check)
export const LatFloatStringReg = new RegExp(
  /(?:(?=(^-?[9][0](\.[0]{0,15})?$)|(^-?[0-8][0-9](\.\d{0,15})?$)|(^-?\d(\.\d{0,15})?$)|(^-?$)))/
);

// Match -180 to 180 string and allow floating point (for input 1 by 1 check)
export const LngFloatStringReg = new RegExp(
  /(?:(?=(^-?[1][8][0](\.[0]{0,15})?$)|(^-?[1][0-7][0-9](\.\d{0,15})?$)|(^-?\d{0,2}(\.\d{0,15})?$)|(^-?$)))/
);

// 3 Digits only
export const ThreeDigitsNumberReg = new RegExp(/^\d{0,3}$/);

// positive float only
export const PositiveFloatReg = new RegExp(/^\d{0,6}(\.\d{0,4})?$/);

// Float
export const FloatRegEx = new RegExp(/^-?\d{0,6}(\.\d{0,4})?$/);

export const F32RegEx = new RegExp(/^$|^-?\d{0,20}(\.\d{0,8})?$/);

// No white space and from 0 to 10 characters
export const NameReg = new RegExp(/^[^\s]{0,10}?$/);

export const String32Bit = new RegExp(/^[^\s]{0,32}?$/);
