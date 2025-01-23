/* eslint-disable no-case-declarations */
import {
  CBDIEditorOverrideMessageFieldSchema,
  CBDIEditorProject,
  CBDIEditorProjectAction,
  CBDIEditorProjectGoal,
  CBDIEditorProjectMessage,
} from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  ModuleConcept,
  IMapping,
  IPlanNode,
  PlanEditorNodeType,
  OptionData,
  Mod,
  CBDIEditorRootConceptType,
  CBDIEditorPlanNodeType,
  EmptyModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { capitalize, getAllObjOptionsForSingleItem, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';

import { v4 } from 'uuid';

export const getConceptDefaultMappings = (currentProject: CBDIEditorProject, objectType: string, moduleConcept: ModuleConcept) => {
  let messageFields: CBDIEditorOverrideMessageFieldSchema[] = [];
  let msgObj: CBDIEditorProjectMessage | undefined;
  const obj = getObjectByModuleConcept(currentProject, moduleConcept);
  // get message of concept
  if (objectType === CBDIEditorRootConceptType.ActionConceptType && obj && (obj as CBDIEditorProjectAction).request) {
    const msgConcept = (obj as CBDIEditorProjectAction).request!;
    msgObj = getObjectByModuleConcept(currentProject, msgConcept) as CBDIEditorProjectMessage;
  } else if (objectType === CBDIEditorRootConceptType.GoalConceptType && obj && (obj as CBDIEditorProjectGoal).message) {
    const msgConcept = (obj as CBDIEditorProjectGoal).message!;
    msgObj = getObjectByModuleConcept(currentProject, msgConcept) as CBDIEditorProjectMessage;
  }

  // get parameters of above message
  if (msgObj) {
    messageFields = msgObj.fields;
  }
  const defaultMappings: IMapping[] = [];
  // TODO
  // temporary make default mapping to be empty string
  messageFields.forEach((param) => {
    defaultMappings.push({ from: '', to: param.name });
  });
  return defaultMappings;
};

export const getAddingTask = (
  currentProject: CBDIEditorProject,
  objectType: CBDIEditorPlanNodeType,
  inputValue: string,
  moduleConcept: ModuleConcept | undefined,
) => {
  let task!: IPlanNode;
  const defaultMappingDic: IMapping[] | undefined = moduleConcept ? getConceptDefaultMappings(currentProject, objectType, moduleConcept) : undefined;
  const id = v4();

  switch (objectType) {
    case CBDIEditorPlanNodeType.ActionPlanNodeType:
      task = {
        nodeId: id,
        nodeData: {
          id,
          note: '',
          type: objectType,
          async: false,
          action: moduleConcept,
          mappings: defaultMappingDic,
        },
        type: PlanEditorNodeType.Rectangle,
      };

      break;
    case CBDIEditorPlanNodeType.GoalPlanNodeType:
      task = {
        nodeId: id,
        nodeData: {
          id,
          note: '',
          type: objectType,
          async: false,
          goal: moduleConcept,
          mappings: defaultMappingDic,
        },
        type: PlanEditorNodeType.Rectangle,
      };

      break;
    case CBDIEditorPlanNodeType.SleepPlanNodeType:
      task = {
        nodeId: id,
        nodeData: {
          id,
          note: '',
          type: objectType,
          duration: Number.isNaN(Number(inputValue)) ? 0 : Number(inputValue),
        },
        type: PlanEditorNodeType.Rectangle,
      };
      break;
    case CBDIEditorPlanNodeType.ConditionPlanNodeType:
      task = {
        nodeId: id,
        nodeData: {
          id,
          note: '',
          type: objectType,
          conditiontext: inputValue,
        },
        type: PlanEditorNodeType.Rectangle,
      };
      break;

    default:
      break;
  }
  return task;
};

const defaultOptions: OptionData[] = [
  {
    value: `create-action`,
    label: `Create Action`,
    objectType: CBDIEditorPlanNodeType.ActionPlanNodeType,
  },
  {
    value: `create-goal`,
    label: `Create Goal`,
    objectType: CBDIEditorPlanNodeType.GoalPlanNodeType,
  },
  {
    value: `create-condition`,
    label: `Create Condition`,
    objectType: CBDIEditorPlanNodeType.ConditionPlanNodeType,
  },
  {
    value: `create-sleep`,
    label: `Create Sleep`,
    objectType: CBDIEditorPlanNodeType.SleepPlanNodeType,
  },
  {
    value: 'Empty Action',
    label: 'Empty Action',
    objectType: CBDIEditorPlanNodeType.ActionPlanNodeType,
    moduleConcept: EmptyModuleConcept,
  },
  {
    value: 'Empty Goal',
    label: 'Empty Goal',
    objectType: CBDIEditorPlanNodeType.GoalPlanNodeType,
    moduleConcept: EmptyModuleConcept,
  },
  {
    value: 'Empty Sleep',
    label: 'Empty Sleep',
    objectType: CBDIEditorPlanNodeType.SleepPlanNodeType,
  },
  {
    value: 'Empty Condition',
    label: 'Empty Condition',
    objectType: CBDIEditorPlanNodeType.ConditionPlanNodeType,
  },
];

// options for create new concept
const createNewConceptArr = [
  CBDIEditorRootConceptType.TeamConceptType,
  CBDIEditorRootConceptType.AgentConceptType,
  CBDIEditorRootConceptType.RoleConceptType,
  CBDIEditorRootConceptType.GoalConceptType,
  CBDIEditorRootConceptType.ServiceConceptType,
  CBDIEditorRootConceptType.ActionConceptType,
  CBDIEditorRootConceptType.MessageConceptType,
  CBDIEditorRootConceptType.ResourceConceptType,
  CBDIEditorRootConceptType.EnumConceptType,
  CBDIEditorRootConceptType.EntityConceptType,
  CBDIEditorRootConceptType.EventConceptType,
];

const createNewConceptOptions: OptionData[] = createNewConceptArr.map((conceptType) => ({
  value: `create-${conceptType}`,
  label: `Create ${capitalize(conceptType)}`,
  objectType: conceptType,
}));

const createNewConceptArrWithPlanTactic = [
  CBDIEditorRootConceptType.TeamConceptType,
  CBDIEditorRootConceptType.AgentConceptType,
  CBDIEditorRootConceptType.RoleConceptType,
  CBDIEditorRootConceptType.GoalConceptType,
  CBDIEditorRootConceptType.PlanConceptType,
  CBDIEditorRootConceptType.TacticConceptType,
  CBDIEditorRootConceptType.ServiceConceptType,
  CBDIEditorRootConceptType.ActionConceptType,
  CBDIEditorRootConceptType.MessageConceptType,
  CBDIEditorRootConceptType.ResourceConceptType,
  CBDIEditorRootConceptType.EnumConceptType,
  CBDIEditorRootConceptType.EntityConceptType,
  CBDIEditorRootConceptType.EventConceptType,
];

const createNewConceptOptionsWithPlanTactic: OptionData[] = createNewConceptArrWithPlanTactic.map((conceptType) => ({
  value: `create-${conceptType}`,
  label: `Create ${capitalize(conceptType)}`,
  objectType: conceptType,
}));

export const getDefaultOptions = (
  mode: 'conceptSwitching' | 'taskAdding',
  currentPorject: CBDIEditorProject | null,
  selectedTreeNodeConcept: ModuleConcept | null,
  inputValue: string,
) => {
  if (currentPorject === null) {
    return [];
  }
  if (mode === 'conceptSwitching') {
    const options: OptionData[] = (() => {
      const selectedTreeNode = getObjectByModuleConcept(currentPorject, selectedTreeNodeConcept);
      if (
        selectedTreeNode?._mod !== Mod.Deletion &&
        (selectedTreeNode?._objectType === CBDIEditorRootConceptType.GoalConceptType ||
          selectedTreeNode?._objectType === CBDIEditorRootConceptType.PlanConceptType ||
          selectedTreeNode?._objectType === CBDIEditorRootConceptType.TacticConceptType)
      ) {
        return [...createNewConceptOptionsWithPlanTactic];
      }
      return [...createNewConceptOptions];
    })();
    return options;
  }
  // if input value cannot be coverted to number, exclude create new sleep
  if (Number.isNaN(Number(inputValue))) {
    return defaultOptions.filter((el) => el.value !== 'create-sleep');
  }
  return defaultOptions;
};

export const getOptions = (mode: 'conceptSwitching' | 'taskAdding', currentPorject: CBDIEditorProject) => {
  const cbdiObjectDic = currentPorject.cbdiObjects;
  const options: OptionData[] = [];

  if (mode === 'conceptSwitching') {
    Object.keys(cbdiObjectDic).forEach((key) => {
      const object = cbdiObjectDic[key];
      if (object._mod !== Mod.Deletion) {
        const moduleConcept = {
          uuid: object.uuid,
          module: object.module,
          name: object.name,
        };
        const option: OptionData = {
          value: JSON.stringify(moduleConcept),
          moduleConcept,
          label: `${object.name}`,
          objectType: object._objectType,
        };
        options.push(option);
      }
    });

    return options;
  }

  const allActions = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.ActionConceptType, currentPorject);
  const allGoals = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.GoalConceptType, currentPorject);
  const allTasks = allActions.concat(allGoals);
  allTasks.forEach((taskConcept) => {
    const object = getObjectByModuleConcept(currentPorject, taskConcept);
    if (object && object._mod !== Mod.Deletion) {
      const option: OptionData = {
        value: JSON.stringify(taskConcept),
        moduleConcept: taskConcept,
        label: `${object.name}`,
        objectType: object._objectType,
      };
      options.push(option);
    }
  });

  return options;
};

const regex = /\+(.*?)(>|$)/;

export const filterCreateOption = (
  mode: 'conceptSwitching' | 'taskAdding',
  options: OptionData[],
  option: OptionData,
  searchValue: string,
): boolean => {
  if (option.value === `create-${option.objectType}`) {
    // task Adding mode
    if (mode === 'taskAdding') {
      if (searchValue.trim().length === 0 || options.find((moption) => moption.label === searchValue)) {
        return false;
      }
      return true;
    }
    // concept switching mode
    const index = searchValue.indexOf('>');
    const conceptName = index > -1 ? searchValue.substring(index + 1).trim() : '';
    if (!searchValue.startsWith('+') || options.find((moption) => moption.label === conceptName)) {
      return false;
    }
    const match = searchValue.match(regex);
    if (match && match.length >= 2) {
      const newSearchValue = match[1].toLocaleLowerCase();
      return option.objectType.toLowerCase().includes(newSearchValue);
    }
    return false;
  }
  return true;
};
