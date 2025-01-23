import { useTheme } from '@mui/material';
import { ConnectionLineComponentProps, getBezierPath } from 'reactflow';

export default function FloatingConnectionLine({ toX, toY, fromPosition, toPosition, fromNode, fromX, fromY }: ConnectionLineComponentProps) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();

  if (!fromNode) {
    return null;
  }

  const [edgePath] = getBezierPath({
    sourceX: fromX,
    sourceY: fromY,
    sourcePosition: fromPosition,
    targetPosition: toPosition,
    targetX: toX,
    targetY: toY,
  });

  return (
    <g>
      <path fill="none" stroke={theme.editor.graphView.textColor} strokeWidth={1.5} className="animated" d={edgePath} />
      {/* Move the circle to the end of the path */}
      <circle cx={toX} cy={toY} fill="transparent" r={6} stroke="#222" strokeWidth={1.5} />
    </g>
  );
}
