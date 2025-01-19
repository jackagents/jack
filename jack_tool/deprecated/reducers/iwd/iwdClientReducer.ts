import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import {
  CREATOR_MENU,
  MODE,
  ZONE_TYPE,
} from 'misc/constant/common/cmConstants';
import {
  Agent,
  BuildMode,
  CurrentSelectElement,
  CustomMenu,
  LoadResult,
  PointOfInterestType,
  SavedData,
  SelectableElement,
} from 'types/iwd/iwdTypes';

export interface IwdClientState {
  // Template
  explorer: {
    mode: string;
    prevMode: string;
  };
  // Control left menu
  menu: CustomMenu;
  // Important: control whole application state
  currentBuildMode: BuildMode;
  // Control creator tool (1st button of left menu)
  currentCreatorTool: string;
  // Selected zone type from creator menu / add zone
  currentZoneType: number;
  // For save/cancel modal in build zone/entity
  saveMode: BuildMode;
  // For undo zone
  undoZoneBuilder: boolean;
  // For drag zone when build
  dragMode: boolean;
  // Data loaded from json file
  loadedData: any[];
  // Important: all data of zone and entities in application
  savedData: SavedData;
  // For incremental id of objects created in application
  incrementId: number;
  // Data loaded from database to use in shopping menu
  entityDB: null | {
    types: string[];
    radars: {
      prototypes: any[];
    };
    defended_assets: { prototypes: any[] };
    threats: { prototypes: any[] };
    effectors: { prototypes: any[] };
    weaponGroup: { prototypes: any[] };
  };
  // For control prompt window
  prompt: {
    isOpen: boolean;
    isLoading: boolean;
  };
  // Agents
  agents: Agent[];
  // Points of interest
  pointsOfInterest: PointOfInterestType[];
  // Current select element
  currentSelectElement: CurrentSelectElement;
}

export const iwdInitialState: IwdClientState = {
  explorer: {
    mode: MODE.MODE_NONE,
    prevMode: MODE.MODE_NONE,
  },
  menu: {
    searchMenu: false,
    creatorMenu: false,
    settingMenu: false,
    simMapMenu: false,
    explainability: false,
    scenarioEditor: false,
  },
  currentBuildMode: BuildMode.NONE,
  currentCreatorTool: CREATOR_MENU.NONE,
  currentZoneType: ZONE_TYPE.NONE,
  saveMode: BuildMode.NONE,
  undoZoneBuilder: false,
  dragMode: false,
  loadedData: [],
  savedData: { zone: [], entities: [] },
  incrementId: 0,
  entityDB: null,
  prompt: {
    isOpen: false,
    isLoading: false,
  },
  agents: [],
  pointsOfInterest: [],
  currentSelectElement: {
    type: SelectableElement.NONE,
    value: undefined,
  },
};

