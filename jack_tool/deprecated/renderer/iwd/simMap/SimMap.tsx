import React from 'react';
import Konva from 'konva';
import './SimMap.css';
import { Stage as ReactKonvaStage } from 'react-konva';
import { KonvaAgent } from 'types/iwd/iwdTypes';
import { KonvaEventObject } from 'konva/lib/Node';
import {
  float2BytesArray,
  generatePursueManualMoveGoalMsg,
} from 'misc/utils/cbdi/cbdiUtils';
import { Stage } from 'konva/lib/Stage';
import { request } from 'projectEvents/common/cmEvents';
import AgentLayer from './children/AgentLayer';
import GridLayer from './children/GridLayer';
import ObstacleLayer from './children/ObstacleLayer';

interface SizeProp {
  w: number;
  h: number;
}

function SimMap() {
  // Canvas size
  const [size, setSize] = React.useState<SizeProp>({ w: 0, h: 0 });

  // Selected konva agent
  const [selectedKonvaAgent, setSelectedKonvaAgent] =
    React.useState<KonvaAgent | null>(null);

  // Konva stage state
  const [stageState, setStageState] = React.useState({
    stageScale: 1,
    stageX: 0,
    stageY: 0,
  });

  const stageRef = React.useRef<Stage | null>(null);

  /**
   * Callback on wheel in konva canvas
   */
  const zoomStage = React.useCallback(
    (event: Konva.KonvaEventObject<WheelEvent>) => {
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

        const newScale =
          event.evt.deltaY < 0 ? oldScale * scaleBy : oldScale / scaleBy;

        setStageState({
          stageScale: newScale,
          stageX: (pPos.x / newScale - mouseePointTo.x) * newScale,
          stageY: (pPos.y / newScale - mouseePointTo.y) * newScale,
        });
      }
    },
    []
  );

  /**
   * Callback on window resized
   */
  const onWindowResize = () => {
    // Get content-viewport element
    const simMap = document.getElementById('content-viewport');

    // Set new size
    if (simMap) {
      setSize({ w: simMap.clientWidth, h: simMap.clientHeight });
    }
  };

  /**
   * Handle click on stage callback
   */
  const handleClick = React.useCallback((e: KonvaEventObject<PointerEvent>) => {
    // Left click button
    if (e.evt.button === 0) {
      setSelectedKonvaAgent(null);
    }
  }, []);

  /**
   * Handle right click on stage callback
   */
  const handleContextMenuClick = React.useCallback(
    (e: KonvaEventObject<PointerEvent>) => {
      if (selectedKonvaAgent) {
        const pointerPos = e.currentTarget.getRelativePointerPosition();
        const convertedPosX = float2BytesArray(pointerPos.x);
        const convertedPosY = float2BytesArray(pointerPos.y);

        const msg = generatePursueManualMoveGoalMsg(selectedKonvaAgent, {
          x: convertedPosX,
          y: convertedPosY,
        });

        const strMess = JSON.stringify(msg);

        window.ipcRenderer.send(request.websocket.send, strMess);
      }
    },
    [selectedKonvaAgent]
  );

  React.useEffect(() => {
    onWindowResize();

    window.addEventListener('resize', onWindowResize);

    return () => {
      window.removeEventListener('resize', onWindowResize);
    };
  }, []);

  return (
    <ReactKonvaStage
      className="konva-canvas"
      ref={stageRef}
      width={size.w}
      height={size.h}
      draggable
      onWheel={zoomStage}
      x={stageState.stageX}
      y={stageState.stageY}
      scale={{ x: stageState.stageScale, y: stageState.stageScale }}
      onClick={handleClick}
      onContextMenu={handleContextMenuClick}
      onDragEnd={(e) => {
        // e.evt.preventDefault();
        // setStageState({
        //   ...stageState,
        //   stageX: e.currentTarget.position().x,
        //   stageY: e.currentTarget.position().y,
        // });
      }}
    >
      <GridLayer stagePos={{ x: stageState.stageX, y: stageState.stageY }} />
      <ObstacleLayer />
      <AgentLayer
        selectedKonvaAgent={selectedKonvaAgent}
        setSelectedKonvaAgent={setSelectedKonvaAgent}
      />
    </ReactKonvaStage>
  );
}

export default React.memo(SimMap);
