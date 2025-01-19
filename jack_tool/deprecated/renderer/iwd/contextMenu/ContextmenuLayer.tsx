import { LatLng, LeafletMouseEvent, Point } from 'leaflet';
import React from 'react';
import { useMapEvents } from 'react-leaflet';
import { IpcRendererEvent } from 'electron';
import { useDispatch, useSelector } from 'react-redux';
import {
  BuildMode,
  Agent,
  CurrentAgent,
  CurrentSelectElement,
  SavedZone,
  Zone,
  SelectableElement,
} from 'types/iwd/iwdTypes';
import { v4 } from 'uuid';
import { request, response } from 'projectEvents/common/cmEvents';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { RootState } from 'projectRedux/Store';
import { ZONE_TYPE } from 'constant/common/cmConstants';
import { findPoIByPosition } from 'misc/utils/common/rendererUtils';
import { IwdVehicleModel } from 'types/iwd/iwdVehicleModel';
import { iwdRequest } from 'projectEvents/iwd/iwdEvents';
import { NodeType } from 'misc/types/cbdi/auto-generated-types';
import { ContextMenu } from './ContextMenu';

interface Cursor {
  containerPoint: Point;
  latlngPoint: LatLng;
}

enum Target {
  NONE,
  MAP_NONE_SELECTED,
  MAP_AGENT_SELECTED,
  FIRST_AGENT,
  SECOND_AGENT,
  POINT_OF_INTEREST,
  VEHICLE,
}

interface Props {
  agent: Agent | null;
  cursor: Cursor | null;
  target: Target;
  secondAgent: Agent | null;
  vehicle?: IwdVehicleModel;
}

const initState = {
  agent: null,
  secondAgent: null,
  cursor: null,
  target: Target.NONE,
};

const CtxMenu = React.memo(ContextMenu);

