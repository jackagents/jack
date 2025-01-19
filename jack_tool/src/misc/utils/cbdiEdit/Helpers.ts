/* eslint-disable no-case-declarations */
/* eslint-disable no-continue */
/* eslint-disable no-restricted-syntax */
import { DEFAULT_PLAN_POLICY } from 'constant/cbdiEdit/cbdiEditConstant';
import fs from 'fs';
import {
  CBDIEditorProject,
  InitialEndNode,
  InitialStartNode,
  CBDIEditorProjectPlan,
  CBDIEditorProjectTactic,
  CBDIEditorProjectTeam,
  CBDIEditorProjectAgent,
  CBDIEditorProjectRole,
  CBDIEditorProjectService,
  CBDIEditorProjectResource,
  CBDIEditorProjectGoal,
  CBDIEditorProjectAction,
  CBDIEditorProjectMessage,
  CBDIEditorProjectEnum,
  CBDIEditorProjectEntity,
  CBDIEditorProjectEvent,
  CBDIEditorRootObjectDic,
} from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  CBDIEditorObject,
  CBDIEditorPlanEditorEdgeCondition,
  CBDIEditorRootConceptType,
  EmptyModuleConcept,
  CBDIEditorTBasicMessageSchema,
  Mod,
  ModuleConcept,
  ProjectConceptListType,
  Vec2Type,
  defaultEdgeControlPoints,
  IPlanNode,
  PlanEditorNodeType,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { v4 } from 'uuid';
import { copyObj, parseNumber } from '../common/commonUtils';

export function getObjectById<T extends CBDIEditorObject>(project: CBDIEditorProject, id: string): T | undefined {
  if (!project) {
    console.error('Cannot get object by id, project is undefined');
    return undefined;
  }
  return <T>project.cbdiObjects[id];
}

export function getObjectsByIds<T extends CBDIEditorObject>(project: CBDIEditorProject, ids: string[]): T[] {
  if (!project) {
    console.error('Cannot get object by id, project is undefined');
    return [];
  }
  const objects: T[] = [];
  for (const id of ids) {
    const object = getObjectById<T>(project, id);
    if (object) {
      objects.push(object);
    }
  }
  return objects;
}

export function getObjectByModuleConcept<T extends CBDIEditorObject>(
  project: CBDIEditorProject | null,
  moduleConcept: ModuleConcept | undefined | null,
): T | undefined {
  if (!project) {
    console.error('Cannot get object by id, project is undefined');
    return undefined;
  }
  if (moduleConcept === undefined || moduleConcept === null) {
    return undefined;
  }

  if (project.cbdiObjects[moduleConcept.uuid]) {
    return <T>project.cbdiObjects[moduleConcept.uuid];
  }
  return undefined;
}

export function getObjectByModuleConcepts<T extends CBDIEditorObject>(project: CBDIEditorProject, moduleConcepts: ModuleConcept[]): T[] {
  if (!project) {
    console.error('Cannot get object by id, project is undefined');
    return [];
  }
  const objects: T[] = [];
  for (const moduleConcept of moduleConcepts) {
    const object = getObjectByModuleConcept<T>(project, moduleConcept);
    if (object) {
      objects.push(object);
    }
  }
  return objects;
}

export function isObjectDirty(currentProject: CBDIEditorProject, savedProject: CBDIEditorProject, id: string): boolean {
  if (!currentProject) {
    console.error('Cannot check if object is dirty, project is undefined');
    return false;
  }
  const currentObject = currentProject.cbdiObjects[id];
  const savedObject = savedProject.cbdiObjects[id];
  if (!currentObject) {
    console.error('Cannot check if object is dirty, object is undefined');
    return false;
  }
  // if current and saved object is deleted, it it not dirty
  if (currentObject._mod === Mod.Deletion && savedObject?._mod === Mod.Deletion) {
    return false;
  }
  if (currentObject._mod !== Mod.None) {
    return true;
  }

  return false;
}

// find all concept options to add from project
export function getAllObjOptionsForSingleItem(addingItemType: CBDIEditorRootConceptType, project: CBDIEditorProject | null | undefined) {
  if (!project) {
    return [];
  }
  const conceptTypes: ProjectConceptListType = (() => {
    if (addingItemType === CBDIEditorRootConceptType.EntityConceptType) {
      return 'entities';
    }
    return `${addingItemType}s`;
  })();
  const addingModuleConcepts: ModuleConcept[] = [];
  // put all objects from project

  for (const addingItemListModuleConcept of project[conceptTypes]) {
    const object = project.cbdiObjects[addingItemListModuleConcept.uuid];
    if (!object) {
      continue;
    }
    if (object._objectType === addingItemType) {
      addingModuleConcepts.push({
        uuid: object.uuid,
        name: object.name,
        module: object.module,
      });
    }
  }

  return addingModuleConcepts;
}

