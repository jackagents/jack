/* eslint-disable @typescript-eslint/naming-convention */
/* eslint-disable no-continue */
/* eslint-disable no-restricted-syntax */
import {
  CBDIEditorConceptSchema,
  CBDIEditorObject,
  CBDIEditorJSONPlanEdge,
  CBDIEditorPlanEditorEdgeCondition,
  CBDIEditorJSONPlanNode,
  CBDIEditorPlanNodeType,
  CBDIEditorRootConceptType,
  CBDIEditorTBasicMessageSchema,
  Mod,
  ModuleConcept,
  IPlanEdge,
  IPlanNode,
  PlanNodeMetaData,
  PlanEditorNodeType,
  Vec2Type,
  PlanEdgeData,
  defaultEdgeControlPoints,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { v4 } from 'uuid';
import {
  CBDIEditorRootObjectDic,
  CBDIEditorRootMessageSchema,
  CBDIEditorRootEnumSchema,
  CBDIEditorRootEventSchema,
  CBDIEditorRootResourceSchema,
  CBDIEditorRootActionSchema,
  CBDIEditorRootGoalSchema,
  CBDIEditorRootServiceSchema,
  CBDIEditorRootEntitySchema,
  CBDIEditorRootPlanSchema,
  CBDIEditorRootTacticSchema,
  CBDIEditorRootRoleSchema,
  CBDIEditorRootAgentSchema,
  InitialEndNode,
  InitialStartNode,
  FlatNodeMetaDataModel,
  CBDIEditorRootTeamSchema,
  CBDIEditorOverridePlanSchema,
  CBDIEditorOverrideMessageSchema,
  CBDIEditorOverrideMessageFieldSchema,
  CBDIEditorProjectMessage,
  CBDIEditorProjectEvent,
  CBDIEditorProjectAction,
  CBDIEditorProjectGoal,
  CBDIEditorProjectService,
  CBDIEditorProjectEntity,
  CBDIEditorProjectPlan,
  CBDIEditorProjectTactic,
  CBDIEditorProjectRole,
  CBDIEditorProjectTeam,
  CBDIEditorProjectAgent,
  CBDIEditorOverrideTacticSchema,
  FlatEdgeMetaDataModel,
  FlatMetaData,
  CBDIEditorModuleWithMetaDataModel,
  CBDIEditorProject,
  CBDIEditorProjectModel,
  ModulePathObjModel,
} from '../../types/cbdiEdit/cbdiEditModel';
import { copy, pareseVec2String, updateCbdiObjectsNameAndModule } from './Helpers';
import packageJson from '../../../../package.json';

/**
 * get node meta data base on node id
 * @param nodeId
 * @param metaDataArrModel
 * @returns
 */
const getNodeMetaDataWithNodeId = (nodeId: string, metaDataArrModel: FlatNodeMetaDataModel[] | undefined): PlanNodeMetaData | undefined => {
  if (!metaDataArrModel) {
    return undefined;
  }
  const matchedMetaData = metaDataArrModel.find((el) => el.NodeId === nodeId) as FlatNodeMetaDataModel | undefined;
  if (!matchedMetaData) {
    return undefined;
  }
  return {
    position: { x: matchedMetaData.PositionX, y: matchedMetaData.PositionY },
    timestamp: matchedMetaData.Timestamp,
  };
};

/**
 * get edge meta data base on source id and target id
 * @param nodeId
 * @param metaDataArrModel
 * @returns
 */
const getEdgeMetaDataWithSourceTarget = (sourceId: string, targetId: string, metaDataArrModel: FlatEdgeMetaDataModel[] | undefined) => {
  if (!metaDataArrModel) {
    return { controlPoints: defaultEdgeControlPoints, timestamp: Date.now() };
  }
  const matchedMetaData = metaDataArrModel.find((el) => el.SourceId === sourceId && el.TargetId === targetId) as FlatEdgeMetaDataModel | undefined;
  if (!matchedMetaData) {
    return { controlPoints: defaultEdgeControlPoints, timestamp: Date.now() };
  }
  return {
    controlPoints: matchedMetaData.ControlPoints,
    timestamp: matchedMetaData.Timestamp,
  };
};

/**
 * register root concept into cbdiObjects dic
 * @param cbdiObjects
 * @param concept
 * @param rootConceptType
 * @param moduleName module name when register module's cbdiObjects dic
 */
const registerRootConcept = (cbdiObjects: CBDIEditorRootObjectDic, concept: CBDIEditorConceptSchema, rootConceptType: CBDIEditorRootConceptType) => {
  // eslint-disable-next-line no-param-reassign
  cbdiObjects[concept.uuid] = {
    ...concept,
    _mod: Mod.None,
    _objectType: rootConceptType,
  };
};

/**
 * @param projectWithMetaDataModel module model with meta data model to be serialised
 * @returns
 */
export function convertModuleProjectFileToProject(
  moduleName: string,
  projectWithMetaDataModel: CBDIEditorModuleWithMetaDataModel,
): CBDIEditorProject {
  const { moduleModel, flatMetaData } = projectWithMetaDataModel;

  /* ------------------ process project version and generator ----------------- */
  // get cbdi version from cbdi config json file
  let cbdiVersion: string;

  // add version, generator info if model does not have
  if (!moduleModel.project.major_version || !moduleModel.project.minor_version || !moduleModel.project.patch_version) {
    cbdiVersion = packageJson.version;
  } else {
    cbdiVersion = [moduleModel.project.major_version, moduleModel.project.minor_version, moduleModel.project.patch_version].join('.');
  }

  if (!moduleModel.project.generator) {
    moduleModel.project.generator = 'JACK Editor';
  }
  /* --------------------------- register cbdi object -------------------------- */
  const cbdiObjects: CBDIEditorRootObjectDic = {};

  const enums: ModuleConcept[] = [];
  moduleModel.enums.forEach((el) => {
    enums.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.EnumConceptType);
  });

  const teams: ModuleConcept[] = [];
  moduleModel.teams.forEach((el) => {
    teams.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.TeamConceptType);
  });

  const agents: ModuleConcept[] = [];
  moduleModel.agents.forEach((el) => {
    agents.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.AgentConceptType);
  });

  const roles: ModuleConcept[] = [];
  moduleModel.roles.forEach((el) => {
    roles.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.RoleConceptType);
  });

  const actions: ModuleConcept[] = [];
  moduleModel.actions.forEach((el) => {
    actions.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.ActionConceptType);
  });

  const goals: ModuleConcept[] = [];
  moduleModel.goals.forEach((el) => {
    goals.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.GoalConceptType);
  });

  const resources: ModuleConcept[] = [];
  moduleModel.resources.forEach((el) => {
    resources.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.ResourceConceptType);
  });
  /* ---------------------------- process messages ---------------------------- */
  const messages: ModuleConcept[] = [];
  moduleModel.messages.forEach((el) => {
    messages.push({ uuid: el.uuid, name: el.name, module: el.module });

    const messageFields = el.fields.map((field) => {
      const mfield = { ...field } as CBDIEditorOverrideMessageFieldSchema;
      // add uuid for each message field
      const id = v4();
      mfield.id = id;
      mfield.isDefaultValid = mfield.default !== '';
      // if default value is not empty string
      // meaning it is valid default value
      if (mfield.default !== '') {
        // if field type if enum
        // convert string name to number value of enum field
        if (typeof mfield.type === 'object' && 'Enum' in mfield.type && typeof mfield.default === 'string') {
          const numberValue = parseInt(mfield.default, 10);
          if (!Number.isNaN(numberValue)) {
            mfield.default = numberValue;
          }
        } else {
          // convert defalt value base on type
          switch (field.type) {
            case CBDIEditorTBasicMessageSchema.BoolType:
              mfield.default = field.default === 'true';
              break;
            case CBDIEditorTBasicMessageSchema.Vec2Type:
              mfield.default = pareseVec2String(field.default);
              break;
            default:
              break;
          }
        }
      }

      return mfield;
    });
    const overrideMessage: CBDIEditorOverrideMessageSchema = {
      ...el,
      fields: messageFields,
    };

    registerRootConcept(cbdiObjects, overrideMessage, CBDIEditorRootConceptType.MessageConceptType);
  });

  const services: ModuleConcept[] = [];
  moduleModel.services.forEach((el) => {
    services.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.ServiceConceptType);
  });

  const entities: ModuleConcept[] = [];
  moduleModel.entities.forEach((el) => {
    entities.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.EntityConceptType);
  });

  const events: ModuleConcept[] = [];
  moduleModel.events.forEach((el) => {
    events.push({ uuid: el.uuid, name: el.name, module: el.module });
    registerRootConcept(cbdiObjects, el, CBDIEditorRootConceptType.EventConceptType);
  });

  /* ----------------------------- Process plans. ----------------------------- */
  const plans: ModuleConcept[] = [];
  for (const planModel of moduleModel.plans) {
    const planId = planModel.uuid;
    const startNode = copy(InitialStartNode) as IPlanNode;
    startNode.nodeId = `start/${planId}`;
    const endNode = copy(InitialEndNode) as IPlanNode;
    endNode.nodeId = `end/${planId}`;
    const tasks: IPlanNode[] = [];
    const edges: IPlanEdge[] = [];
    if (planModel.tasks.length === 0 && planModel.edges.length === 0) {
      startNode.metaData!.timestamp = Date.now();
      endNode.metaData!.timestamp = Date.now();
      tasks.push(startNode, endNode);
      edges.push({
        source: startNode.nodeId,
        target: endNode.nodeId,
        edgeData: {
          condition: CBDIEditorPlanEditorEdgeCondition.True,
          controlPoints: defaultEdgeControlPoints,
          timestamp: Date.now(),
        },
      });
    } else {
      tasks.push(
        {
          ...startNode,
          metaData: getNodeMetaDataWithNodeId(startNode.nodeId, flatMetaData?.node),
        },
        {
          ...endNode,
          metaData: getNodeMetaDataWithNodeId(endNode.nodeId, flatMetaData?.node),
        },
      );

      for (const nodeModel of planModel.tasks) {
        const task: IPlanNode = {
          nodeId: nodeModel.id,
          type: PlanEditorNodeType.Rectangle,
          nodeData: nodeModel,
          metaData: getNodeMetaDataWithNodeId(nodeModel.id, flatMetaData?.node),
        };

        tasks.push(task);
      }

      for (const edgeModel of planModel.edges) {
        let sourceId = edgeModel.sourceid;
        let targetId = edgeModel.targetid;
        // process start and end node id
        if (sourceId === CBDIEditorPlanNodeType.StartPlanNodeType || sourceId === CBDIEditorPlanNodeType.EndPlanNodeType) {
          sourceId = `${edgeModel.sourceid}/${planId}`;
        }

        if (targetId === CBDIEditorPlanNodeType.StartPlanNodeType || targetId === CBDIEditorPlanNodeType.EndPlanNodeType) {
          targetId = `${edgeModel.targetid}/${planId}`;
        }

        if (!tasks.some((node) => node.nodeId === sourceId) || !tasks.some((node) => node.nodeId === targetId)) {
          continue;
        }

        const edgeMetaData = getEdgeMetaDataWithSourceTarget(sourceId, targetId, flatMetaData?.edge);

        const edge: IPlanEdge = {
          source: sourceId,
          target: targetId,
          edgeData: {
            condition: edgeModel.condition,
            ...edgeMetaData,
          },
        };
        edges.push(edge);
      }
    }

    const plan: CBDIEditorOverridePlanSchema = {
      ...planModel,
      handles: planModel.handles,
      tasks,
      edges,
    };
    plans.push({ uuid: plan.uuid, name: plan.name, module: plan.module });
    registerRootConcept(cbdiObjects, plan, CBDIEditorRootConceptType.PlanConceptType);
  }

  /* ----------------------------- Process tactic ----------------------------- */
  const tactics: ModuleConcept[] = [];
  for (const tacticModel of moduleModel.tactics) {
    // add id for tactic's plan list item
    const plan_listWithoutId = tacticModel.plan_list.map((plan) => ({
      id: v4(),
      moduleConcept: plan,
    }));

    // if is loading module and tactic's module is from module itself
    // set module of this tactic hander to be module name
    const tactic: CBDIEditorOverrideTacticSchema = {
      ...tacticModel,
      goal: tacticModel.goal,
      plan_list: plan_listWithoutId,
    };
    tactics.push({ uuid: tactic.uuid, name: tactic.name, module: tactic.module });
    registerRootConcept(cbdiObjects, tactic, CBDIEditorRootConceptType.TacticConceptType);
  }

  /* ------------------------------ Project info ------------------------------ */
  const project: CBDIEditorProject = {
    name: moduleName,
    namespaces: [],
    jack_model_version: '',
    modulePaths: [],
    moduleProjectInfoDic: { [moduleName]: moduleModel.project },
    cbdiObjects,
    enums,
    teams,
    agents,
    tactics,
    roles,
    goals,
    plans,
    resources,
    actions,
    messages,
    services,
    entities,
    events,
  };

  return project;
}

