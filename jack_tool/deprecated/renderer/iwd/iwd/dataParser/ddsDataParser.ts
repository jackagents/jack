// import {
//   CBDIData,
//   BinTypes,
//   EventType,
//   CBDIEvent,
//   Delegation,
//   Pursue,
//   ActionBegin,
//   Percept,
// } from 'misc/types/cbdi/cbdiTypes';
import {
  Any,
  AnyType,
  EventType,
  Event,
  Delegation,
  Pursue,
  ActionBegin,
  Percept,
} from 'misc/types/cbdi/auto-generated-types';
import { BinaryParser } from 'root/deprecated/renderer/iwd/iwd/dataParser/binaryParser';

const parseCBDIData = (element: Any) => {
  const cbdiData = element;
  switch (element.type) {
    // OTHER
    case AnyType.AnyType_OTHER: {
      break;
    }

    // All type except string and other
    default: {
      // if (element.type === 'STRING') console.log('bin', cbdiData.data);
      const bin: number[] = [];
      element.data.forEach((str: string) => {
        // let code = 0;
        // // if str exist, find the code from str using cp1252 encoding
        // if (str.length > 0) {
        //   [code] = cp1252.encode(str);
        // }

        // bin.push(code);
        bin.push(parseInt(str, 10));
      });
      cbdiData.data = BinaryParser(bin, element.type);
      break;
    }
  }

  return cbdiData;
};

const parseParams = (parameters: Any | Any[]) => {
  let params = parameters;
  if (Array.isArray(params)) {
    for (let i = 0; i < params.length; i += 1) {
      const element = params[i];
      params[i] = parseCBDIData(element);
    }
  } else {
    params = parseCBDIData(params);
  }

  return params;
};

export const ParseData = (eventType: EventType, jsonData: Event) => {
  const data = jsonData;
  const { body } = jsonData;
  let binData: Any[] | Any | null = null;
  switch (eventType) {
    case EventType.EventType_DELEGATION: {
      binData = (body as Delegation).parameters;
      (body as Delegation).parameters = parseParams(binData) as Any[];
      break;
    }
    case EventType.EventType_PURSUE: {
      binData = (body as Pursue).parameters;
      (body as Pursue).parameters = parseParams(binData) as Any[];
      break;
    }
    case EventType.EventType_ACTION_BEGIN: {
      binData = (body as ActionBegin).parameters;
      (body as ActionBegin).parameters = parseParams(binData) as Any[];
      break;
    }
    case EventType.EventType_PERCEPT: {
      binData = (body as Percept).value;
      (body as Percept).value = parseParams(binData) as Any;
      break;
    }
    case EventType.EventType_CONTROL:
    case EventType.EventType_DROP:
    case EventType.EventType_HEARTBEAT:
    case EventType.EventType_REGISTER:
    case EventType.EventType_DEREGISTER:
    case EventType.EventType_AGENT_JOIN_TEAM:
    case EventType.EventType_AGENT_LEAVE_TEAM:
    case EventType.EventType_ACTION_UPDATE:
    default:
      break;
  }

  if (binData) {
    data.body = body;
  }

  return data;
};
// const parseCBDIData = (element: CBDIData) => {
//   const cbdiData = element;
//   switch (element.type) {
//     // OTHER
//     case BinTypes.OTHER: {
//       break;
//     }

//     // All type except string and other
//     default: {
//       // if (element.type === 'STRING') console.log('bin', cbdiData.data);
//       const bin: number[] = [];
//       element.data.forEach((str: string) => {
//         // let code = 0;
//         // // if str exist, find the code from str using cp1252 encoding
//         // if (str.length > 0) {
//         //   [code] = cp1252.encode(str);
//         // }

//         // bin.push(code);
//         bin.push(parseInt(str, 10));
//       });
//       cbdiData.data = BinaryParser(bin, element.type);
//       break;
//     }
//   }

//   return cbdiData;
// };

// const parseParams = (parameters: CBDIData | CBDIData[]) => {
//   let params = parameters;
//   if (Array.isArray(params)) {
//     for (let i = 0; i < params.length; i += 1) {
//       const element = params[i];
//       params[i] = parseCBDIData(element);
//     }
//   } else {
//     params = parseCBDIData(params);
//   }

//   return params;
// };

// export const ParseData = (eventType: EventType, jsonData: CBDIEvent) => {
//   const data = jsonData;
//   const { body } = jsonData;
//   let binData: CBDIData[] | CBDIData | null = null;
//   switch (eventType) {
//     case EventType.DELEGATION: {
//       binData = (body as Delegation).parameters;
//       (body as Delegation).parameters = parseParams(binData) as CBDIData[];
//       break;
//     }
//     case EventType.PURSUE: {
//       binData = (body as Pursue).parameters;
//       (body as Pursue).parameters = parseParams(binData) as CBDIData[];
//       break;
//     }
//     case EventType.ACTION_BEGIN: {
//       binData = (body as ActionBegin).parameters;
//       (body as ActionBegin).parameters = parseParams(binData) as CBDIData[];
//       break;
//     }
//     case EventType.PERCEPT: {
//       binData = (body as Percept).value;
//       (body as Percept).value = parseParams(binData) as CBDIData;
//       break;
//     }
//     case EventType.CONTROL:
//     case EventType.DROP:
//     case EventType.HEARTBEAT:
//     case EventType.REGISTER:
//     case EventType.DEREGISTER:
//     case EventType.AGENT_JOIN_TEAM:
//     case EventType.AGENT_LEAVE_TEAM:
//     case EventType.ACTION_UPDATE:
//     default:
//       break;
//   }

//   if (binData) {
//     data.body = body;
//   }

//   return data;
// };
