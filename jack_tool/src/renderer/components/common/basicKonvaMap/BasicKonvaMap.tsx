import React from 'react';
import Konva from 'konva';
import { Stage as ReactKonvaStage } from 'react-konva';
import { Stage } from 'konva/lib/Stage';
import './BasicKonvaMap.css';
import { Vector2d } from 'konva/lib/types';
import { KonvaEventObject } from 'konva/lib/Node';
import GridLayer from './gridLayer/GridLayer';

interface SizeProp {
  w: number;
  h: number;
}

interface BasicKonvaMapProps extends React.PropsWithChildren {
  rootRef: React.RefObject<HTMLDivElement>;
  grid?: boolean;
  draggable?: boolean;
  onClick?: (pointerPos: Vector2d | null) => void;
  onMouseDown?: (evt: KonvaEventObject<MouseEvent>) => void;
  onMouseUp?: (evt: KonvaEventObject<MouseEvent>) => void;
  onMouseMove?: (evt: KonvaEventObject<MouseEvent>) => void;
  onMouseOver?: (evt: KonvaEventObject<MouseEvent>) => void;
  onMouseLeave?: (evt: KonvaEventObject<MouseEvent>) => void;
}

const MIN_SCALE = 0.5;
const MAX_SCALE = 2;

function BasicKonvaMap({
  rootRef,
  children,
  grid = false,
  draggable = false,
  onClick,
  onMouseDown,
  onMouseMove,
  onMouseUp,
  onMouseLeave,
  onMouseOver,
}: BasicKonvaMapProps) {
  const stageRef = React.useRef<Stage>(null);

  // Canvas size
  const [size, setSize] = React.useState<SizeProp>({ w: 0, h: 0 });

  // Konva stage state
  const [stageState, setStageState] = React.useState({
    stageScale: 1,
    stageX: 0,
    stageY: 0,
  });

  /**
   * Callback on wheel in konva canvas
   */
  const zoomStage = React.useCallback((event: Konva.KonvaEventObject<WheelEvent>) => {
    event.evt.preventDefault();

    const stage = event.target.getStage();
    if (!stage) {
      // console.log('stage not exist');
      return;
    }

    const oldScale = stage.scaleX();
    const scaleBy = 1.05;
    const pPos = stage.getPointerPosition();

    if (pPos) {
      const mouseePointTo = {
        x: pPos.x / oldScale - stage.x() / oldScale,
        y: pPos.y / oldScale - stage.y() / oldScale,
      };

      const newScale = event.evt.deltaY < 0 ? oldScale * scaleBy : oldScale / scaleBy;

      if (newScale < MIN_SCALE || newScale > MAX_SCALE) {
        return;
      }

      setStageState({
        stageScale: newScale,
        stageX: (pPos.x / newScale - mouseePointTo.x) * newScale,
        stageY: (pPos.y / newScale - mouseePointTo.y) * newScale,
      });
    }
  }, []);

  const handleDragEnd = React.useCallback(
    (e: Konva.KonvaEventObject<DragEvent>) => {
      setStageState({
        ...stageState,
        stageX: e.currentTarget.position().x,
        stageY: e.currentTarget.position().y,
      });
    },
    [stageState],
  );

  // Resize observer on parent div
  React.useEffect(() => {
    const resizeObserver = new ResizeObserver((entries) => {
      const r = entries.find((e) => e.target.id === rootRef.current?.id);
      if (r) {
        setSize({ w: r.contentRect.width, h: r.contentRect.height });
      }
    });

    if (rootRef.current) {
      resizeObserver.observe(rootRef.current);
    }

    return () => {
      if (rootRef.current) {
        resizeObserver.unobserve(rootRef.current);
      }
    };
  }, [rootRef]);

  /* --------------------------------- Render --------------------------------- */
  return (
    // Prevent implementing size 0 canvas which will crash
    (size.w > 0 && size.h > 0 && (
      <ReactKonvaStage
        id="basic-konva-canvas-stage"
        className="basic-konva-canvas"
        ref={stageRef}
        width={size.w}
        height={size.h}
        draggable={draggable}
        onWheel={zoomStage}
        x={stageState.stageX}
        y={stageState.stageY}
        scale={{ x: stageState.stageScale, y: stageState.stageScale }}
        onDragEnd={handleDragEnd}
        onMouseDown={onMouseDown}
        onMouseMove={onMouseMove}
        onMouseUp={onMouseUp}
        onMouseLeave={onMouseLeave}
        onMouseOver={onMouseOver}
        onClick={() => {
          if (stageRef.current && onClick) {
            onClick(stageRef.current.getRelativePointerPosition());
          }
        }}
      >
        {grid && <GridLayer stageState={stageState} />}

        {children}
      </ReactKonvaStage>
    )) ||
    null
  );
}

export default React.memo(BasicKonvaMap);
