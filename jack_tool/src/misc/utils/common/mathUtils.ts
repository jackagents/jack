export const rad2Deg = (radians: number) => {
  const pi = Math.PI;
  return radians * (180 / pi);
};

export const hexToRgba = (hex: string, alpha: number): string => {
  const newHex = hex.replace('#', '');

  const red = parseInt(newHex.substring(0, 2), 16);
  const green = parseInt(newHex.substring(2, 4), 16);
  const blue = parseInt(newHex.substring(4, 6), 16);

  return `rgba(${red}, ${green}, ${blue}, ${alpha})`;
};
