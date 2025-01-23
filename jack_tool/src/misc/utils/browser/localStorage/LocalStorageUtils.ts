export const storeValueToLocalStorage = (key: string, stringifiedValue: string) => {
  if (localStorage) {
    localStorage.setItem(key, stringifiedValue);
  } else {
    console.warn('localStorage is not supported');
    console.warn('Cannot store', key, 'to localStorage');
  }
  return stringifiedValue;
};

export const getValueFromLocalStorage = (key: string) => {
  if (localStorage) {
    return localStorage.getItem(key);
  }

  console.warn('localStorage is not supported');
  console.warn('Cannot get', key, 'from localStorage');
  return null;
};
