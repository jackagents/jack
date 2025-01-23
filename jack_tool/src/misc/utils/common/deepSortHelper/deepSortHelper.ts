export const deepSortObjectKeys = (obj: any): any => {
  if (typeof obj !== 'object' || obj === null) {
    return obj;
  }

  if (Array.isArray(obj)) {
    return obj.map(deepSortObjectKeys);
  }

  const sortedKeys = Object.keys(obj).sort();
  const sortedObj: any = {};

  sortedKeys.forEach((key) => {
    sortedObj[key] = deepSortObjectKeys(obj[key]);
  });

  return sortedObj;
};
