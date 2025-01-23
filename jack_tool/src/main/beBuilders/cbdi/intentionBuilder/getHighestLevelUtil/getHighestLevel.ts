import { BDILogLevel } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

export const getLevelValue = (level: BDILogLevel) => {
  switch (level) {
    case BDILogLevel.NORMAL:
      return 0;
    case BDILogLevel.IMPORTANT:
      return 1;
    default:
      return 2;
  }
};

const getLevelEnum = (value: number) => {
  switch (value) {
    case 0:
      return BDILogLevel.NORMAL;
    case 1:
      return BDILogLevel.IMPORTANT;
    default:
      return BDILogLevel.CRITICAL;
  }
};

export const getHighestLevel = (level1: BDILogLevel, level2: BDILogLevel) => {
  const v1 = getLevelValue(level1);
  const v2 = getLevelValue(level2);

  return getLevelEnum(Math.max(v1, v2));
};
