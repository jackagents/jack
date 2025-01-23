interface Props {
  id?: string;
  path: string;
  markerEnd: string | undefined;
  style: React.CSSProperties | undefined;
  interactionWidth?: number;
  onClick?: (event: any) => void;
  className?: string;
}

function ClickableBaseEdge({ id, path, style, markerEnd, interactionWidth = 20, className, onClick }: Props) {
  return (
    <>
      <path
        id={id}
        style={style}
        d={path}
        fill="none"
        className={className ? `react-flow__edge-path ${className}` : 'react-flow__edge-path'}
        markerEnd={markerEnd}
      />
      {interactionWidth && (
        <path d={path} fill="none" strokeOpacity={0} strokeWidth={interactionWidth} className="react-flow__edge-interaction" onClick={onClick} />
      )}
    </>
  );
}

ClickableBaseEdge.displayName = 'BaseEdge';

export default ClickableBaseEdge;
