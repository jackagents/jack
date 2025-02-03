// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

// Not in use
// import { EventType, CBDIEvent } from 'misc/types/cbdi/cbdiTypes';
// import { ParseData } from 'main/beBuilders/iwd/dataParser/ddsDataParser';

// class CBDIMessageBuilder {
//   /* -------------------------------------------------------------------------- */
//   /*                                 CONSTRUCTOR                                */
//   /* -------------------------------------------------------------------------- */

//   /* -------------------------------------------------------------------------- */
//   /*                                   PRIVATE                                  */
//   /* -------------------------------------------------------------------------- */

//   /* -------------------------------------------------------------------------- */
//   /*                                   PUBLIC                                   */
//   /* -------------------------------------------------------------------------- */

//   /**
//    * Parse the CBDI DDS data to object
//    * @param input RTI DDS Connector input
//    * @param data RTI DDS Connector data
//    * @returns
//    */
//   parseMessage = (data: string) => {
//     const jsonData = JSON.parse(data) as CBDIEvent;

//     const { body_type: bodyType } = jsonData;

//     let eventType = EventType.NONE;
//     // Assign event type
//     if (bodyType.toLowerCase().includes('delegation')) {
//       eventType = EventType.DELEGATION;
//     } else if (bodyType.toLowerCase().includes('pursue')) {
//       eventType = EventType.PURSUE;
//     } else if (bodyType.toLowerCase().includes('actionbegin')) {
//       eventType = EventType.ACTION_BEGIN;
//     } else if (bodyType.toLowerCase().includes('percept')) {
//       eventType = EventType.PERCEPT;
//     } else if (bodyType.toLowerCase().includes('control')) {
//       eventType = EventType.CONTROL;
//     } else if (bodyType.toLowerCase().includes('drop')) {
//       eventType = EventType.DROP;
//     } else if (bodyType.toLowerCase().includes('heartbeat')) {
//       eventType = EventType.HEARTBEAT;
//     } else if (bodyType.toLowerCase().includes('register')) {
//       eventType = EventType.REGISTER;
//     } else if (bodyType.toLowerCase().includes('deregister')) {
//       eventType = EventType.DEREGISTER;
//     } else if (bodyType.toLowerCase().includes('join')) {
//       eventType = EventType.AGENT_JOIN_TEAM;
//     } else if (bodyType.toLowerCase().includes('leave')) {
//       eventType = EventType.AGENT_LEAVE_TEAM;
//     } else if (bodyType.toLowerCase().includes('update')) {
//       eventType = EventType.ACTION_UPDATE;
//     } else if (bodyType.toLowerCase().includes('log')) {
//       eventType = EventType.BDI_LOG;
//     } else if (bodyType.toLowerCase().includes('message')) {
//       eventType = EventType.MESSAGE;
//     }

//     const parsedData = ParseData(eventType, jsonData);

//     const message = { eventType, data: parsedData };

//     return message;
//   };
// }

// const MSG_BUILDER = new CBDIMessageBuilder();
// export default MSG_BUILDER;
