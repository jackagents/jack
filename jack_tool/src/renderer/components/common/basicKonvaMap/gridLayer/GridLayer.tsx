import React from 'react';
import { Line, Layer } from 'react-konva';

interface Props {
  stageState: {
    stageScale: number;
    stageX: number;
    stageY: number;
  };
}

const WIDTH = 35;
const HEIGHT = 35;

function GridLayer({ stageState }: Props) {
  const [lines, setLines] = React.useState<JSX.Element[]>([]);

  const startX = React.useMemo(
    () =>
      Math.floor((-stageState.stageX - window.innerWidth) / WIDTH) *
      WIDTH *
      (1 / stageState.stageScale),
    [stageState]
  );

  const endX = React.useMemo(() => {
    return (
      Math.floor((-stageState.stageX + window.innerWidth * 2) / WIDTH) *
      WIDTH *
      (1 / stageState.stageScale)
    );
  }, [stageState]);

  const startY = React.useMemo(
    () =>
      Math.floor((-stageState.stageY - window.innerHeight) / HEIGHT) *
      HEIGHT *
      (1 / stageState.stageScale),
    [stageState]
  );

  const endY = React.useMemo(
    () =>
      Math.floor((-stageState.stageY + window.innerHeight * 2) / HEIGHT) *
      HEIGHT *
      (1 / stageState.stageScale),
    [stageState]
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
  }, [endX, endY, stageState, startX, startY]);

  return <Layer>{...lines}</Layer>;
}

export default React.memo(GridLayer);
