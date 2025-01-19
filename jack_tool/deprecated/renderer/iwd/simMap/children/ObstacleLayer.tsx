import { Image, Layer } from 'react-konva';
import imgs from 'misc/images';
import useImage from 'use-image';
import React from 'react';
import { iwdResponse } from 'projectEvents/iwd/iwdEvents';
import { IpcRendererEvent } from 'electron';
// import { EventType, CBDIEvent, Message } from 'types/cbdi/cbdiTypes';
import { Event } from 'types/cbdi/auto-generated-types';

interface ObstaclePos {
  x: number;
  y: number;
  id: string;
}

const OBSTACLE_W = 30;
const OBSTACLE_H = 30;

const obstacles: ObstaclePos[] = [];

function ObstacleLayer() {
  const [obstacleImg] = useImage(imgs.obstacleImg);

  const [drawableObs, setDrawableObs] = React.useState<ObstaclePos[]>([]);

  /**
   * callback on receive percept message
   */
  const onMessage = React.useCallback(
    (_e: IpcRendererEvent, strData: string) => {
      const parsed = JSON.parse(strData) as Event;

      const { body } = parsed;
      const { fields, schema } = body.message!;

      if (schema.toLowerCase() === 'obstaclebeliefs') {
        const pos = fields.find(
          (x: any) => x.name.toLowerCase() === 'position'
        );

        if (pos) {
          const uuid = pos.data as string;

          if (uuid === 'none') return;

          // Get position
          const position = uuid.split(':').map((x) => parseFloat(x));

          // Find obstacle index using id
          const oIndex = obstacles.findIndex((x) => x.id === uuid);

          // Add new obstacle
          if (oIndex < 0) {
            obstacles.push({
              id: uuid,
              x: position[0],
              y: position[1],
            });

            // change state used to draw obstacles
            setDrawableObs([...obstacles]);
          }
        }
      }
    },
    []
  );

  /**
   * Init at the start
   */
  React.useEffect(() => {
    window.ipcRenderer.on(iwdResponse.websocket.message, onMessage);

    return () => {
      obstacles.length = 0;

      window.ipcRenderer.removeListener(
        iwdResponse.websocket.message,
        onMessage
      );
    };
  }, [onMessage]);

  return (
    <Layer>
      {drawableObs.map((o) => {
        return (
          <Image
            key={o.id}
            image={obstacleImg}
            x={o.x}
            y={o.y}
            offsetX={OBSTACLE_W / 2}
            offsetY={OBSTACLE_H / 2}
            width={OBSTACLE_W}
            height={OBSTACLE_H}
          />
        );
      })}
    </Layer>
  );
}

export default React.memo(ObstacleLayer);
