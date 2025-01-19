import React from 'react';
import { Line, Layer } from 'react-konva';

interface Props {
  stagePos: {
    x: number;
    y: number;
  };
}

const WIDTH = 35;
const HEIGHT = 35;

export default function GridLayer({ stagePos }: Props) {
  const [lines, setLines] = React.useState<JSX.Element[]>([]);

  const startX = React.useMemo(
    () => Math.floor((-stagePos.x - window.innerWidth) / WIDTH) * WIDTH,
    [stagePos]
  );

  const endX = React.useMemo(
    () => Math.floor((-stagePos.x + window.innerWidth * 2) / WIDTH) * WIDTH,
    [stagePos]
  );

  const startY = React.useMemo(
    () => Math.floor((-stagePos.y - window.innerHeight) / HEIGHT) * HEIGHT,
    [stagePos]
  );

  const endY = React.useMemo(
    () => Math.floor((-stagePos.y + window.innerHeight * 2) / HEIGHT) * HEIGHT,
    [stagePos]
  );

  React.useEffect(() => {
    const tempLines = [];

    for (let x = startX - WIDTH / 2; x < endX; x += WIDTH) {
      const line = (
        <Line
          key={`vertical:${x},${startY}:${x},${endY}`}
          stroke="black"
          strokeWidth={1}
          points={[x, startY - HEIGHT / 2, x, endY - HEIGHT / 2]}
        />
      );
      tempLines.push(line);
    }

    for (let y = startY - HEIGHT / 2; y < endY; y += HEIGHT) {
      const line = (
        <Line
          key={`horizontal:${startX},${y}:${endX},${y}`}
          stroke="black"
          strokeWidth={1}
          points={[startX - WIDTH / 2, y, endX - WIDTH / 2, y]}
        />
      );
      tempLines.push(line);
    }

    setLines([...tempLines]);
  }, [stagePos]);

  return <Layer>{...lines}</Layer>;
}
