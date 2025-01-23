import { EdgeProps, getSmoothStepPath, BaseEdge, Position } from 'reactflow';

export const DelegationRoleEdge = ({
  id,
  sourceX,
  sourceY,
  targetX,
  targetY,
  sourcePosition,
  targetPosition,
  data,
}: EdgeProps) => {
  const [edgePath] = getSmoothStepPath({
    sourceX,
    sourceY,
    sourcePosition,
    targetX,
    targetY,
    targetPosition,
  });

  return (
    <BaseEdge
      id={id}
      path={edgePath}
      style={{
        stroke: data.delegationStatus === 'Pending' ? 'green' : 'gray',
        strokeWidth: 4,
      }}
    />
  );
};

export const NonDelegationRoleEdge = ({
  id,
  sourceX,
  sourceY,
  targetX,
  targetY,
}: EdgeProps) => {
  const edgePath = (() => {
    // Customize the turn position here
    const isSameX = sourceX === targetX;
    if (isSameX) {
      return `M${sourceX},${sourceY} L${targetX},${targetY}`;
    }
    const turnX = sourceX;
    const turnY = targetY - 40;

    return `M${sourceX},${sourceY} L${turnX},${turnY} L${targetX},${turnY} L${targetX},${targetY}`;
  })();

  return <BaseEdge id={id} path={edgePath} />;
};

export const HiddenEdge = (_data: EdgeProps) => {
  // This component won't render anything, effectively hiding the node.
  return null;
};