// find the available concept options to add both from project and modules
export function getAddingModuleConceptOptions(
  moduleConceptsInList: ModuleConcept[],
  addingProjectField: ProjectConceptListType,
  project: CBDIEditorProject,
) {
  const addingItemType = (() => {
    if (addingProjectField === 'entities') {
      return CBDIEditorRootConceptType.EntityConceptType;
    }
    return addingProjectField.slice(0, -1);
  })() as CBDIEditorRootConceptType;
  const addingModuleConcepts: ModuleConcept[] = [];

  // 1. put the unadded objects from project
  for (const addingItemListModuleConcept of project[addingProjectField]) {
    let addFlag = false;
    if (!moduleConceptsInList.some((value: ModuleConcept) => value.uuid === addingItemListModuleConcept.uuid)) {
      addFlag = true;
    }

    if (addFlag) {
      const object = project.cbdiObjects[addingItemListModuleConcept.uuid];
      if (!object) {
        continue;
      }
      if (object._objectType === addingItemType) {
        addingModuleConcepts.push({
          uuid: object.uuid,
          module: object.module,
          name: object.name,
        });
      }
    }
  }

  return addingModuleConcepts;
}

export function capitalize(str: string): string {
  return str.charAt(0).toUpperCase() + str.slice(1);
}

export function copy(obj: any): any {
  const objString = JSON.stringify(obj);
  if (objString) {
    return JSON.parse(JSON.stringify(obj));
  }
  return undefined;
}

// get first matching file path from a given directiory for a given filename
export function getFirstMatchingDirbyFileName(dir: string, fileName: string): string | undefined {
  const files = fs.readdirSync(dir);

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    const filePath = `${dir}\\${file}`;

    if (fs.statSync(filePath).isDirectory()) {
      const result = getFirstMatchingDirbyFileName(filePath, fileName);
      if (!result) {
        continue;
      }
      return result;
    }
    if (fileName === file) {
      return filePath;
    }
  }
  return undefined;
}

export function createNewPlanWithGoal(newPlanId: string, goalModuleConcept: ModuleConcept, conceptName?: string) {
  const startNode: IPlanNode = { ...InitialStartNode };
  startNode.nodeId = `start/${newPlanId}`;
  startNode.metaData!.timestamp = Date.now();
  const endNode = { ...InitialEndNode };
  endNode.nodeId = `end/${newPlanId}`;
  endNode.metaData!.timestamp = Date.now();

  const newPlan: CBDIEditorProjectPlan = {
    uuid: newPlanId,
    _mod: Mod.Addition,
    _objectType: CBDIEditorRootConceptType.PlanConceptType,
    module: goalModuleConcept.module,
    name: conceptName || `NewPlan_${newPlanId.substring(0, 4)}`,
    note: '',
    query_messages: [],
    precondition: { custom: false, query: '' },
    dropcondition: { custom: false, query: '' },
    effects: false,
    handles: goalModuleConcept,
    tasks: [startNode, endNode],
    edges: [
      {
        edgeData: {
          condition: CBDIEditorPlanEditorEdgeCondition.True,
          controlPoints: defaultEdgeControlPoints,
          timestamp: Date.now(),
        },
        source: startNode.nodeId,
        target: endNode.nodeId,
      },
    ],
  };
  return newPlan;
}

export function createNewTacticWithGoal(newTacticId: string, goalModuleConcept: ModuleConcept, conceptName?: string) {
  const newTactic: CBDIEditorProjectTactic = {
    uuid: newTacticId,
    _mod: Mod.Addition,
    module: goalModuleConcept.module,
    _objectType: CBDIEditorRootConceptType.TacticConceptType,
    name: conceptName || `NewTactic_${newTacticId.substring(0, 4)}`,
    note: '',
    goal: goalModuleConcept,
    ...DEFAULT_PLAN_POLICY,
  };
  return newTactic;
}

