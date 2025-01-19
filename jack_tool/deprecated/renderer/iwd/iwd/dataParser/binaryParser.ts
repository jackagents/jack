import { buf2hex, hex2Float, hex2a } from 'misc/utils/cbdi/cbdiUtils';
// import { BinTypes } from 'misc/types/cbdi/cbdiTypes';
import { AnyType } from 'misc/types/cbdi/auto-generated-types';

// const parser = new Parser().uint8('uint8').int32le("int32").bit2("bool");
export function BinaryParser(bin: number[], type: AnyType) {
  // create uint8array
  const arrayBuffer = new Uint8Array(bin);

  // bytes array to hex string
  const hexStr = buf2hex(arrayBuffer);

  // turn little endian into big endian and parse
  switch (type) {
    case AnyType.AnyType_I8:
    // Fallthrough
    case AnyType.AnyType_I16:
    // Fallthrough
    case AnyType.AnyType_I32:
    // Fallthrough
    case AnyType.AnyType_I64:
    // Fallthrough
    case AnyType.AnyType_U8:
    // Fallthrough
    case AnyType.AnyType_U16:
    // Fallthrough
    case AnyType.AnyType_U32:
    // Fallthrough
    case AnyType.AnyType_U64:
      return parseInt(`0x${hexStr.match(/../g)?.reverse().join('')}`, 10);

    case AnyType.AnyType_BOOL:
      return Boolean(parseInt(hexStr, 10));

    case AnyType.AnyType_F32:
    // Fallthrough
    case AnyType.AnyType_F64:
      return hex2Float(hexStr);

    case AnyType.AnyType_STRING:
      return hex2a(hexStr);

    case AnyType.AnyType_OTHER:
    // Fallthrough
    default:
      return null;
  }
}
// export function BinaryParser(bin: number[], type: BinTypes) {
//   // create uint8array
//   const arrayBuffer = new Uint8Array(bin);

//   // bytes array to hex string
//   const hexStr = buf2hex(arrayBuffer);

//   // turn little endian into big endian and parse
//   switch (type) {
//     case BinTypes.I8:
//     // Fallthrough
//     case BinTypes.I16:
//     // Fallthrough
//     case BinTypes.I32:
//     // Fallthrough
//     case BinTypes.I64:
//     // Fallthrough
//     case BinTypes.U8:
//     // Fallthrough
//     case BinTypes.U16:
//     // Fallthrough
//     case BinTypes.U32:
//     // Fallthrough
//     case BinTypes.U64:
//       return parseInt(`0x${hexStr.match(/../g)?.reverse().join('')}`, 10);

//     case BinTypes.BOOL:
//       return Boolean(parseInt(hexStr, 10));

//     case BinTypes.F32:
//     // Fallthrough
//     case BinTypes.F64:
//       return hex2Float(hexStr);

//     case BinTypes.STRING:
//       return hex2a(hexStr);

//     case BinTypes.OTHER:
//     // Fallthrough
//     default:
//       return null;
//   }
// }
