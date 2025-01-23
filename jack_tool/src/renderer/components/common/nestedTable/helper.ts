export const getUniqueKeys = (data: Record<string, any>, id: string | null = null): string[] => {
  const currentKey = id || 'root';
  const keys: string[] = [currentKey];

  if (typeof data === 'object' && !Array.isArray(data) && data !== null && data !== undefined) {
    Object.entries(data).forEach(([key, item]) => {
      const childKey = `${currentKey}/${key}`;
      keys.push(...getUniqueKeys(item, childKey));
    });
  } else if (Array.isArray(data)) {
    data.forEach((item, index) => {
      const childKey = `${currentKey}/${index}`;
      keys.push(...getUniqueKeys(item, childKey));
    });
  }

  return keys;
};