/**
 * Convert node meta data to flat obj
 * @param nodeId
 * @param metaData
 * @returns
 */
const convertNodeMetaDataToFlatObj = (nodeId: string, metaData: PlanNodeMetaData | undefined): FlatNodeMetaDataModel | undefined => {
  if (!metaData) {
    return undefined;
  }
  return {
    NodeId: nodeId,
    PositionX: metaData.position.x,
    PositionY: metaData.position.y,
    Timestamp: metaData.timestamp,
  };
};

/**
 * Convert edge meta data to flat obj
 * @param sourceId
 * @param targetId
 * @param edgeData
 * @returns
 */
const convertEdgeMetaDataToFlatObj = (sourceId: string, targetId: string, edgeData: PlanEdgeData): FlatEdgeMetaDataModel | undefined => {
  if (edgeData.controlPoints.length === 0) {
    return undefined;
  }
  const controlPointsWithoutActive = edgeData.controlPoints.map((ph) => ({
    x: ph.x,
    y: ph.y,
  }));
  return {
    SourceId: sourceId,
    TargetId: targetId,
    ControlPoints: controlPointsWithoutActive,
    Timestamp: edgeData.timestamp,
  };
};

const getRootModuleConceptFromCBDIEditorObject = (cbdiEditorObject: CBDIEditorObject) => {
  const { _mod, _objectType, ...model } = cbdiEditorObject;
  if (_mod === Mod.Deletion) {
    return undefined;
  }
  const { uuid, name, module, note, ...others } = model;
  return { uuid, name, module, note, ...others };
};