export const iwdReducers = {
  resetStore: (state: IwdClientState) => {
    state.savedData = { zone: [], entities: [] };
    state.pointsOfInterest = [];
    state.agents = [];
    state.menu = {
      searchMenu: false,
      creatorMenu: false,
      settingMenu: false,
      simMapMenu: false,
      explainability: false,
      scenarioEditor: false,
    };

    // eslint-disable-next-line no-param-reassign
    state = {
      ...iwdInitialState,
    };
  },
  setExplorerMode: (state: IwdClientState, action: PayloadAction<string>) => {
    if (state.explorer.mode !== MODE.MODE_NONE) {
      state.explorer.prevMode = state.explorer.mode;
    }
    state.explorer.mode = action.payload;
  },
  setMenu: (state: IwdClientState, action: PayloadAction<CustomMenu>) => {
    state.menu = action.payload;
  },
  changeBuildMode: (
    state: IwdClientState,
    action: PayloadAction<BuildMode>
  ) => {
    state.currentBuildMode = action.payload;
    console.log('buildMode', state.currentBuildMode);
  },
  changeCurrentCreatorTool: (
    state: IwdClientState,
    action: PayloadAction<string>
  ) => {
    state.currentCreatorTool = action.payload;
  },
  setZoneType: (state: IwdClientState, action: PayloadAction<number>) => {
    state.currentZoneType = action.payload;
  },
  setSaveMode: (state: IwdClientState, action: PayloadAction<BuildMode>) => {
    state.saveMode = action.payload;
  },
  toggleUndoZoneBuilder: (
    state: IwdClientState,
    action: PayloadAction<boolean>
  ) => {
    state.undoZoneBuilder = action.payload;
  },
  toggleDragMode: (state: IwdClientState, action: PayloadAction<boolean>) => {
    state.dragMode = action.payload;
  },
  updateLoadedData: (state: IwdClientState, action: PayloadAction<any[]>) => {
    state.loadedData = action.payload;
  },
  updateLoadedDataToStore: (
    state: IwdClientState,
    action: PayloadAction<LoadResult>
  ) => {
    state.savedData = action.payload.savedData;
    console.log(state.savedData);
  },
  updateSavedData: (
    state: IwdClientState,
    action: PayloadAction<SavedData>
  ) => {
    state.savedData = action.payload;
    console.log(state.savedData);
  },
  setIncrementId: (state: IwdClientState, action: PayloadAction<number>) => {
    state.incrementId = action.payload;
  },
  updateIncrementId: (state: IwdClientState) => {
    state.incrementId += 1;
  },
  saveEntityDB: (state: IwdClientState, action: PayloadAction<any>) => {
    state.entityDB = action.payload;
  },
  closeConfigDisplay: (state: IwdClientState) => {
    state.currentBuildMode = BuildMode.NONE;
    state.currentSelectElement = {
      type: SelectableElement.NONE,
      value: undefined,
    };
  },
  togglePrompt: (state: IwdClientState) => {
    state.prompt = {
      ...state.prompt,
      isOpen: !state.prompt.isOpen,
    };
  },
  toggleLoading: (state: IwdClientState, action: PayloadAction<boolean>) => {
    state.prompt = {
      ...state.prompt,
      isLoading: action.payload,
    };
  },
  updateAgents: (state: IwdClientState, action: PayloadAction<Agent[]>) => {
    state.agents = action.payload;
  },
  updateAllPointsOfInterest: (
    state: IwdClientState,
    action: PayloadAction<string>
  ) => {
    state.pointsOfInterest = JSON.parse(
      action.payload
    ) as PointOfInterestType[];
  },
  addNewPointOfInterest: (
    state: IwdClientState,
    action: PayloadAction<string>
  ) => {
    const poi = JSON.parse(action.payload) as PointOfInterestType;
    state.pointsOfInterest.push(poi);
  },
  updatePointOfInterest: (
    state: IwdClientState,
    action: PayloadAction<string>
  ) => {
    const poi = JSON.parse(action.payload) as PointOfInterestType;
    const index = state.pointsOfInterest.findIndex((x) => poi.id === x.id);

    if (index < 0) {
      console.log("redux can't updatePointOfIntersest");
      return;
    }

    state.pointsOfInterest[index] = poi;

    // Update current select element
    if (
      state.currentSelectElement &&
      (state.currentSelectElement.value as PointOfInterestType).id === poi.id
    ) {
      state.currentSelectElement.value = poi;
    }
  },
  removePointsOfInterest: (
    state: IwdClientState,
    action: PayloadAction<string>
  ) => {
    const poi = JSON.parse(action.payload) as PointOfInterestType;

    // Check if the point of interest is exist
    const result = state.pointsOfInterest.findIndex((x) => poi.id === x.id);

    if (result > -1) {
      // Check if current select element is the removed point of interest
      if (
        state.currentSelectElement &&
        state.currentSelectElement.type === SelectableElement.POI
      ) {
        const target = state.currentSelectElement.value as PointOfInterestType;

        // Return current select element to none
        if (target.id === poi.id) {
          state.currentSelectElement = {
            type: SelectableElement.NONE,
            value: undefined,
          };
        }
      }

      // Remove from points of interest
      state.pointsOfInterest.splice(result, 1);
    }
  },
  changeCurrentSelectElement: (
    state: IwdClientState,
    action: PayloadAction<string>
  ) => {
    const payload = JSON.parse(action.payload) as CurrentSelectElement;

    state.currentSelectElement = payload;
  },
};

export const iwdClientSlice = createSlice({
  name: 'iwd',
  initialState: iwdInitialState,
  reducers: iwdReducers,
});

export default iwdClientSlice.reducer;

/* ************************************************************************************************
 * Actions
 * *********************************************************************************************** */
export const { actions: iwdActions } = iwdClientSlice;
