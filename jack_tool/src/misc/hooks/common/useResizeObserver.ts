import React from 'react';

/**
 * Resize observer hook.
 * @returns
 */
const useResizeObserver = (): [
  React.RefObject<HTMLDivElement>,
  { width: number; height: number }
] => {
  const observeResizeRef = React.useRef<HTMLDivElement>(null);
  const [dimensions, setDimensions] = React.useState({
    width: 0,
    height: 0,
  });

  React.useEffect(() => {
    const observeTarget = observeResizeRef.current;
    const resizeObserver = new ResizeObserver((entries) => {
      entries.forEach((entry) => {
        if (entry.target.id === observeResizeRef.current?.id) {
          setDimensions({
            width: entry.contentRect.width,
            height: entry.contentRect.height,
          });
        }
      });
    });

    if (observeTarget) {
      resizeObserver.observe(observeTarget);
    }

    return () => {
      if (observeTarget) {
        resizeObserver.unobserve(observeTarget);
      }
    };
  }, []);

  return [observeResizeRef, dimensions];
};

export default useResizeObserver;
