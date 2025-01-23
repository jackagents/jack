import { Event } from 'types/cbdi/cbdi-types-non-flatbuffer';
// import { Event } from 'misc/types/cbdi/cbdi-types-json';

export const groupByOneSecond = (events: Event[]) => {
  const interval = 1000000; // 1 second in microseconds

  // Assume array of events is in order
  const resultArray = [];
  let currentIntervalStart = events[0].timestampUs;
  let currentInterval: Event[] = [];

  events.forEach((obj) => {
    // Within 1 second
    if (obj.timestampUs < currentIntervalStart + interval) {
      currentInterval.push(obj);
    }
    // Next second
    else {
      resultArray.push(currentInterval);
      currentInterval = [obj];
      currentIntervalStart = obj.timestampUs;
    }
  });

  // Add the last interval
  if (currentInterval.length > 0) {
    resultArray.push(currentInterval);
  }

  return resultArray;
};

export const secondToMillisecond = (sec: number) => {
  return sec * 1000000;
};

export const secondToMicrosecond = (sec: number) => {
  return sec * 1000;
};

export const microsecondToMillisecond = (microsecond: number) => {
  return microsecond * 1000;
};
