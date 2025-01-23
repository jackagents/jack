/* eslint-disable no-nested-ternary */
/* eslint-disable @typescript-eslint/no-shadow */
/* eslint-disable react/no-array-index-key */
import React from 'react';
import {
  CBDIEditorOverrideMessageFieldSchema,
  CBDIEditorProjectAction,
  CBDIEditorProjectGoal,
  CBDIEditorProjectMessage,
  CBDIEditorProjectPlan,
} from 'types/cbdiEdit/cbdiEditModel';
import { FlexCol, FlexRow, Fluid, List } from 'components/common/base/BaseContainer';
import { styled, Tooltip, SelectChangeEvent } from '@mui/material';
import { copy, getAllObjOptionsForSingleItem, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import TextEdit from 'components/common/textEdit/TextEdit';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedFieldWithBorder, ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import {
  IPlanNode,
  ModuleConcept,
  IMapping,
  Mod,
  CBDIEditorRootConceptType,
  CBDIEditorPlanNodeType,
  EmptyModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import { NodeWithDistanceDic, NodesWithFieldsDistance, findBranchesAndCreateNodeDictionary } from './helper';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)({
  height: '100%',
  padding: '0 5px',
  fontSize: '.9em',
});

const Row = styled('li')({});

const FieldRow = styled(FlexRow)({
  width: '100%',
  marginBottom: 5,
  '& > *': {
    flex: 1,
  },
});

const TextSlot = styled(FlexCol)({
  justifyContent: 'center',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

const FieldTextView = styled(TextView)({
  overflow: 'hidden',
  whiteSpace: 'nowrap',
  textOverflow: 'ellipsis',
});

/* ----------------------------- TaskDetailTask ----------------------------- */

// for field of reply message, it will be mapped by action/goal in the upstream of plan
interface FieldTaskMappingDic {
  [field: string]: {
    taskId: string | undefined;
    type: CBDIEditorRootConceptType.ActionConceptType | CBDIEditorRootConceptType.GoalConceptType | undefined;
    replyMsgFieldName: string;
  };
}

interface Props {
  task: IPlanNode;
}

function TaskDetailView({ task }: Props) {
  const { nodeId, nodeData } = task;
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const actionNodesContextArr = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.graph.actionNodesContextArr);
  const dispatch = useDispatch();
  const { updatePlan, setActionNodesContextArr } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const plan = React.useMemo(
    () => getObjectByModuleConcept(current!, selectedTreeNodeConcept!) as CBDIEditorProjectPlan,
    [current!, selectedTreeNodeConcept!],
  );

  const [msgObj, actionRpyMsgObj, allActions, allGoals] = React.useMemo(() => {
    const taskModuleConcept = nodeData.action || nodeData.goal;
    const taskObj = taskModuleConcept ? getObjectByModuleConcept(current!, taskModuleConcept) : undefined;
    let taskNodeMessageConcept: ModuleConcept | undefined;
    let mactionRpyMsgObj: CBDIEditorProjectMessage | undefined;
    if (taskObj) {
      if (nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType) {
        if ((taskObj as CBDIEditorProjectAction).request) {
          taskNodeMessageConcept = (taskObj as CBDIEditorProjectAction).request!;
        }
        if ((taskObj as CBDIEditorProjectAction).reply) {
          const actionRpyMsgConcept = (taskObj as CBDIEditorProjectAction).reply!;
          mactionRpyMsgObj = getObjectByModuleConcept(current!, actionRpyMsgConcept);
        }
      } else if (nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType && (taskObj as CBDIEditorProjectGoal).message) {
        taskNodeMessageConcept = (taskObj as CBDIEditorProjectGoal).message!;
      }
    }
    let mmsgObj: CBDIEditorProjectMessage | undefined;
    if (taskNodeMessageConcept) {
      mmsgObj = getObjectByModuleConcept(current!, taskNodeMessageConcept);
    }

    let mallActions = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.ActionConceptType, current!);
    if (nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType && nodeData.action) {
      mallActions.push(nodeData.action);
    }
    // remove duplicated actions
    if (mallActions.length > 1) {
      mallActions = mallActions.filter((value, index, self) => index === self.findIndex((t) => t.uuid === value.uuid && t.module === value.module));
    }
    let mallGoals = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.GoalConceptType, current!);
    if (nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType && nodeData.goal) {
      mallGoals.push(nodeData.goal);
    }
    // remove duplicated goals
    if (mallGoals.length > 1) {
      mallGoals = mallGoals.filter((value, index, self) => index === self.findIndex((t) => t.uuid === value.uuid && t.module === value.module));
    }
    return [mmsgObj, mactionRpyMsgObj, mallActions, mallGoals];
  }, [current, task]);

  const defaultMappingDic = React.useMemo(() => {
    const result: FieldTaskMappingDic = {};
    if (msgObj) {
      const handleGoalObj = getObjectByModuleConcept(current!, plan.handles) as CBDIEditorProjectGoal | undefined;
      const handleGoalMsgObj = getObjectByModuleConcept(current!, handleGoalObj?.message) as CBDIEditorProjectMessage | undefined;
      const goalContextFields = handleGoalMsgObj?.fields || [];

      msgObj?.fields.forEach((field) => {
        let foundNode: NodesWithFieldsDistance | undefined;
        let replyMsgField: CBDIEditorOverrideMessageFieldSchema | undefined;
        const fromValueOfMapping = nodeData.mappings?.find((el) => el.to === field.name)?.from;
        for (let index = 0; index < actionNodesContextArr.length; index++) {
          const element = actionNodesContextArr[index];
          replyMsgField = element.fields.find((el: CBDIEditorOverrideMessageFieldSchema) => el.name === field.name || el.name === fromValueOfMapping);
          if (replyMsgField) {
            foundNode = element;
            break;
          }
        }
        if (foundNode && replyMsgField) {
          result[field.name] = {
            taskId: foundNode.taskId,
            type: foundNode.type,
            replyMsgFieldName: replyMsgField.name,
          };
        } else {
          replyMsgField = goalContextFields.find((el) => el.name === field.name || el.name === fromValueOfMapping);
          if (replyMsgField) {
            result[field.name] = {
              taskId: handleGoalObj!.uuid,
              type: CBDIEditorRootConceptType.GoalConceptType,
              replyMsgFieldName: replyMsgField.name,
            };
          }
        }
      });
    }
    return result;
  }, [msgObj, task, actionNodesContextArr]);

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    let nodesBetweenDic: NodeWithDistanceDic = {};
    if (task.nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType || task.nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType) {
      nodesBetweenDic = findBranchesAndCreateNodeDictionary(plan.tasks, plan.edges, CBDIEditorPlanNodeType.StartPlanNodeType, nodeId);
      Object.keys(nodesBetweenDic).forEach((nodeId) => {
        if (nodesBetweenDic[nodeId].task.nodeData.type !== CBDIEditorPlanNodeType.ActionPlanNodeType) {
          delete nodesBetweenDic[nodeId];
        }
      });
    }

    const nodeWithDistanceArr = Object.values(nodesBetweenDic).sort((a, b) => a.distance - b.distance);

    const nodesWithFieldsDistanceArr = nodeWithDistanceArr
      .map((el) => {
        const message = el.task.nodeData.action || el.task.nodeData.goal;
        if (message) {
          const actionMsg = getObjectByModuleConcept(current!, message) as CBDIEditorProjectAction | undefined;
          if (actionMsg && actionMsg.reply) {
            const replyMsg = getObjectByModuleConcept(current!, actionMsg.reply) as CBDIEditorProjectMessage | undefined;
            if (replyMsg) {
              return {
                taskId: el.task.nodeId,
                messageName: actionMsg.name,
                fields: replyMsg.fields,
                distance: el.distance,
                type: CBDIEditorRootConceptType.ActionConceptType,
              };
            }
          }
        }
        return undefined;
      })
      .filter((el) => el !== undefined) as NodesWithFieldsDistance[];
    dispatch(setActionNodesContextArr(nodesWithFieldsDistanceArr));
  }, [plan, nodeId]);

  /* -------------------------------- Callbacks ------------------------------- */

  const onDurationChange = (plan: CBDIEditorProjectPlan, oldTask: IPlanNode, duration: number) => {
    const mtask: IPlanNode = copy(oldTask);
    // eslint-disable-next-line radix
    mtask.nodeData.duration = parseInt(duration.toString());

    const newTasks = plan.tasks.filter((task) => task.nodeId !== oldTask.nodeId);
    newTasks.push(mtask);
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        nodes: newTasks,
      }),
    );
  };

  const onConditiontextChange = (plan: CBDIEditorProjectPlan, oldTask: IPlanNode, conditiontext: string) => {
    const mtask: IPlanNode = copy(oldTask);
    mtask.nodeData.conditiontext = conditiontext;

    const newTasks = plan.tasks.filter((task) => task.nodeId !== oldTask.nodeId);
    newTasks.push(mtask);
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        nodes: newTasks,
      }),
    );
  };

  const onMessageChange = (plan: CBDIEditorProjectPlan, oldTask: IPlanNode, moduleConceptId: string) => {
    const mtask: IPlanNode = copy(oldTask);
    const moduleConcept: ModuleConcept = [...allActions, ...allGoals].find((el) => el.uuid === moduleConceptId) || EmptyModuleConcept;

    const newTaskMessageObj = getObjectByModuleConcept(current!, moduleConcept) as any;

    // change task action/goal
    if (nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType) {
      mtask.nodeData.action = moduleConcept;
    } else if (nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType) {
      mtask.nodeData.goal = moduleConcept;
    }
    // if task's new message(action/goal) have its message, get its mapping
    if (!areModuleConceptsEqual(EmptyModuleConcept, moduleConcept)) {
      const defaultMappings: IMapping[] = [];
      let taskNodeMessageConcept: ModuleConcept | undefined;
      if (mtask.nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType && (newTaskMessageObj as CBDIEditorProjectAction).request) {
        taskNodeMessageConcept = (newTaskMessageObj as CBDIEditorProjectAction).request!;
      } else if (mtask.nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType && (newTaskMessageObj as CBDIEditorProjectGoal).message) {
        taskNodeMessageConcept = (newTaskMessageObj as CBDIEditorProjectGoal).message!;
      }
      if (taskNodeMessageConcept) {
        const taskNodeMessage = getObjectByModuleConcept(current!, taskNodeMessageConcept) as CBDIEditorProjectMessage | undefined;
        if (taskNodeMessage) {
          taskNodeMessage.fields.forEach((field: CBDIEditorOverrideMessageFieldSchema) => {
            // TODO
            // temporary make deafult to be empty string
            const mapping = {
              from: '',
              to: field.name,
            };
            defaultMappings.push(mapping);
          });
        }
      }
      mtask.nodeData.mappings = defaultMappings;
    }

    mtask.nodeData.async = false;

    const newTasks = plan.tasks.filter((task) => task.nodeId !== oldTask.nodeId);
    newTasks.push(mtask);
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        nodes: newTasks,
      }),
    );
  };

  const onNoteChange = (newNote: string) => {
    const processedNote = newNote.replace(/\s+$/, '');

    const newTasks = plan.tasks.map((mtask) => {
      if (mtask.nodeId === task.nodeId) {
        return {
          ...mtask,
          nodeData: { ...mtask.nodeData, note: processedNote },
        };
      }
      return mtask;
    });
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        nodes: newTasks,
      }),
    );
  };

  const onMappingsChange = (plan: CBDIEditorProjectPlan, oldTask: IPlanNode, name: string, text: string) => {
    const task: IPlanNode = copy(oldTask);
    // if text is '', delete that mapping
    if (text === '') {
      if (task.nodeData.mappings) {
        const index = task.nodeData.mappings!.findIndex((mapping) => mapping.to === name);
        if (index > -1) {
          task.nodeData.mappings!.splice(index, 1);
        }
      }
    }
    // if text is not '', push/update that mapping
    else if (task.nodeData.mappings) {
      const index = task.nodeData.mappings!.findIndex((mapping) => mapping.to === name);
      if (index > -1) {
        task.nodeData.mappings![index] = {
          from: text,
          to: name,
        };
      } else {
        task.nodeData.mappings!.push({
          from: text,
          to: name,
        });
      }
    } else {
      task.nodeData.mappings = [
        {
          from: text,
          to: name,
        },
      ];
    }

    const newTasks = plan.tasks.filter((task) => task.nodeId !== oldTask.nodeId);
    newTasks.push(task);
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        nodes: newTasks,
      }),
    );
  };

  const onToggleAsync = (plan: CBDIEditorProjectPlan, oldTask: IPlanNode) => {
    const task: IPlanNode = copy(oldTask);
    task.nodeData.async = !task.nodeData.async;
    const newTasks = plan.tasks.filter((task) => task.nodeId !== oldTask.nodeId);
    newTasks.push(task);
    dispatch(
      updatePlan({
        planRerferConcept: selectedTreeNodeConcept!,
        nodes: newTasks,
      }),
    );
  };
  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <List
        style={{
          paddingTop: 5,
          borderBottom: 'thin solid #ffffff20',
        }}
      >
        {nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType ? (
          <Row>
            <TextSlot>
              <FieldRow>
                <FieldTextView>Action</FieldTextView>
                <ThemedSelect
                  hasborder="true"
                  value={nodeData.action?.uuid || EmptyModuleConcept.uuid}
                  onChange={(event: SelectChangeEvent<unknown>) => {
                    onMessageChange(plan, task, event.target.value as string);
                  }}
                >
                  <ThemedMenuItem value={EmptyModuleConcept.uuid}>Please select an action</ThemedMenuItem>
                  {allActions.map((actionConcept, index) => {
                    const object = getObjectByModuleConcept(current!, actionConcept);
                    const prefix = `${actionConcept.module}::`;
                    if (object) {
                      const isMissing = object._mod === Mod.Deletion;
                      return (
                        <ThemedMenuItem key={index} value={actionConcept.uuid} disabled={isMissing}>
                          <span className={isMissing ? 'editor-missing' : undefined} title={prefix + object.name}>
                            {object.name}
                          </span>
                        </ThemedMenuItem>
                      );
                    }
                    return null;
                  })}
                  {nodeData.action &&
                  !areModuleConceptsEqual(nodeData.action, EmptyModuleConcept) &&
                  !getObjectByModuleConcept(current!, nodeData.action) ? (
                    <ThemedMenuItem value={nodeData.action.uuid} disabled>
                      <span className="editor-missing">{nodeData.action.name}</span>
                    </ThemedMenuItem>
                  ) : null}
                </ThemedSelect>
              </FieldRow>
              <FieldRow>
                <FieldTextView>Request Message</FieldTextView>
                <ThemedFieldWithBorder customStyles={{ color: msgObj ? undefined : '#888888' }}>{msgObj?.name || 'None'}</ThemedFieldWithBorder>
              </FieldRow>
              <FieldRow>
                <FieldTextView>Reply Message</FieldTextView>
                <ThemedFieldWithBorder
                  customStyles={{
                    color: actionRpyMsgObj ? undefined : '#888888',
                  }}
                >
                  {actionRpyMsgObj?.name || 'None'}
                </ThemedFieldWithBorder>
              </FieldRow>
            </TextSlot>
          </Row>
        ) : null}
        {nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType ? (
          <Row>
            <TextSlot>
              <FieldRow>
                <FieldTextView>Goal</FieldTextView>
                <ThemedSelect
                  hasborder="true"
                  value={nodeData.goal?.uuid}
                  onChange={(event: SelectChangeEvent<unknown>) => {
                    onMessageChange(plan, task, event.target.value as string);
                  }}
                >
                  <ThemedMenuItem value={EmptyModuleConcept.uuid}>Please select a goal</ThemedMenuItem>
                  {allGoals.map((goalConcept, index) => {
                    const object = getObjectByModuleConcept(current!, goalConcept);
                    const prefix = `${goalConcept.module}::`;
                    if (object) {
                      const isMissing = object._mod === Mod.Deletion;

                      return (
                        <ThemedMenuItem key={index} value={goalConcept.uuid} disabled={isMissing}>
                          <span className={isMissing ? 'editor-missing' : undefined} title={prefix + object.name}>
                            {object.name}
                          </span>
                        </ThemedMenuItem>
                      );
                    }
                    return null;
                  })}
                  {nodeData.goal &&
                  !areModuleConceptsEqual(nodeData.goal, EmptyModuleConcept) &&
                  !getObjectByModuleConcept(current!, nodeData.goal) ? (
                    <ThemedMenuItem value={nodeData.goal.uuid} disabled>
                      <span className="editor-missing">{nodeData.goal.name}</span>
                    </ThemedMenuItem>
                  ) : null}
                </ThemedSelect>
              </FieldRow>
              <FieldRow>
                <FieldTextView>Goal Message</FieldTextView>
                <ThemedFieldWithBorder customStyles={{ color: msgObj ? undefined : '#888888' }}>{msgObj?.name || 'None'}</ThemedFieldWithBorder>
              </FieldRow>
            </TextSlot>
          </Row>
        ) : null}
        {nodeData.type === CBDIEditorPlanNodeType.ActionPlanNodeType || nodeData.type === CBDIEditorPlanNodeType.GoalPlanNodeType ? (
          <BooleanValueToggler
            onToggle={() => {
              onToggleAsync(plan, task);
            }}
            currentValue={!!nodeData.async}
            label="Async"
          />
        ) : null}

        {nodeData.type === CBDIEditorPlanNodeType.SleepPlanNodeType ? (
          <Row>
            <TextSlot>
              <FieldRow>
                <FieldTextView>Duration</FieldTextView>
                <TextEdit
                  text={nodeData.duration ? nodeData.duration : ''}
                  onDoneEditing={(text: string | number) => {
                    onDurationChange(plan, task, text as number);
                  }}
                />
              </FieldRow>
            </TextSlot>
          </Row>
        ) : null}

        {nodeData.type === CBDIEditorPlanNodeType.ConditionPlanNodeType ? (
          <Row>
            <TextSlot>
              <FieldRow>
                <FieldTextView>Condition Text</FieldTextView>
                <TextEdit
                  text={nodeData.conditiontext ? nodeData.conditiontext : ''}
                  onDoneEditing={(text: string | number) => {
                    onConditiontextChange(plan, task, text as string);
                  }}
                  multiLine
                />
              </FieldRow>
            </TextSlot>
          </Row>
        ) : null}

        {msgObj ? (
          <List style={{ paddingTop: 5 }}>
            <Row style={{ marginBottom: 5 }}>
              <TextSlot>
                <FieldRow
                  style={{
                    fontWeight: 'bold',
                  }}
                >
                  Mappings
                </FieldRow>
              </TextSlot>
            </Row>
            {msgObj.fields.map((field, index: number) => {
              // get current mapping value
              const currentFromValue = (() => {
                if (nodeData.mappings) {
                  const mapping = nodeData.mappings.find((mmapping) => mmapping.to === field.name) as IMapping | undefined;
                  if (mapping) {
                    return mapping.from;
                  }
                }
                return '';
              })();

              const [badgeType, presetText] = (() => {
                if (!defaultMappingDic[field.name]) {
                  return [undefined, undefined];
                }
                return [defaultMappingDic[field.name].type, defaultMappingDic[field.name].replyMsgFieldName];
              })();

              return (
                <Row key={index}>
                  <TextSlot>
                    <FieldRow>
                      <Tooltip title={field.note} placement="left" arrow>
                        <FieldTextView>{field.name}</FieldTextView>
                      </Tooltip>
                      <TextEdit
                        text={currentFromValue}
                        onDoneEditing={(text: string | number) => onMappingsChange(plan, task, field.name, text as string)}
                        badgeType={badgeType}
                        presetText={presetText}
                      />
                    </FieldRow>
                  </TextSlot>
                </Row>
              );
            })}
            {msgObj && msgObj.fields.length === 0 ? (
              <Row key={-1}>
                <TextSlot>
                  <FieldRow>No attribute to map</FieldRow>
                </TextSlot>
              </Row>
            ) : null}
          </List>
        ) : null}

        <Row>
          <TextSlot>
            <FieldRow>
              <FieldTextView>Task note</FieldTextView>
              <TextEdit text={nodeData.note} onDoneEditing={(text: string | number) => onNoteChange(text as string)} multiLine />
            </FieldRow>
          </TextSlot>
        </Row>
      </List>
    </Root>
  );
}

export default React.memo(TaskDetailView);