/**
 *
 * @param project
 * @param moduleName saving module name
 * @returns
 */
export function saveCBDIJsonFromProject(project: CBDIEditorProject, moduleName: string): CBDIEditorModuleWithMetaDataModel {
  const flatMetaData: FlatMetaData = { node: [], edge: [] };

  // Update cbdiObjects
  // Make sure object and its properties have correct name and module
  const cbdiObjects = updateCbdiObjectsNameAndModule(project.cbdiObjects);
  /* --------------------------- Process enum types --------------------------- */
  const enumModels: CBDIEditorRootEnumSchema[] = [];
  for (const moduleConcept of project.enums) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]);
    if (model === undefined) {
      continue;
    }
    enumModels.push(model as CBDIEditorRootEnumSchema);
  }

  /* ---------------------------- Process messages. --------------------------- */
  const messageModels: CBDIEditorRootMessageSchema[] = [];
  for (const moduleConcept of project.messages) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectMessage;

    if (model === undefined) {
      continue;
    }

    const newFields = model.fields.map((field) => {
      const { id: fieldId, isDefaultValid, ...newField } = field;

      if (isDefaultValid) {
        // if field type if enum
        // convert number value to string name of enum field
        if (field.default !== '' && typeof field.type === 'object' && 'Enum' in field.type) {
          newField.default = field.default.toString();
        } else if (field.type === CBDIEditorTBasicMessageSchema.Vec2Type) {
          newField.default = `${(field.default as Vec2Type).x},${(field.default as Vec2Type).y}`;
        } else if (field.type === CBDIEditorTBasicMessageSchema.BoolType) {
          newField.default = JSON.stringify(field.default);
        }
      } else {
        newField.default = '';
      }
      return newField;
    });
    const newModel = { ...model, fields: newFields };

    messageModels.push(newModel as CBDIEditorRootMessageSchema);
  }

  /* ----------------------------- Process events. ---------------------------- */
  const eventModels: CBDIEditorRootEventSchema[] = [];
  for (const moduleConcept of project.events) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectEvent;
    if (model === undefined) {
      continue;
    }
    model.message = {
      ...model.message,
      // Hack
      // make event's message defaults empty array when save out
      defaults: [],
    };
    eventModels.push(model as CBDIEditorRootEventSchema);
  }
  /* --------------------------- Process resources. --------------------------- */
  const resourceModels: CBDIEditorRootResourceSchema[] = [];
  for (const moduleConcept of project.resources) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]);
    if (model === undefined) {
      continue;
    }
    resourceModels.push(model as CBDIEditorRootResourceSchema);
  }
  /* ---------------------------- Process actions. ---------------------------- */
  const actionModels: CBDIEditorRootActionSchema[] = [];
  for (const moduleConcept of project.actions) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectAction;
    if (model === undefined) {
      continue;
    }
    actionModels.push(model as CBDIEditorRootActionSchema);
  }
  /* ----------------------------- Process goals. ----------------------------- */
  const goalModels: CBDIEditorRootGoalSchema[] = [];
  for (const moduleConcept of project.goals) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectGoal;
    if (model === undefined) {
      continue;
    }
    goalModels.push(model as CBDIEditorRootGoalSchema);
  }

  /* ---------------------------- Process services. --------------------------- */
  const serviceModels: CBDIEditorRootServiceSchema[] = [];
  for (const moduleConcept of project.services) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectService;
    if (model === undefined) {
      continue;
    }

    serviceModels.push(model as CBDIEditorRootServiceSchema);
  }
  /* ---------------------------- Process entities. --------------------------- */
  const entityModels: CBDIEditorRootEntitySchema[] = [];
  for (const moduleConcept of project.entities) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectEntity;
    if (model === undefined) {
      continue;
    }
    entityModels.push(model as CBDIEditorRootEntitySchema);
  }
  /* ----------------------------- Process plans. ----------------------------- */
  const planModels: CBDIEditorRootPlanSchema[] = [];
  for (const moduleConcept of project.plans) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectPlan;
    if (model === undefined) {
      continue;
    }

    const taskModels: CBDIEditorJSONPlanNode[] = [];
    const edgeModels: CBDIEditorJSONPlanEdge[] = [];

    for (const task of model.tasks) {
      const { nodeId, nodeData, metaData } = task;
      // save node flat meta data
      const flatNodeMetaData = convertNodeMetaDataToFlatObj(nodeId, metaData);
      if (flatNodeMetaData) {
        flatMetaData.node.push(flatNodeMetaData);
      }
      // skip start and end node for save out to model
      if (nodeData.type !== CBDIEditorPlanNodeType.StartPlanNodeType && nodeData.type !== CBDIEditorPlanNodeType.EndPlanNodeType) {
        taskModels.push({
          id: nodeId,
          note: nodeData.note,
          type: nodeData.type,
          action: nodeData.action,
          goal: nodeData.goal,
          async: nodeData.async,
          duration: nodeData.duration,
          conditiontext: nodeData.conditiontext,
          mappings: nodeData.mappings,
        });
      }
    }

    for (const edge of (model as CBDIEditorOverridePlanSchema).edges) {
      let source = edge.source;
      let target = edge.target;

      // save edge flat meta data
      const flatEdgeMetaData = convertEdgeMetaDataToFlatObj(source, target, edge.edgeData);
      if (flatEdgeMetaData) {
        flatMetaData.edge.push(flatEdgeMetaData);
      }

      // process start and end node id
      if (source === `${CBDIEditorPlanNodeType.StartPlanNodeType}/${moduleConcept.uuid}`) {
        source = CBDIEditorPlanNodeType.StartPlanNodeType;
      }
      if (target === `${CBDIEditorPlanNodeType.StartPlanNodeType}/${moduleConcept.uuid}`) {
        target = CBDIEditorPlanNodeType.StartPlanNodeType;
      }

      if (source === `${CBDIEditorPlanNodeType.EndPlanNodeType}/${moduleConcept.uuid}`) {
        source = CBDIEditorPlanNodeType.EndPlanNodeType;
      }
      if (target === `${CBDIEditorPlanNodeType.EndPlanNodeType}/${moduleConcept.uuid}`) {
        target = CBDIEditorPlanNodeType.EndPlanNodeType;
      }

      edgeModels.push({
        condition: edge.edgeData.condition,
        sourceid: source,
        targetid: target,
      });
    }

    const planModel = {
      ...model,
      tasks: taskModels,
      edges: edgeModels,
    };
    planModels.push(planModel as CBDIEditorRootPlanSchema);
  }

  /* ---------------------------- Process tactics. ---------------------------- */
  const tacticModels: CBDIEditorRootTacticSchema[] = [];
  for (const moduleConcept of project.tactics) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectTactic;
    if (model === undefined) {
      continue;
    }
    // Remove id of plan from plan_list
    const plan_list = model.plan_list.map((item) => item.moduleConcept);

    const newModel = { ...model, plan_list };
    tacticModels.push(newModel);
  }

  /* ----------------------------- Process roles. ----------------------------- */
  const roleModels: CBDIEditorRootRoleSchema[] = [];
  for (const moduleConcept of project.roles) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectRole;
    if (model === undefined) {
      continue;
    }

    roleModels.push(model as CBDIEditorRootRoleSchema);
  }

  /* ----------------------------- Process teams. ----------------------------- */
  const teamModels: CBDIEditorRootTeamSchema[] = [];
  for (const moduleConcept of project.teams) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectTeam;
    if (model === undefined) {
      continue;
    }

    teamModels.push(model as CBDIEditorRootTeamSchema);
  }

  /* ----------------------------- Process agents. ---------------------------- */
  const agentModels: CBDIEditorRootAgentSchema[] = [];
  for (const moduleConcept of project.agents) {
    if (moduleConcept.module !== moduleName) {
      continue;
    }
    const model = getRootModuleConceptFromCBDIEditorObject(cbdiObjects[moduleConcept.uuid]) as CBDIEditorProjectAgent;
    if (model === undefined) {
      continue;
    }

    agentModels.push(model as CBDIEditorRootAgentSchema);
  }

  /* ------------------------ get module project info------------------------ */

  const projectInfo = project.moduleProjectInfoDic[moduleName];
  const projectModel = {
    project: projectInfo,
    enums: enumModels,
    teams: teamModels,
    agents: agentModels,
    tactics: tacticModels,
    roles: roleModels,
    resources: resourceModels,
    actions: actionModels,
    goals: goalModels,
    plans: planModels,
    messages: messageModels,
    services: serviceModels,
    entities: entityModels,
    events: eventModels,
  };

  return { moduleModel: projectModel, flatMetaData };
}

/**
 * save cbdi editor project file
 */
export function saveCBDIProjectFromProject(project: CBDIEditorProject): CBDIEditorProjectModel {
  const modules: ModulePathObjModel[] = project.modulePaths.map((el) => ({
    name: el.name,
    path: el.path!,
  }));
  return { name: project.name, jack_model_version: project.jack_model_version, modules };
}
