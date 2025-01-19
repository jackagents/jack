import { EdgeBezierControlPoint } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Position } from 'reactflow';

interface GetBezierPathParams {
  sourceX: number;
  sourceY: number;
  sourcePosition?: Position;
  targetX: number;
  targetY: number;
  targetPosition?: Position;
  curvaturePos?: 'source' | 'target' | 'both' | 'none';
  curvature?: number;
}

export function getBezierEdgeCenter({
  sourceX,
  sourceY,
  targetX,
  targetY,
  sourceControlX,
  sourceControlY,
  targetControlX,
  targetControlY,
}: {
  sourceX: number;
  sourceY: number;
  targetX: number;
  targetY: number;
  sourceControlX: number;
  sourceControlY: number;
  targetControlX: number;
  targetControlY: number;
}): [number, number, number, number] {
  // cubic bezier t=0.5 mid point, not the actual mid point, but easy to calculate
  // https://stackoverflow.com/questions/67516101/how-to-find-distance-mid-point-of-bezier-curve
  const centerX =
    sourceX * 0.125 +
    sourceControlX * 0.375 +
    targetControlX * 0.375 +
    targetX * 0.125;
  const centerY =
    sourceY * 0.125 +
    sourceControlY * 0.375 +
    targetControlY * 0.375 +
    targetY * 0.125;
  const offsetX = Math.abs(centerX - sourceX);
  const offsetY = Math.abs(centerY - sourceY);

  return [centerX, centerY, offsetX, offsetY];
}

function calculateControlOffset(distance: number, curvature: number): number {
  if (distance >= 0) {
    return 0.5 * distance;
  }

  return curvature * 50 * Math.sqrt(-distance);
}

interface GetControlWithCurvatureParams {
  pos: Position;
  x1: number;
  y1: number;
  x2: number;
  y2: number;
  c: number;
}

function getControlWithCurvature({
  pos,
  x1,
  y1,
  x2,
  y2,
  c,
}: GetControlWithCurvatureParams): [number, number] {
  switch (pos) {
    case Position.Left:
      return [x1 - calculateControlOffset(x1 - x2, c), y1];
    case Position.Right:
      return [x1 + calculateControlOffset(x2 - x1, c), y1];
    case Position.Top:
      return [x1, y1 - calculateControlOffset(y1 - y2, c)];
    case Position.Bottom:
      return [x1, y1 + calculateControlOffset(y2 - y1, c)];
    default:
      return [x1 - calculateControlOffset(x1 - x2, c), y1];
  }
}

export function getCustomBezierPath({
  sourceX,
  sourceY,
  sourcePosition = Position.Bottom,
  targetX,
  targetY,
  targetPosition = Position.Top,
  curvaturePos = 'both',
  curvature = 0.25,
}: GetBezierPathParams): [
  path: string,
  labelX: number,
  labelY: number,
  offsetX: number,
  offsetY: number
] {
  let sourceCurvature = curvature;
  let targetCurvature = curvature;
  switch (curvaturePos) {
    case 'none':
      sourceCurvature = 0;
      targetCurvature = 0;
      break;
    case 'source':
      targetCurvature = 0;
      break;
    case 'target':
      sourceCurvature = 0;
      break;
    default:
      break;
  }

  const [sourceControlX, sourceControlY] = getControlWithCurvature({
    pos: sourcePosition,
    x1: sourceX,
    y1: sourceY,
    x2: targetX,
    y2: targetY,
    c: sourceCurvature,
  });
  const [targetControlX, targetControlY] = getControlWithCurvature({
    pos: targetPosition,
    x1: targetX,
    y1: targetY,
    x2: sourceX,
    y2: sourceY,
    c: targetCurvature,
  });
  const [labelX, labelY, offsetX, offsetY] = getBezierEdgeCenter({
    sourceX,
    sourceY,
    targetX,
    targetY,
    sourceControlX,
    sourceControlY,
    targetControlX,
    targetControlY,
  });

  return [
    `M${sourceX},${sourceY} C${sourceControlX},${sourceControlY} ${targetControlX},${targetControlY} ${targetX},${targetY}`,
    labelX,
    labelY,
    offsetX,
    offsetY,
  ];
}

function calculateMidpoint(
  sourceX: number,
  sourceY: number,
  controlPoint1X: number,
  controlPoint1Y: number,
  controlPoint2X: number,
  controlPoint2Y: number,
  targetX: number,
  targetY: number
) {
  const labelX =
    sourceX / 8 +
    (controlPoint1X / 8) * 3 +
    (controlPoint2X / 8) * 3 +
    targetX / 8;
  const labelY =
    sourceY / 8 +
    (controlPoint1Y / 8) * 3 +
    (controlPoint2Y / 8) * 3 +
    targetY / 8;

  return [labelX, labelY];
}

interface GetBezierPathWithControlPointsParams {
  sourceX: number;
  sourceY: number;
  targetX: number;
  targetY: number;
  controlPoints: EdgeBezierControlPoint[];
}

export function getBezierPathWithControlPoints({
  sourceX,
  sourceY,
  targetX,
  targetY,
  controlPoints,
}: GetBezierPathWithControlPointsParams) {
  // Start the path at the source point
  let path = `M ${sourceX} ${sourceY}`;
  let labelX = (sourceX + targetX) / 2;
  let labelY = (sourceY + targetY) / 2;
  // if no control points, create straight line from source to target
  if (controlPoints.length === 0) {
    path += `L ${targetX} ${targetY}`;
  } else if (controlPoints.length === 2) {
    const sourceControlPoint = controlPoints[0];
    const targetControlPoint = controlPoints[1];

    // calculate the absoulte position of source control point
    const sourceControlPointAbsoluteX = sourceX + sourceControlPoint.x;
    const sourceControlPointAbsoluteY = sourceY + sourceControlPoint.y;
    // calculate the absoulte position of target control point
    const targetControlPointAbsoluteX = targetX + targetControlPoint.x;
    const targetControlPointAbsoluteY = targetY + targetControlPoint.y;

    [labelX, labelY] = calculateMidpoint(
      sourceX,
      sourceY,
      sourceControlPointAbsoluteX,
      sourceControlPointAbsoluteY,
      targetControlPointAbsoluteX,
      targetControlPointAbsoluteY,
      targetX,
      targetY
    );

    path += ` C ${sourceControlPointAbsoluteX} ${sourceControlPointAbsoluteY}, ${targetControlPointAbsoluteX} ${targetControlPointAbsoluteY}, ${targetX} ${targetY}`;
  }

  return { path, labelX, labelY };
}