export function createNewConcept(newConceptId: string, conceptType: CBDIEditorRootConceptType, moduleName: string, conceptName?: string) {
  const newConceptObjects: CBDIEditorObject[] = [];

  switch (conceptType) {
    case CBDIEditorRootConceptType.TeamConceptType: {
      const newTeam: CBDIEditorProjectTeam = {
        uuid: newConceptId,
        _objectType: CBDIEditorRootConceptType.TeamConceptType,
        _mod: Mod.Addition,
        module: moduleName,
        name: conceptName || `NewTeam_${newConceptId.substring(0, 4)}`,
        note: '',
        resources: [],
        message_handlers: [],
        action_handlers: [],
        beliefs: [],
        goals: [],
        plans: [],
        roles: [],
        services: [],
      };
      newConceptObjects.push(newTeam);
      break;
    }

    case CBDIEditorRootConceptType.AgentConceptType: {
      const newAgent: CBDIEditorProjectAgent = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.AgentConceptType,
        name: conceptName || `NewAgent_${newConceptId.substring(0, 4)}`,
        note: '',
        resources: [],
        message_handlers: [],
        action_handlers: [],
        beliefs: [],
        goals: [],
        plans: [],
        roles: [],
        services: [],
      };
      newConceptObjects.push(newAgent);

      break;
    }

    case CBDIEditorRootConceptType.RoleConceptType: {
      const newRole: CBDIEditorProjectRole = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.RoleConceptType,
        name: conceptName || `NewRole_${newConceptId.substring(0, 4)}`,
        note: '',
        goals: [],
        messages: [],
      };
      newConceptObjects.push(newRole);

      break;
    }

    case CBDIEditorRootConceptType.ServiceConceptType: {
      const newService: CBDIEditorProjectService = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.ServiceConceptType,
        name: conceptName || `NewService_${newConceptId.substring(0, 4)}`,
        note: '',
        action_handlers: [],
        topics: [],
      };
      newConceptObjects.push(newService);
      break;
    }

    case CBDIEditorRootConceptType.ResourceConceptType: {
      const newRsc: CBDIEditorProjectResource = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.ResourceConceptType,
        name: conceptName || `NewResource_${newConceptId.substring(0, 4)}`,
        note: '',
        type: CBDIEditorTBasicMessageSchema.I32Type,
        min: 0,
        max: 1,
      };
      newConceptObjects.push(newRsc);
      break;
    }

    case CBDIEditorRootConceptType.GoalConceptType: {
      const newGoal: CBDIEditorProjectGoal = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.GoalConceptType,
        name: conceptName || `NewGoal_${newConceptId.substring(0, 4)}`,
        note: '',
        query_messages: [],
        precondition: { custom: false, query: '' },
        dropcondition: { custom: false, query: '' },
        satisfied: { custom: false, query: '' },
        heuristic: false,
        resources: [],
        message: EmptyModuleConcept,
      };
      newConceptObjects.push(newGoal);
      // create plan when create a goal
      const handleGoalModuleConcept: ModuleConcept = {
        module: newGoal.module,
        name: newGoal.name,
        uuid: newGoal.uuid,
      };
      const newPlanId = v4();
      const newPlanName = `${newGoal.name} Plan`;
      const newPlan = createNewPlanWithGoal(newPlanId, handleGoalModuleConcept, newPlanName);
      newConceptObjects.push(newPlan);

      break;
    }

    case CBDIEditorRootConceptType.PlanConceptType: {
      const startNode: IPlanNode = { ...InitialStartNode };
      startNode.nodeId = `start/${newConceptId}`;
      startNode.metaData!.timestamp = Date.now();
      const endNode = { ...InitialEndNode };
      endNode.nodeId = `end/${newConceptId}`;
      endNode.metaData!.timestamp = Date.now();

      const newPlan: CBDIEditorProjectPlan = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        _objectType: CBDIEditorRootConceptType.PlanConceptType,
        module: moduleName,
        name: conceptName || `NewPlan_${newConceptId.substring(0, 4)}`,
        note: '',
        query_messages: [],
        precondition: { custom: false, query: '' },
        dropcondition: { custom: false, query: '' },
        effects: false,
        handles: EmptyModuleConcept,
        tasks: [startNode, endNode],
        edges: [
          {
            edgeData: {
              condition: CBDIEditorPlanEditorEdgeCondition.True,
              controlPoints: defaultEdgeControlPoints,
              timestamp: Date.now(),
            },
            source: startNode.nodeId,
            target: endNode.nodeId,
          },
        ],
      };
      newConceptObjects.push(newPlan);

      break;
    }

    case CBDIEditorRootConceptType.TacticConceptType: {
      const newTactic: CBDIEditorProjectTactic = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.TacticConceptType,
        name: conceptName || `NewTactic_${newConceptId.substring(0, 4)}`,
        note: '',
        goal: EmptyModuleConcept,
        ...DEFAULT_PLAN_POLICY,
      };
      newConceptObjects.push(newTactic);

      break;
    }

    case CBDIEditorRootConceptType.ActionConceptType: {
      const newAction: CBDIEditorProjectAction = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.ActionConceptType,
        name: conceptName || `NewAction_${newConceptId.substring(0, 4)}`,
        note: '',
        request: EmptyModuleConcept,
        reply: EmptyModuleConcept,
        feedback: EmptyModuleConcept,
      };
      newConceptObjects.push(newAction);
      break;
    }

    case CBDIEditorRootConceptType.MessageConceptType: {
      const newMessage: CBDIEditorProjectMessage = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.MessageConceptType,
        name: conceptName || `NewMessage_${newConceptId.substring(0, 4)}`,
        note: '',
        component: false,
        fields: [],
        editor: [],
      };
      newConceptObjects.push(newMessage);

      break;
    }

    case CBDIEditorRootConceptType.EnumConceptType: {
      const newEnumType: CBDIEditorProjectEnum = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.EnumConceptType,
        name: conceptName || `NewEnum_${newConceptId.substring(0, 4)}`,
        note: '',
        fields: [],
      };
      newConceptObjects.push(newEnumType);
      break;
    }

    case CBDIEditorRootConceptType.EntityConceptType: {
      const newEntity: CBDIEditorProjectEntity = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.EntityConceptType,
        name: conceptName || `NewEntity_${newConceptId.substring(0, 4)}`,
        note: '',
        children: [],
        services: [],
        messages: [],
        agent: true,
      };
      newConceptObjects.push(newEntity);

      break;
    }

    case CBDIEditorRootConceptType.EventConceptType: {
      const newEvent: CBDIEditorProjectEvent = {
        uuid: newConceptId,
        _mod: Mod.Addition,
        module: moduleName,
        _objectType: CBDIEditorRootConceptType.EventConceptType,
        name: conceptName || `NewEvent_${newConceptId.substring(0, 4)}`,
        note: '',
        message: {
          ...EmptyModuleConcept,
          defaults: [],
        },
        requires_entity: false,
      };
      newConceptObjects.push(newEvent);
      break;
    }

    default:
      break;
  }
  return newConceptObjects;
}

