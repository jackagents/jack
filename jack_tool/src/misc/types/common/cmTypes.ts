import { PropsWithChildren } from 'react';

/**
 * Util type to create Deep Partial type
 */
export type DeepPartial<T> = {
  [P in keyof T]?: T[P] extends object ? DeepPartial<T[P]> : T[P];
};

export default interface MenuAction {
  label?: string;
  submenu?: MenuAction[];
  accelerator?: string;
  disabled?: boolean;
  onClick?: () => void;
}

/**
 * Common interface props across application
 */
export interface InterfaceProps extends PropsWithChildren {
  scaleFactor?: number;
}

/* ************************************************************************************************
 * Vector2
 * *********************************************************************************************** */
export class Vector2 {
  /* ********************************************************************************************
   * Properties
   * ******************************************************************************************* */
  x: number;

  y: number;

  /* ********************************************************************************************
   * Constructor
   * ******************************************************************************************* */
  constructor(x = 0, y = 0) {
    this.x = x;
    this.y = y;
  }

  /* ********************************************************************************************
   * Functions
   * ******************************************************************************************* */
  add(other: Vector2) {
    return new Vector2(this.x + other.x, this.y + other.y);
  }
}

export type ThemeType = 'Dark' | 'Light';

export type Point2 = {
  x: number;
  y: number;
};

export type Point3 = {
  x: number;
  y: number;
  z: number;
};

/**
 * Positional coordinate tuple.
 */
export type Coordinate = [number, number] | [number, number, number];

// export type Coordinate3 = [number, number, number];

export type Line = Coordinate[]; // Line is similar to polygon but does not have a closed coords

export type Bound = Coordinate[]; // Bound is similar to line but the first and the last coords are similar to make a closed polygon/boundary

/**
 * It is actually just a GeoJSON Polygon, the first element is main polygon, and others are holes in that polygon.
 */
export type GeospatialPolygon = Bound[];
