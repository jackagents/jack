import React, { useEffect, useState, HTMLAttributes } from 'react';

export interface MouseTooltipProps extends HTMLAttributes<HTMLDivElement> {
  visible: boolean;
  offsetX: number;
  offsetY: number;
}

const MouseTooltip: React.FC<MouseTooltipProps> = ({
  children,
  className,
  visible,
  style,
  offsetX,
  offsetY,
  ...props
}) => {
  const [xPosition, setXPosition] = useState<number>(0);
  const [yPosition, setYPosition] = useState<number>(0);
  const [mouseMoved, setMouseMoved] = useState<boolean>(false);
  const [listenerActivate, setListenerActive] = useState<boolean>(false);

  const getTooltipPosition = ({ clientX, clientY }: MouseEvent) => {
    setXPosition(clientX);
    setYPosition(clientY);
    setMouseMoved(true);
  };

  const addListener = () => {
    window.addEventListener('mousemove', getTooltipPosition);
    setListenerActive(true);
  };

  const removeListener = () => {
    window.removeEventListener('mousemove', getTooltipPosition);
    setListenerActive(false);
  };

  useEffect(() => {
    if (visible && !listenerActivate) {
      addListener();
    } else if (!visible && listenerActivate) {
      removeListener();
    }

    return () => {
      removeListener();
    };
  }, [visible]);

  return (
    <div
      className={className}
      style={{
        display: visible && mouseMoved ? 'block' : 'none',
        position: 'fixed',
        top: yPosition + offsetY,
        left: xPosition + offsetX,
        ...style,
      }}
      // eslint-disable-next-line react/jsx-props-no-spreading
      {...props}
    >
      {children}
    </div>
  );
};

export default MouseTooltip;