function ContextMenuLayer() {
  const [contextMenu, setContextMenu] = React.useState(false);
  const [state, setState] = React.useState<Props>({ ...initState });

  const { currentSelectElement, currentBuildMode, agents, pointsOfInterest } =
    useSelector((rootstate: RootState) => rootstate.iwd);

  const { closeConfigDisplay } = iwdActions;

  const dispatch = useDispatch();

  /**
   * Callback for right click on marker
   * @param _event
   * @param curCursorPos stringified cursor object
   * @param agentData (optional) stringified agent data
   * @returns
   */
  const onContextmenuOnAgent = (
    _event: IpcRendererEvent,
    curCursorPos: string,
    agentData?: string
  ) => {
    if (!agentData) return;

    const a = JSON.parse(agentData) as Agent;
    const m = JSON.parse(curCursorPos) as Cursor;

    if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.AGENT
    ) {
      // Second agent
      setState({
        ...state,
        secondAgent: { ...a },
        cursor: { ...m },
        target: Target.SECOND_AGENT,
      });

      return;
    }

    // First agent
    setState({
      agent: { ...a },
      secondAgent: null,
      cursor: { ...m },
      target: Target.FIRST_AGENT,
    });
  };

  /**
   * Callback for right click on zone
   * @param _event
   * @param _curCursorPos stringified cursor object
   * @param zoneData stringified zone data
   * @returns
   */
  const onContextmenuOnZone = (
    _event: IpcRendererEvent,
    _curCursorPos: string,
    zoneData?: string
  ) => {
    if (!zoneData) return;

    const z = JSON.parse(zoneData) as SavedZone;

    if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.ZONE
    ) {
      window.ipcRenderer.invoke(request.project.undoZone, JSON.stringify(z));
    }

    // TODO: select
  };

  /**
   * Callback for right click on vehicle
   * @param _event
   * @param curCursorPos stringified cursor object
   * @param zoneData stringified zone data
   * @returns
   */
  const onContextmenuOnVehicle = (
    _event: IpcRendererEvent,
    curCursorPos: string,
    vehicleData?: string
  ) => {
    if (!vehicleData) return;

    const v = JSON.parse(vehicleData) as IwdVehicleModel;

    setState({
      ...initState,
      cursor: JSON.parse(curCursorPos) as Cursor,
      target: Target.VEHICLE,
      vehicle: v,
    });
  };

  /**
   * Callback for right click on point of interest
   * @param _event
   * @param curCursorPos stringified cursor object
   */
  const onContextmenuOnPointOfInterest = (
    _event: IpcRendererEvent,
    curCursorPos: string
  ) => {
    const m = JSON.parse(curCursorPos) as Cursor;

    // contextmenu on point of interest
    setState({ ...initState, cursor: m, target: Target.POINT_OF_INTEREST });
  };

  const [currentMenu, setCurrentMenu] = React.useState<any[] | null>(null);

  /**
   * Callback when closing contextmenu
   */
  const handleClose = React.useCallback(() => {
    setState({ ...initState });
  }, []);

  // Handle create cbdi agent
  const handleCreate = (nodeType: NodeType) => {
    // TODO: Create team/agent at current position
    switch (nodeType) {
      case NodeType.NodeType_TEAM:
        break;
      case NodeType.NodeType_AGENT:
        break;
      default:
        break;
    }

    handleClose();
  };

  // Handle create zone
  /**
   * Callback when creating zone. Takes in a zone type and send create zone event to backend
   * @param zoneType type of the zone
   * @returns
   */
  const handleCreateZone = (zoneType: number) => {
    if (!state.cursor) return;

    const zone: Zone = {
      id: v4(),
      type: ZONE_TYPE.NONE,
      polygons: [[state.cursor.latlngPoint]],
      lastChanged: window.mainUtils.getCurrentDateStr(),
    };

    switch (zoneType) {
      case ZONE_TYPE.GEO_FENCE: {
        zone.type = ZONE_TYPE.GEO_FENCE;
        break;
      }
      case ZONE_TYPE.NO_GO: {
        zone.type = ZONE_TYPE.NO_GO;
        break;
      }
      default:
        break;
    }

    // Create zone
    window.ipcRenderer.invoke(request.project.createZone, JSON.stringify(zone));
  };

  /**
   * Call back when creating vehicle.
   * @returns
   */
  const handleCreateVehicle = () => {
    if (!state.cursor) return;

    // Create vehicle
    window.ipcRenderer.invoke(
      iwdRequest.vehicle.create,
      JSON.stringify(state.cursor.latlngPoint)
    );
  };

  // Common menu
  const commonMenu: any[] = [];

  /**
   * contextmenu on map (without any selected agent)
   */
  const mapCtxMenu = [
    {
      label: 'New',
      submenu: [
        {
          label: 'Zone',
          submenu: [
            {
              label: 'Geo-fence',
              onClick: () => {
                // Create Team
                handleCreateZone(ZONE_TYPE.GEO_FENCE);

                // Close context menu
                handleClose();
              },
            },
            {
              label: 'No-go',
              onClick: () => {
                // Create Agent
                handleCreateZone(ZONE_TYPE.NO_GO);

                // Close context menu
                handleClose();
              },
            },
          ],
        },
        {
          label: 'Vehicle',
          onClick: () => {
            // Create vehicle
            handleCreateVehicle();

            handleClose();
          },
        },
        // {
        //   label: 'CBDI',
        //   submenu: [
        //     {
        //       label: 'Team',
        //       onClick: () => {
        //         // Create Team
        //         handleCreate(NodeType.TEAM);

        //         // Close context menu
        //         handleClose();
        //       },
        //     },
        //     {
        //       label: 'Agent',
        //       onClick: () => {
        //         // Create Agent
        //         handleCreate(NodeType.AGENT);

        //         // Close context menu
        //         handleClose();
        //       },
        //     },
        //   ],
        // },
        {
          label: 'PoI',
          onClick: () => {
            if (!state.cursor) return;

            // Create point of interest
            window.ipcRenderer.invoke(
              request.project.createPointOfInterest,
              state.cursor.latlngPoint.lat,
              state.cursor.latlngPoint.lng
            );

            // Close context menu
            handleClose();
          },
        },
      ],
    },
  ];

  /**
   * context menu on agents
   */
  const propertiesMenu = [
    {
      label: 'Select',
      onClick: () => {
        if (!state || !state.agent) return;

        const payload: CurrentSelectElement = {
          type: SelectableElement.AGENT,
          value: { id: state.agent.address.id },
        };

        // Change current agent id
        dispatch(
          iwdActions.changeCurrentSelectElement(JSON.stringify(payload))
        );

        // Close context menu
        handleClose();
      },
    },
  ];

  /**
   * contextmenu on poi
   */
  const poiContextmenu = [
    {
      label: 'Select',
      onClick: () => {
        if (!state || !state.cursor) return;

        const poi = findPoIByPosition(
          pointsOfInterest,
          state.cursor.latlngPoint
        );

        if (!poi) {
          console.log('cannot find point of interest');
          return;
        }

        const payload: CurrentSelectElement = {
          type: SelectableElement.POI,
          value: poi,
        };

        dispatch(
          iwdActions.changeCurrentSelectElement(JSON.stringify(payload))
        );

        // Close context menu
        handleClose();
      },
    },
    {
      label: 'Remove',
      onClick: () => {
        if (!state || !state.cursor) return;

        const poi = findPoIByPosition(
          pointsOfInterest,
          state.cursor.latlngPoint
        );

        dispatch(iwdActions.removePointsOfInterest(JSON.stringify(poi)));

        // Close context menu
        handleClose();
      },
    },
  ];

  /**
   * contextmenu on map (with agent selected)
   */
  const agentActionMenu = [
    {
      label: 'Move here',
      onClick: () => {
        if (!state || !state.agent) return;

        // TODO: Move current agent to selected position

        // Close context menu
        handleClose();
      },
    },
  ];

  const interactMenu = [
    {
      label: 'Select',
      onClick: () => {
        if (!state || !state.secondAgent) return;

        const payload: CurrentSelectElement = {
          type: SelectableElement.AGENT,
          value: { id: state.secondAgent.address.id },
        };

        // Change current agent id
        dispatch(
          iwdActions.changeCurrentSelectElement(JSON.stringify(payload))
        );

        // Close context menu
        handleClose();
      },
    },
    {
      label: 'Interact',
      onClick: () => {
        if (!state || !state.secondAgent) return;

        // Interact menu

        // Close context menu
        handleClose();
      },
    },
  ];

  /**
   * Vehicle interact menu
   */
  const vehicleInteractMenu = [
    {
      label: 'Set base',
      onClick: () => {
        // Allow rebase target
        window.ipcRenderer.send(
          iwdRequest.vehicle.rebase,
          JSON.stringify(state.vehicle)
        );

        // Close context menu
        handleClose();

        // Close entity config window
        dispatch(closeConfigDisplay());
      },
    },
  ];

  // Set context menu content when state changed
  React.useEffect(() => {
    switch (state.target) {
      case Target.MAP_NONE_SELECTED:
        setCurrentMenu([...mapCtxMenu, ...commonMenu]);
        break;
      case Target.MAP_AGENT_SELECTED:
        setCurrentMenu([...agentActionMenu, ...commonMenu]);
        break;
      case Target.FIRST_AGENT:
        setCurrentMenu([...propertiesMenu, ...commonMenu]);
        break;
      case Target.SECOND_AGENT:
        setCurrentMenu([...interactMenu, ...commonMenu]);
        break;
      case Target.POINT_OF_INTEREST:
        setCurrentMenu([...poiContextmenu, ...commonMenu]);
        break;
      case Target.VEHICLE:
        setCurrentMenu([...vehicleInteractMenu, ...commonMenu]);
        break;
      default:
        setCurrentMenu(null);
        break;
    }
  }, [state]);

  React.useEffect(() => {
    window.ipcRenderer.on(
      response.project.contextmenu,
      (
        event: IpcRendererEvent,
        type: SelectableElement,
        curCursorPos: string,
        data?: string
      ) => {
        switch (type) {
          case SelectableElement.POI:
            onContextmenuOnPointOfInterest(event, curCursorPos);
            break;
          case SelectableElement.AGENT:
            onContextmenuOnAgent(event, curCursorPos, data);
            break;
          case SelectableElement.ZONE:
            onContextmenuOnZone(event, curCursorPos, data);
            break;
          case SelectableElement.VEHICLE:
            onContextmenuOnVehicle(event, curCursorPos, data);
            break;
          default:
            break;
        }
      }
    );

    // if current element is none
    if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.NONE
    ) {
      handleClose();
    }
    // if current element is agent
    else if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.AGENT
    ) {
      const currAgent = currentSelectElement.value as CurrentAgent;
      setState({
        ...initState,
        agent: agents.find((x) => x.address.id === currAgent.id) || null,
        target: Target.NONE,
      });
    }
    // if current element is point of interest
    else if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.POI
    ) {
      // TODO:
      // menu = move here ?
    }

    // if current element is zone
    else if (
      currentSelectElement &&
      currentSelectElement.type === SelectableElement.ZONE
    ) {
      // TODO:
    }

    return () => {
      window.ipcRenderer.removeAllListeners(response.project.contextmenu);
    };
  }, [currentSelectElement]);

  React.useEffect(() => {
    if (currentMenu) {
      setContextMenu(true);
    } else {
      setContextMenu(false);
    }
  }, [currentMenu]);

  /**
   * Leaflet map object
   */
  useMapEvents({
    contextmenu: (event: LeafletMouseEvent) => {
      // Right click on map
      event.originalEvent.preventDefault();

      // Set cursor container point and latlngpoint on right click on map
      const c = {
        containerPoint: event.containerPoint,
        latlngPoint: event.latlng,
      };

      // If is drawing zone
      if (currentBuildMode === BuildMode.ZONE) {
        // undo last point in zone
        window.ipcRenderer.invoke(request.project.undoZone);
      }

      // No agent chosen
      else if (
        currentSelectElement &&
        currentSelectElement.type === SelectableElement.NONE
      ) {
        setState({
          ...initState,
          cursor: { ...c },
          target: Target.MAP_NONE_SELECTED,
        });
      }

      // If current agent exist
      else if (
        currentSelectElement &&
        currentSelectElement.type === SelectableElement.AGENT
      ) {
        setState({
          ...initState,
          cursor: { ...c },
          target: Target.MAP_AGENT_SELECTED,
        });
      }

      // If current zone
      else if (
        currentSelectElement &&
        currentSelectElement.type === SelectableElement.ZONE
      ) {
        // TODO: open menu while contextmenu on map ?
      }
    },
  });

  return (
    (contextMenu && state.cursor && (
      <CtxMenu
        cursor={state.cursor.containerPoint}
        menu={currentMenu || []}
        onClose={handleClose}
        offSetX={-50}
        offSetY={-70}
      />
    )) ||
    null
  );
}

export default React.memo(ContextMenuLayer);
