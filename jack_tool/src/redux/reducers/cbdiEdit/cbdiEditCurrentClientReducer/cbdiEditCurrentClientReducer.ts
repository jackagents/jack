/* eslint-disable no-param-reassign */
import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import {
  CBDIEditorProject,
  IModelError,
  CBDIEditorProjectAgent,
  CBDIEditorProjectTeam,
  CBDIEditorProjectPlan,
  IModelWarning,
} from 'types/cbdiEdit/cbdiEditModel';
import { request } from 'projectEvents/cbdiEdit/editEvents';
import { getObjectByModuleConcept, copy, updateCbdiObjectsNameAndModule } from 'misc/utils/cbdiEdit/Helpers';
import { MODE_NONE, MODE_PROJECT } from 'constant/cbdiEdit/cbdiEditConstant';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import { NodesWithFieldsDistance } from 'components/cbdiEdit/detail/conceptDetail/planTask/helper';
import {
  ModuleConcept,
  Mod,
  IPlanNode,
  IPlanEdge,
  CBDIEditorObject,
  CBDIEditorRootConceptType,
  CBDIEditorPlanNodeType,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { v4 } from 'uuid';
import { deleteObject, putObject, deregisterObject, changeObjectModule } from '../Helpers';
import packageJson from '../../../../../package.json';

export interface CbdiEditClientState {
  theme: string;
  project: {
    current: CBDIEditorProject | null;
    errors: IModelError[];
    warnings: IModelWarning[];
  };
  aboutDialog: {
    isVisible: boolean;
  };
  explorer: {
    mode: string;
    prevMode: string;
  };
  debugConsole: {
    isVisible: boolean;
  };
  errorConceptType: string | null;
  selectedTreeNodeConcept: ModuleConcept | null;
  scrollToSelectedTreeNodeFlag: boolean;
  selectedTreeNodeConceptStack: ModuleConcept[];
  selectedTreeNodeConceptStackActiveIndex: number;
  graph: {
    selectedNodeConcept: ModuleConcept | null;
    detail: {
      isVisible: boolean;
    };
    actionNodesContextArr: NodesWithFieldsDistance[];
  };
  xaiLiveDebug: boolean;
}

const cbdiEditInitialState: CbdiEditClientState = {
  theme: window.electronStore?.get('editorTheme') || 'light',
  project: {
    current: null,
    errors: [],
    warnings: [],
  },
  aboutDialog: {
    isVisible: false,
  },
  explorer: {
    mode: MODE_PROJECT,
    prevMode: MODE_PROJECT,
  },
  debugConsole: {
    isVisible: false,
  },
  errorConceptType: null,
  selectedTreeNodeConcept: null,
  scrollToSelectedTreeNodeFlag: false,
  selectedTreeNodeConceptStack: [],
  selectedTreeNodeConceptStackActiveIndex: 0,
  graph: {
    selectedNodeConcept: null,
    detail: {
      isVisible: true,
    },

    actionNodesContextArr: [],
  },
  xaiLiveDebug: false,
};

export const CBDI_UNDO_TYPE = 'cbdi-editor-undo';
export const CBDI_REDO_TYPE = 'cbdi-editor-redo';
export const CBDI_CLEAR_HISTORY_TYPE = 'cbdi-editor-clear-history';

export const cbdiEditReducers = {
  resetCbdiEditor: (state: CbdiEditClientState) => cbdiEditInitialState,
  setTheme: (state: CbdiEditClientState, action: PayloadAction<'dark' | 'light'>) => {
    window.electronStore.set('editorTheme', action.payload);
    state.theme = action.payload;
  },
  switchTeamAgent: (state: CbdiEditClientState, action: PayloadAction<ModuleConcept>) => {
    const current: CBDIEditorProject = copy(state.project.current);
    const currentModuleConcept = action.payload;
    const originalObj = current.cbdiObjects[currentModuleConcept.uuid];
    const targetType =
      originalObj._objectType === CBDIEditorRootConceptType.AgentConceptType
        ? CBDIEditorRootConceptType.TeamConceptType
        : CBDIEditorRootConceptType.AgentConceptType;

    const currentDir = `${originalObj._objectType}s` as 'teams' | 'agents';
    const targetDir = `${targetType}s` as 'teams' | 'agents';
    current[currentDir] = current[currentDir].filter((moduleConcept) => moduleConcept.uuid !== currentModuleConcept.uuid);
    current[targetDir].push(currentModuleConcept);
    current.cbdiObjects[currentModuleConcept.uuid] = {
      ...originalObj,
      _objectType: targetType,
      _mod: Mod.Update,
    } as CBDIEditorProjectAgent | CBDIEditorProjectTeam;
    state.project.current = current;
  },
  toggleAboutDialog: (state: CbdiEditClientState, action: PayloadAction<boolean>) => {
    state.aboutDialog.isVisible = action.payload;
  },
  updatePlan: (
    state: CbdiEditClientState,
    action: PayloadAction<{
      planRerferConcept: ModuleConcept;
      nodes?: IPlanNode[];
      edges?: IPlanEdge[];
    }>,
  ) => {
    const current: CBDIEditorProject = copy(state.project.current);
    const updatingPlanObj = getObjectByModuleConcept(current, action.payload.planRerferConcept) as CBDIEditorProjectPlan;
    if (action.payload.nodes !== undefined) {
      updatingPlanObj.tasks = action.payload.nodes;
    }
    if (action.payload.edges !== undefined) {
      updatingPlanObj.edges = action.payload.edges;
    }
    // make the planNode right after start to be the first element of tasks in model

    const firstNodeId = updatingPlanObj.edges.find(
      (edge) => edge.source === `${CBDIEditorPlanNodeType.StartPlanNodeType}/${updatingPlanObj.uuid}`,
    )?.target;
    const sortedTasks = [...updatingPlanObj.tasks].sort((x, y) =>
      // eslint-disable-next-line no-nested-ternary
      x.nodeId === firstNodeId ? -1 : y.nodeId === firstNodeId ? 1 : 0,
    );
    updatingPlanObj.tasks = sortedTasks;

    state.project.current = current;
  },
  deleteObjects: (state: CbdiEditClientState, action: PayloadAction<string | string[]>) => {
    const current: CBDIEditorProject = copy(state.project.current);
    if (Array.isArray(action.payload)) {
      action.payload.forEach((objectId: string) => deleteObject(current, objectId));
    } else {
      deleteObject(current, action.payload as string);
    }
    state.project.current = current;
  },
  putObjects: (state: CbdiEditClientState, action: PayloadAction<CBDIEditorObject | CBDIEditorObject[]>) => {
    const current: CBDIEditorProject = copy(state.project.current);
    if (Array.isArray(action.payload)) {
      action.payload.forEach((object: CBDIEditorObject) => putObject(current, object));
    } else {
      putObject(current, action.payload as CBDIEditorObject);
    }
    state.project.current = current;
  },
  // Not in use
  // revertObjects: (
  //   state: CbdiEditClientState,
  //   action: PayloadAction<Id | Id[]>
  // ) => {
  //   const current = copy(state.project.current);
  //   const saved = copy(state.project.saved);
  //   const revertObject = (objectId: Id) => {
  //     if (current && saved) {
  //       const currentObject = current.cbdiObjects[objectId];
  //       if (currentObject._mod === Mod.Addition) {
  //         // Reverting a newly added object means deleting it.
  //         deleteObject(current, objectId);
  //       } else {
  //         const object: ICbdiObject = copy(saved.cbdiObjects[objectId]);
  //         current.cbdiObjects[objectId] = object;
  //       }
  //     }
  //   };
  //   if (Array.isArray(action.payload)) {
  //     action.payload.forEach((objectId: Id) => revertObject(objectId));
  //   } else {
  //     revertObject(action.payload as Id);
  //   }
  //   state.project.current = current;
  // },
  // saveProject: (
  //   state: CbdiEditClientState,
  //   action: PayloadAction<{ forceUpdate: boolean } | undefined>
  // ) => {
  //   // if it is not forceUpdate and project current and saved is same, return
  //   if (
  //     action.payload?.forceUpdate === undefined &&
  //     isObjectEqual(state.project.current, state.project.saved)
  //   ) {
  //     return;
  //   }
  //   const current: IProject = copy(state.project.current);
  //   const saved: IProject = copy(state.project.saved);
  //   saved.name = current.name;
  //   saved.namespaces = current.namespaces;
  //   Object.keys(current.cbdiObjects).forEach((id) => {
  //     const object = copy(current.cbdiObjects[id]);
  //     saveObject(current, saved, object._id);
  //   });

  //   state.project.current = current;
  //   state.project.saved = current;

  //   // Informs the backend to save the new project model into file.
  //   window.ipcRenderer.invoke(request.project.update, current);
  // },
  updateCurrent: (state: CbdiEditClientState, action: PayloadAction<CBDIEditorProject>) => {
    state.project.current = action.payload;
  },
  saveAsProject: (state: CbdiEditClientState) => {
    const current: CBDIEditorProject = copy(state.project.current);
    // Informs the backend to save the new project model as a different file.
    window.ipcRenderer.invoke(request.project.saveAs, current);
  },
  createModule: (state: CbdiEditClientState) => {
    const current: CBDIEditorProject = copy(state.project.current);
    const id = v4();
    const moduleName = `newModule${id.slice(0, 4)}`;
    current.modulePaths.push({ name: moduleName, path: undefined, valid: true });
    const versions = packageJson.version.split('.');
    current.moduleProjectInfoDic[moduleName] = {
      name: moduleName,
      namespaces: [],
      major_version: versions[0],
      minor_version: versions[1],
      patch_version: versions[2],
      generator: 'JACK Editor',
      modules: [],
      search_paths: [],
    };
    state.project.current = current;
  },
  // Not in use ?
  // saveObjects: (
  //   state: CbdiEditClientState,
  //   action: PayloadAction<Id | Id[]>
  // ) => {
  //   const current = copy(state.project.current);
  //   const saved = copy(state.project.saved);
  //   if (Array.isArray(action.payload)) {
  //     action.payload.forEach((objectId: Id) =>
  //       saveObject(current, saved, objectId)
  //     );
  //   } else {
  //     saveObject(current, saved, action.payload as Id);
  //   }
  //   state.project.current = current;
  //   state.project.saved = saved;
  //   // Informs the backend to save the new project model into file.
  //   window.ipcRenderer.invoke(request.project.update, saved);
  // },
  setDebugConsoleIsVisible: (state: CbdiEditClientState, action: PayloadAction<boolean>) => {
    state.debugConsole.isVisible = action.payload;
  },
  setExplorerMode: (state: CbdiEditClientState, action: PayloadAction<string>) => {
    if (state.explorer.mode !== MODE_NONE) {
      state.explorer.prevMode = state.explorer.mode;
    }
    state.explorer.mode = action.payload;
  },
  setGraphSelectedNode: (state: CbdiEditClientState, action: PayloadAction<ModuleConcept | null>) => {
    state.graph.selectedNodeConcept = action.payload;
  },
  setActionNodesContextArr: (state: CbdiEditClientState, action: PayloadAction<NodesWithFieldsDistance[]>) => {
    state.graph.actionNodesContextArr = action.payload;
  },
  setGraphDetailIsVisible: (state: CbdiEditClientState, action: PayloadAction<boolean>) => {
    state.graph.detail.isVisible = action.payload;
  },
  setProject: (state: CbdiEditClientState, action: PayloadAction<CBDIEditorProject | null>) => {
    const project: CBDIEditorProject | null = action.payload;
    state.project.current = project;
  },
  setCurrentProject: (state: CbdiEditClientState, action: PayloadAction<CBDIEditorProject | null>) => {
    const project: CBDIEditorProject | null = action.payload;
    state.project.current = project;
  },
  renameModuleName: (state: CbdiEditClientState, action: PayloadAction<{ oldModuleName: string; newModuleName: string }>) => {
    const { oldModuleName, newModuleName } = action.payload;
    const currentProject = copy(state.project.current);
    window.ipcRenderer.invoke(request.project.renameModule, currentProject, oldModuleName, newModuleName);
  },
  setProjectError: (state: CbdiEditClientState, action: PayloadAction<IModelError[]>) => {
    state.project.errors = action.payload;
  },
  setProjectWarning: (state: CbdiEditClientState, action: PayloadAction<IModelWarning[]>) => {
    state.project.warnings = action.payload;
  },
  setErrorConceptType: (state: CbdiEditClientState, action: PayloadAction<string | null>) => {
    state.errorConceptType = action.payload;
  },
  setScrollToSelectedTreeNodeFlag: (state: CbdiEditClientState) => {
    state.scrollToSelectedTreeNodeFlag = !state.scrollToSelectedTreeNodeFlag;
  },
  setSelectedTreeNodeConcept: (state: CbdiEditClientState, action: PayloadAction<ModuleConcept | null>) => {
    if (action.payload !== null && !areModuleConceptsEqual(action.payload, state.selectedTreeNodeConcept)) {
      const stackRemoveForward = state.selectedTreeNodeConceptStack.slice(0, state.selectedTreeNodeConceptStackActiveIndex + 1);
      const newStack = [...stackRemoveForward, action.payload];
      if (newStack.length > 10) {
        newStack.splice(0, newStack.length - 10);
      }
      state.selectedTreeNodeConceptStack = newStack;
      state.selectedTreeNodeConceptStackActiveIndex = newStack.length - 1;
    }
    state.selectedTreeNodeConcept = action.payload;
  },
  popSelectedTreeNodeConceptStack: (state: CbdiEditClientState) => {
    if (state.selectedTreeNodeConceptStackActiveIndex > 0) {
      const newActiveIndex = state.selectedTreeNodeConceptStackActiveIndex - 1;
      const activeSelectedTreeNode = state.selectedTreeNodeConceptStack[newActiveIndex];

      state.selectedTreeNodeConcept = activeSelectedTreeNode;
      state.graph.selectedNodeConcept = activeSelectedTreeNode;
      state.selectedTreeNodeConceptStackActiveIndex = newActiveIndex;
    }
  },
  pushSelectedTreeNodeConceptStack: (state: CbdiEditClientState) => {
    if (state.selectedTreeNodeConceptStackActiveIndex < state.selectedTreeNodeConceptStack.length - 1) {
      const newActiveIndex = state.selectedTreeNodeConceptStackActiveIndex + 1;
      const activeSelectedTreeNode = state.selectedTreeNodeConceptStack[newActiveIndex];

      state.selectedTreeNodeConcept = activeSelectedTreeNode;
      state.graph.selectedNodeConcept = activeSelectedTreeNode;
      state.selectedTreeNodeConceptStackActiveIndex = newActiveIndex;
    }
  },
  updateObjects: (state: CbdiEditClientState, action: PayloadAction<CBDIEditorObject | CBDIEditorObject[]>) => {
    const current: CBDIEditorProject = copy(state.project.current);
    const updateObject = (cbdiObject: CBDIEditorObject) => {
      const object: CBDIEditorObject = copy(cbdiObject);
      object._mod = Mod.Update;
      if (!current.cbdiObjects[object.uuid]) {
        console.error(`Cannot update object ${object.name}: object not found. To create new objects, please use putObjects() instead`);
        return;
      }
      if (current) {
        // if object name changes
        if (current.cbdiObjects[object.uuid] && current.cbdiObjects[object.uuid].name !== object.name) {
          const deletedIdenticalNameObj = Object.values(current.cbdiObjects).find((obj) => {
            if (obj.name === object.name && obj._mod === Mod.Deletion && obj._objectType === object._objectType) {
              return true;
            }
            return false;
          });
          // if there is deleted object with same name
          // reuse the id from deleted object
          if (deletedIdenticalNameObj) {
            deregisterObject(current, object);
            current.cbdiObjects[deletedIdenticalNameObj.uuid] = {
              ...object,
              uuid: deletedIdenticalNameObj.uuid,
            };
          } else {
            current.cbdiObjects[object.uuid] = object;
          }
        } else {
          current.cbdiObjects[object.uuid] = object;
        }
      }
    };
    if (Array.isArray(action.payload)) {
      action.payload.forEach((object: CBDIEditorObject) => updateObject(object));
    } else {
      updateObject(action.payload as CBDIEditorObject);
    }
    current.cbdiObjects = updateCbdiObjectsNameAndModule(current.cbdiObjects);
    state.project.current = current;
  },
  updateProjectName: (state: CbdiEditClientState, action: PayloadAction<string>) => {
    const current: CBDIEditorProject = copy(state.project.current);
    current.name = action.payload;
    state.project.current = current;
  },
  setXaiLiveDebug: (state: CbdiEditClientState, action: PayloadAction<boolean>) => {
    state.xaiLiveDebug = action.payload;
  },
  updateObjectModule: (state: CbdiEditClientState, action: PayloadAction<{ updatingModuleConcept: ModuleConcept; newModuleName: string }>) => {
    const current: CBDIEditorProject = copy(state.project.current);
    const { updatingModuleConcept, newModuleName } = action.payload;
    changeObjectModule(current, updatingModuleConcept, newModuleName);
    // Update cbdiObjects
    current.cbdiObjects = updateCbdiObjectsNameAndModule(current.cbdiObjects);

    state.project.current = current;
    // Update selectedTreeNodeConcept if it is updatingModuleConcept
    if (state.selectedTreeNodeConcept) {
      state.selectedTreeNodeConcept = { ...state.selectedTreeNodeConcept, module: newModuleName };
    }
    // Update graph selectedTreeNodeConcept if it is updatingModuleConcept
    if (state.graph.selectedNodeConcept && state.graph.selectedNodeConcept.uuid === updatingModuleConcept.uuid) {
      state.graph.selectedNodeConcept = { ...state.graph.selectedNodeConcept, module: newModuleName };
    }
  },
};

export const cbdiEditCurrentClientSlice = createSlice({
  name: 'cbdiEditCurrent',
  initialState: cbdiEditInitialState,
  reducers: cbdiEditReducers,
});

export default cbdiEditCurrentClientSlice.reducer;

/* ************************************************************************************************
 * Actions
 * *********************************************************************************************** */
export const { actions: cbdiEditActions } = cbdiEditCurrentClientSlice;
