// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

import { getPropValue, isProxyObj } from 'main/util';
import { Event as NonFlatBufferEvent } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

export abstract class BaseBuilder {
  protected _builderName: string;

  protected _playback: boolean;

  constructor(builderName: string, playback = false) {
    this._builderName = builderName;
    this._playback = playback;
  }

  /* -------------------------------------------------------------------------- */

  /**
   * Clear builder
   */
  abstract clear(): void;

  /**
   * Create snapshot of builder
   * @returns
   */
  abstract createSnapshot(): string;

  /**
   * Restore snapshot to builder
   * @param snapshotString
   */
  abstract restoreSnapshot(snapshotString: string): void;

  /**
   * Builder build
   * @param param0 CBDIMessage
   * @returns
   */
  buildNonFlatBuffer?(data: NonFlatBufferEvent): void;

  getBuilderName() {
    return this._builderName;
  }

  /**
   * Deep find the differences between 2 objects, if the value is array it will be replaced not merge
   * @param original Original object or old object
   * @param current Current object new object
   * @returns Object with differences between new and old objects
   */
  protected calculateDifferences(
    original: Record<string, any>,
    current: Record<string, any>
  ) {
    const originalTarget = isProxyObj(original) ? original.get() : original;
    const currentTarget = isProxyObj(current) ? current.get() : current;

    const differences: Record<string, any> = {};

    Object.keys(currentTarget).forEach((key) => {
      const currentProp = getPropValue(currentTarget);

      // original does not have key
      if (!originalTarget[key]) {
        differences[key] = current[key];
      }
      // original has key
      else {
        const originalProp = getPropValue(originalTarget[key]);

        // is not array
        if (typeof currentProp === 'object' && !Array.isArray(currentProp)) {
          // Recursively calculate differences for nested objects
          const nestedDifferences = this.calculateDifferences(
            originalProp,
            currentProp
          );

          // if nested has value
          if (Object.keys(nestedDifferences).length > 0) {
            differences[key] = nestedDifferences;
          }
        }
        // otherwise check if value has changed
        else if (JSON.stringify(currentProp) !== JSON.stringify(originalProp)) {
          differences[key] = currentProp;
        }
      }
    });

    return differences;
  }
}
