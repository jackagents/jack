import pako from 'pako';

/**
 * Compress object into binary data
 * @param data object
 * @returns Uint8Array
 */
export const compressData = (data: Record<string, any>) => pako.deflate(JSON.stringify(data));

/**
 * Decompress binary data into string using pako lib
 * @param data Uint8Array
 * @returns string
 */
export const decompressData = (data: Uint8Array) => pako.inflate(data, { to: 'string' });
