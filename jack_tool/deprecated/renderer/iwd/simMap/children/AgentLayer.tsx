import { Image, Layer, Line } from 'react-konva';
import { KonvaEventObject } from 'konva/lib/Node';
import imgs from 'misc/images';
import useImage from 'use-image';
import React from 'react';
import { iwdResponse } from 'projectEvents/iwd/iwdEvents';
import { IpcRendererEvent } from 'electron';
import { KonvaAgent } from 'types/iwd/iwdTypes';
// import { EventType, CBDIEvent, Percept, Message } from 'types/cbdi/cbdiTypes';
import { Event } from 'types/cbdi/auto-generated-types';
import { Image as KonvaImage } from 'konva/lib/shapes/Image';
import * as mathUtils from 'misc/utils/common/mathUtils';
import { trimAnySpaceAndLowerCase } from 'misc/utils/common/commonUtils';

const AGENT_W = 30;
const AGENT_H = 30;
const ARROW_W = 15;
const ARROW_H = 15;

interface Props {
  setSelectedKonvaAgent: React.Dispatch<
    React.SetStateAction<KonvaAgent | null>
  >;
  selectedKonvaAgent: KonvaAgent | null;
}

const konvaAgents: KonvaAgent[] = [];

function AgentLayer({ selectedKonvaAgent, setSelectedKonvaAgent }: Props) {
  // Loaded images
  const [carImg] = useImage(imgs.carImg);
  const [downArrowImg] = useImage(imgs.downArrow);

  // Konva agent array to be drawn
  const [drawableAgents, setDrawableAgents] = React.useState<KonvaAgent[]>([]);

  // Arrow ref
  const arrowRef = React.useRef<KonvaImage | null>(null);

  /**
   * Rerender agents
   * @param index index of agent
   * @param konvaAgent konva agent obj
   */
  const rerenderAgents = (index: number, konvaAgent: KonvaAgent) => {
    // update agent
    konvaAgents[index] = konvaAgent;

    // change state used to draw agents
    setDrawableAgents([...konvaAgents]);
  };

  /**
   * callback on receive percept message
   */
  const onMessage = React.useCallback(
    (_e: IpcRendererEvent, strData: string) => {
      const parsed = JSON.parse(strData) as Event;

      const { body, recipient } = parsed;
      const { fields, schema } = body.message!;

      // Init konva agent object
      let konvaAgent: KonvaAgent = {
        x: 0,
        y: 0,
        rotation: 0,
        id: recipient.id,
        name: recipient.name,
        historyPath: [],
      };

      if (trimAnySpaceAndLowerCase(konvaAgent.name).includes('team')) {
        return;
      }

      // Find the index of agent using id
      let index = konvaAgents.findIndex((x) => x.id === recipient.id);

      // Get agent
      if (index >= 0) {
        konvaAgent = { ...konvaAgents[index] };
      }
      // Add new agent
      else {
        konvaAgents.push(konvaAgent);
        index = konvaAgents.length - 1;
      }

      switch (schema.toLowerCase()) {
        // Position beliefs
        case 'positionbeliefs': {
          // if (recipient.name.toLowerCase().includes('wombat')) {
          //   return;
          // }
          fields.forEach((field: any) => {
            switch (field.name.toLowerCase()) {
              case 'positionx':
                konvaAgent.x = field.data as number;
                break;
              case 'positiony':
                konvaAgent.y = field.data as number;
                break;
              default:
                break;
            }
          });

          rerenderAgents(index, konvaAgent);
          break;
        }
        // case 'stringifiedpositionbeliefs': {
        //   // parse the position
        //   const position = (fields[0].data as string)
        //     .split(':')
        //     .map((pos) => parseFloat(pos));

        //   // add history path for wombat
        //   if (
        //     recipient.name.toLowerCase().includes('wombat') &&
        //     konvaAgent.historyPath
        //   ) {
        //     konvaAgent.historyPath.push(...position);
        //   }

        //   const [x, y] = position;
        //   konvaAgent.x = x;
        //   konvaAgent.y = y;

        //   rerenderAgents(index, konvaAgent);

        //   break;
        // }
        // Patrol beliefs
        case 'patrolbeliefs': {
          fields.forEach((field: any) => {
            switch (field.name.toLowerCase()) {
              case 'patrolx':
                konvaAgent.patrolX = field.data;
                break;
              case 'patroly':
                konvaAgent.patrolY = field.data;
                break;
              case 'patrolling':
                if (!field.data) {
                  // reset history path
                  konvaAgent.historyPath.length = 0;
                  konvaAgent.patrolX = undefined;
                  konvaAgent.patrolY = undefined;
                }
                break;
              default:
                break;
            }
          });

          // update agent
          rerenderAgents(index, konvaAgent);
          break;
        }
        case 'rotationbeliefs': {
          if (fields[0].name.toLowerCase() === 'rotationradian') {
            const radian = fields[0].data as number;
            konvaAgent.rotation = mathUtils.rad2Deg(radian);
            rerenderAgents(index, konvaAgent);
          }
          break;
        }
        default:
          break;
      }
    },
    []
  );

  // Not in use currently
  // const onControlMessage = React.useCallback(
  //   (_event: IpcRendererEvent, strData: string) => {
  //     const parsed = JSON.parse(strData) as {
  //       eventType: EventType;
  //       data: CBDIEvent;
  //     };

  //     const { data } = parsed;
  //     const body = body as Control;

  //     const index = konvaAgents.findIndex((x) => x.id === recipient.id);

  //     if (index > -1) {
  // const agent = konvaAgents[index];
  // agent.historyPath = [];
  // const agents = [
  //   ...konvaAgents.slice(0, index),
  //   agent,
  //   ...konvaAgents.slice(index + 1, konvaAgents.length - 1),
  // ];
  // setDrawableAgents([...agents]);
  //     }
  //   },
  //   []
  // );

  /**
   * Handle click agent
   */
  const handleClickKonva = React.useCallback(
    (e: KonvaEventObject<MouseEvent>, a: KonvaAgent) => {
      if (setSelectedKonvaAgent) {
        e.cancelBubble = true;

        setSelectedKonvaAgent(a);
      }
    },
    [setSelectedKonvaAgent]
  );

  /**
   * Init at the start
   */
  React.useEffect(() => {
    window.ipcRenderer.on(iwdResponse.websocket.message, onMessage);
    // window.ipcRenderer.on(iwdResponse.websocket.control, onControlMessage);

    return () => {
      konvaAgents.length = 0;

      window.ipcRenderer.removeListener(
        iwdResponse.websocket.message,
        onMessage
      );
    };
  }, [onMessage]);

  // Animation for arrow - not in use currently
  // React.useEffect(() => {
  //   if (!selectedKonvaAgent || !arrowRef.current) {
  //     return;
  //   }

  //   const anim = new Animation((frame) => {
  //     if (arrowRef.current && frame)
  //       arrowRef.current.scaleX(
  //         (Math.sin(frame.time / ANIMATION_PERIOD) + 1) / 2
  //       );
  //   }, arrowRef.current.getLayer());

  //   anim.start();

  //   // eslint-disable-next-line consistent-return
  //   return () => {
  //     anim.stop();
  //   };
  // }, [selectedKonvaAgent, arrowRef]);

  return (
    <Layer>
      {drawableAgents.map((a) => {
        return (
          <React.Fragment key={a.id}>
            {selectedKonvaAgent && selectedKonvaAgent.id === a.id && (
              <>
                {/* Patrol line */}
                {a.patrolX && a.patrolY && (
                  <Line
                    strokeWidth={1}
                    points={[a.x, a.y, a.patrolX, a.patrolY]}
                    stroke="red"
                  />
                )}

                {/* History paths */}
                {a.historyPath.length > 0 && (
                  <Line
                    strokeWidth={1}
                    points={a.historyPath}
                    stroke="#03fc03"
                  />
                )}
                {/* Selected arrow */}
                <Image
                  ref={arrowRef}
                  key={`selected-icon-${a.id}`}
                  image={downArrowImg}
                  x={a.x}
                  y={a.y - AGENT_H / 2 - 10}
                  offsetX={ARROW_W / 2}
                  offsetY={ARROW_H / 2}
                  width={ARROW_W}
                  height={ARROW_H}
                />
              </>
            )}

            {/* Agent representation */}
            <Image
              onClick={(e) => {
                handleClickKonva(e, a);
              }}
              key={a.id}
              image={carImg}
              x={a.x}
              y={a.y}
              offsetX={AGENT_W / 2}
              offsetY={AGENT_H / 2}
              width={AGENT_W}
              height={AGENT_H}
              rotation={a.rotation}
            />
          </React.Fragment>
        );
      })}
    </Layer>
  );
}

export default React.memo(AgentLayer);