export const createDuplicatedPlanObj = (originalPlan: CBDIEditorProjectPlan, newId: string): CBDIEditorProjectPlan => {
  const newPlanObj = copyObj(originalPlan);
  newPlanObj.uuid = newId;
  newPlanObj.name = `${originalPlan.name} copy`;
  const newTasks: IPlanNode[] = [];
  let newEdges = copyObj(originalPlan.edges);
  originalPlan.tasks.forEach((task) => {
    // create new node id for task
    let newNodeId = v4();
    // if task is start or end, update nodeId base on new plan id
    if (task.type === PlanEditorNodeType.Circle) {
      newNodeId = task.nodeId.replace(originalPlan.uuid, newId);
    }

    newTasks.push({ ...task, nodeId: newNodeId });
    newEdges = newEdges.map((edge) => {
      const newEdge = copyObj(edge);
      if (edge.source === task.nodeId) {
        newEdge.source = newNodeId;
      }
      if (edge.target === task.nodeId) {
        newEdge.target = newNodeId;
      }
      return newEdge;
    });
  });
  newPlanObj.tasks = newTasks;
  newPlanObj.edges = newEdges;

  return newPlanObj;
};

export function hasWhiteSpace(s: string) {
  return /\s/g.test(s);
}

/**
 * sort a moduleConcept list by ascending alphabetical order of its object name
 * @param unSortedModuleConceptList
 * @param currentProject
 * @returns
 */
export function sortModuleConceptList(unSortedModuleConceptList: ModuleConcept[], currentProject: CBDIEditorProject) {
  const sortedModuleConceptList = [...unSortedModuleConceptList].sort((a, b) => {
    const aObj = getObjectByModuleConcept(currentProject, a);
    const bObj = getObjectByModuleConcept(currentProject, b);
    if (!aObj || !bObj) {
      return 0;
    }
    const labelA = `${aObj.name}`;
    const labelB = `${bObj.name}`;
    return labelA.localeCompare(labelB, 'en');
  });
  return sortedModuleConceptList;
}

export const getDirNameFromType = (objectType: string) => {
  if (objectType === CBDIEditorRootConceptType.EntityConceptType) {
    return 'Entities';
  }
  return `${capitalize(objectType)}s`;
};

