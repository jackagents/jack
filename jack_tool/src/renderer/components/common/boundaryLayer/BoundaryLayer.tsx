import { Layer, Rect } from 'react-konva';

interface Props {
  width: number;
  height: number;
  bgColor?: string;
}

// Boundary layer
export default function BoundaryLayer({ width, height, bgColor = '#add8e6' }: Props) {
  return (
    <Layer id="boundary-konva-layer">
      <Rect width={width} height={height} fill={bgColor} />
    </Layer>
  );
}