/**
 * check if module concept is empty
 */
export const checkModuleConceptIsEmpty = (moduleConcept: ModuleConcept) => {
  if (moduleConcept.uuid === '' && moduleConcept.name === '') {
    return true;
  }
  return false;
};

/**
 * parese vec2 string to vect
 * @param value
 * @returns
 */
export const pareseVec2String = (value: string): Vec2Type => {
  const xStr = value.split(',')[0].trim();
  const x = parseNumber(xStr);
  const yStr = value.split(',')[1].trim();
  const y = parseNumber(yStr);

  return { x, y };
};

/**
 * Get moduleConcept with name and module from cbdiObjects
 * @param moduleConcept
 * @param cbdiObjects
 * @returns
 */
const getModuleConceptWithUpdatedNameAndModule = (moduleConcept: ModuleConcept, cbdiObjects: CBDIEditorRootObjectDic) => {
  const { uuid, module, name, ...others } = cbdiObjects[moduleConcept.uuid] || moduleConcept;
  return { uuid, name, module };
};

const getModuleConceptsWithUpdatedNameAndModule = (moduleConcepts: ModuleConcept[], cbdiObjects: CBDIEditorRootObjectDic) =>
  moduleConcepts.map((moduleConcept) => getModuleConceptWithUpdatedNameAndModule(moduleConcept, cbdiObjects));

/**
 * Update cbdiObjects, make child moduleConcept's name and module align the object's name and module in cbdiObjects
 * @param cbdiObjects
 * @returns
 */
export const updateCbdiObjectsNameAndModule = (cbdiObjects: CBDIEditorRootObjectDic) => {
  const newCbdiObjects: CBDIEditorRootObjectDic = {};
  Object.entries(cbdiObjects).forEach(([uuid, object]) => {
    switch (object._objectType) {
      case CBDIEditorRootConceptType.EnumConceptType:
      case CBDIEditorRootConceptType.ResourceConceptType:
        newCbdiObjects[uuid] = object;
        break;

      case CBDIEditorRootConceptType.MessageConceptType:
        const newMessageObj = { ...object } as CBDIEditorProjectMessage;
        const fields = newMessageObj.fields;
        fields.forEach((field, index) => {
          if (typeof field.type === 'object') {
            if ('Custom' in field.type) {
              const customModuleConcept = field.type.Custom;
              const customModuleConceptObj = getModuleConceptWithUpdatedNameAndModule(customModuleConcept, cbdiObjects);
              newMessageObj.fields[index] = { ...field, type: { Custom: customModuleConceptObj } };
            }
            if ('Enum' in field.type) {
              const enumModuleConcept = field.type.Enum;
              const enumModuleConceptObj = getModuleConceptWithUpdatedNameAndModule(enumModuleConcept, cbdiObjects);
              newMessageObj.fields[index] = { ...field, type: { Enum: enumModuleConceptObj } };
            }
          }
        });
        newCbdiObjects[uuid] = newMessageObj;
        break;

      case CBDIEditorRootConceptType.EventConceptType:
        const newEventObj = { ...object } as CBDIEditorProjectEvent;
        const newMessageModuleConcept = getModuleConceptWithUpdatedNameAndModule(newEventObj.message, cbdiObjects);
        newEventObj.message = { ...newEventObj.message, ...newMessageModuleConcept };
        newCbdiObjects[uuid] = newEventObj;
        break;

      case CBDIEditorRootConceptType.ActionConceptType:
        const newActionObj = { ...object } as CBDIEditorProjectAction;
        newActionObj.request = getModuleConceptWithUpdatedNameAndModule(newActionObj.request, cbdiObjects);
        newActionObj.reply = getModuleConceptWithUpdatedNameAndModule(newActionObj.reply, cbdiObjects);
        newActionObj.feedback = getModuleConceptWithUpdatedNameAndModule(newActionObj.feedback, cbdiObjects);
        newCbdiObjects[uuid] = newActionObj;
        break;

      case CBDIEditorRootConceptType.GoalConceptType:
        const newGoalObj = { ...object } as CBDIEditorProjectGoal;
        newGoalObj.query_messages = getModuleConceptsWithUpdatedNameAndModule(newGoalObj.query_messages, cbdiObjects);
        newGoalObj.resources = getModuleConceptsWithUpdatedNameAndModule(newGoalObj.resources, cbdiObjects);
        newGoalObj.message = getModuleConceptWithUpdatedNameAndModule(newGoalObj.message, cbdiObjects);
        newCbdiObjects[uuid] = newGoalObj;
        break;

      case CBDIEditorRootConceptType.ServiceConceptType:
        const newServiceObj = { ...object } as CBDIEditorProjectService;
        newServiceObj.action_handlers = getModuleConceptsWithUpdatedNameAndModule(newServiceObj.action_handlers, cbdiObjects);
        newServiceObj.topics = newServiceObj.topics.map((topic) => ({
          ...topic,
          message: getModuleConceptWithUpdatedNameAndModule(topic.message, cbdiObjects),
        }));
        newCbdiObjects[uuid] = newServiceObj;
        break;

      case CBDIEditorRootConceptType.EntityConceptType:
        const newEntityObj = { ...object } as CBDIEditorProjectEntity;
        newEntityObj.children = getModuleConceptsWithUpdatedNameAndModule(newEntityObj.children, cbdiObjects);
        newEntityObj.services = getModuleConceptsWithUpdatedNameAndModule(newEntityObj.services, cbdiObjects);
        newEntityObj.messages = getModuleConceptsWithUpdatedNameAndModule(newEntityObj.messages, cbdiObjects);
        newCbdiObjects[uuid] = newEntityObj;
        break;

      case CBDIEditorRootConceptType.PlanConceptType:
        const newPlanObj = { ...object } as CBDIEditorProjectPlan;
        newPlanObj.query_messages = getModuleConceptsWithUpdatedNameAndModule(newPlanObj.query_messages, cbdiObjects);
        newPlanObj.handles = getModuleConceptWithUpdatedNameAndModule(newPlanObj.handles, cbdiObjects);
        newPlanObj.tasks = newPlanObj.tasks.map((task) => {
          const newTask = { ...task };
          newTask.nodeData.action = task.nodeData.action && getModuleConceptWithUpdatedNameAndModule(task.nodeData.action, cbdiObjects);
          newTask.nodeData.goal = task.nodeData.goal && getModuleConceptWithUpdatedNameAndModule(task.nodeData.goal, cbdiObjects);
          return newTask;
        });
        newCbdiObjects[uuid] = newPlanObj;
        break;

      case CBDIEditorRootConceptType.TacticConceptType:
        const newTacticObj = { ...object } as CBDIEditorProjectTactic;
        newTacticObj.plan_list = newTacticObj.plan_list.map((plan) => ({
          ...plan,
          moduleConcept: getModuleConceptWithUpdatedNameAndModule(plan.moduleConcept, cbdiObjects),
        }));
        newTacticObj.goal = getModuleConceptWithUpdatedNameAndModule(newTacticObj.goal, cbdiObjects);

        newCbdiObjects[uuid] = newTacticObj;
        break;

      case CBDIEditorRootConceptType.RoleConceptType:
        const newRoleObj = { ...object } as CBDIEditorProjectRole;
        newRoleObj.goals = getModuleConceptsWithUpdatedNameAndModule(newRoleObj.goals, cbdiObjects);
        newRoleObj.messages = newRoleObj.messages.map((message) => ({
          ...message,
          ...getModuleConceptWithUpdatedNameAndModule(message, cbdiObjects),
        }));

        newCbdiObjects[uuid] = newRoleObj;
        break;

      case CBDIEditorRootConceptType.TeamConceptType:
      case CBDIEditorRootConceptType.AgentConceptType:
        const newAgentObj = { ...object } as CBDIEditorProjectTeam | CBDIEditorProjectAgent;
        newAgentObj.action_handlers = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.action_handlers, cbdiObjects);
        newAgentObj.beliefs = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.beliefs, cbdiObjects);
        newAgentObj.plans = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.plans, cbdiObjects);
        newAgentObj.resources = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.resources, cbdiObjects);
        newAgentObj.roles = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.roles, cbdiObjects);
        newAgentObj.message_handlers = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.message_handlers, cbdiObjects);
        newAgentObj.services = getModuleConceptsWithUpdatedNameAndModule(newAgentObj.services, cbdiObjects);
        newAgentObj.goals = newAgentObj.goals.map((goal) => ({
          ...goal,
          ...getModuleConceptWithUpdatedNameAndModule(goal, cbdiObjects),
          startup_tactic: getModuleConceptWithUpdatedNameAndModule(goal.startup_tactic, cbdiObjects),
        }));

        newCbdiObjects[uuid] = newAgentObj;
        break;

      default:
        break;
    }
  });

  return newCbdiObjects;
};
