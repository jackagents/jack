/* eslint-disable react/no-array-index-key */
import React from 'react';
import { SelectChangeEvent } from '@mui/material';
import { Fluid } from 'components/common/base/BaseContainer';
import { copy, getAllObjOptionsForSingleItem, getObjectByModuleConcept, sortModuleConceptList } from 'misc/utils/cbdiEdit/Helpers';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { CBDIEditorProjectAgent, CBDIEditorProjectPlan, CBDIEditorProjectTeam } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorObject, CBDIEditorRootConceptType, EmptyModuleConcept, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';

/* ---------------------------- DetailConceptSingleView ---------------------------- */

interface Props {
  moduleConcept: ModuleConcept;
  field: string;
  optionType: CBDIEditorRootConceptType;
  canBeEmpty: boolean;
  customOptions?: ModuleConcept[];
}

function DetailConceptSingleView({ moduleConcept, field, optionType, canBeEmpty, customOptions }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const [object, currentFieldModuleConcept, allOptions] = React.useMemo(() => {
    const mobject = getObjectByModuleConcept(current!, moduleConcept) as any;
    const mcurrentFieldModuleConcept = mobject[field] as ModuleConcept;

    let mallOptions = customOptions || getAllObjOptionsForSingleItem(optionType, current!);
    mallOptions = sortModuleConceptList(mallOptions, current!);
    return [mobject as CBDIEditorObject, mcurrentFieldModuleConcept, mallOptions];
  }, [current, moduleConcept]);

  /* -------------------------------- Callbacks ------------------------------- */

  const onChange = (moduleConceptId: string) => {
    const updatingObjs: CBDIEditorObject[] = [];
    const newObject = copy(object);
    const newModuleConcept = allOptions.find((el) => el.uuid === moduleConceptId);
    if (newModuleConcept) {
      // if it is editing plan's handles
      // update team/agent (having this plan)'s agent goal
      if (object._objectType === CBDIEditorRootConceptType.PlanConceptType && field === 'handles') {
        const oldGoalModuleConcept = (object as CBDIEditorProjectPlan).handles;
        if (oldGoalModuleConcept.uuid !== moduleConceptId) {
          // loop all teams and agents
          [...current!.teams, ...current!.agents].forEach((agentModuleConcept) => {
            const obj = getObjectByModuleConcept(current!, agentModuleConcept) as CBDIEditorProjectAgent | CBDIEditorProjectTeam | undefined;
            // if obj exists and not deleted
            if (obj && obj._mod !== Mod.Deletion) {
              const newObj = copy(obj) as CBDIEditorProjectAgent | CBDIEditorProjectTeam;
              // if team/agent has current plan
              if (obj.plans.find((plan) => areModuleConceptsEqual(plan, agentModuleConcept))) {
                // get team/agent's plans excluding current plan
                const allPlans = obj.plans.filter((item) => !areModuleConceptsEqual(item, agentModuleConcept));
                // count number of plans which has same handle goal as current plan's old handle goal
                const sameOldGoalPlanCount = allPlans.reduce((accumulator, currentValue) => {
                  const currentPlanObj = getObjectByModuleConcept(current, currentValue) as CBDIEditorProjectPlan | undefined;
                  if (currentPlanObj && areModuleConceptsEqual(currentPlanObj.handles, oldGoalModuleConcept)) {
                    return accumulator + 1;
                  }
                  return accumulator;
                }, 0);
                // count number of plans which has same handle goal as current plan's new handle goal
                const sameNewGoalPlanCount = allPlans.reduce((accumulator, currentValue) => {
                  const currentPlanObj = getObjectByModuleConcept(current, currentValue) as CBDIEditorProjectPlan | undefined;
                  if (currentPlanObj && areModuleConceptsEqual(currentPlanObj.handles, newModuleConcept)) {
                    return accumulator + 1;
                  }
                  return accumulator;
                }, 0);

                if (sameOldGoalPlanCount === 0 || sameNewGoalPlanCount === 0) {
                  // if this old handle goal does not exists in any plans of a team/agent
                  // delete this old handle goal from this team/agent
                  if (sameOldGoalPlanCount === 0) {
                    newObj.goals = newObj.goals.filter((item) => !areModuleConceptsEqual(item, oldGoalModuleConcept));
                  }
                  // if this new handle goal does not exists in any plans of a team/agent
                  // ass this new handle goal into this team/agent
                  if (sameNewGoalPlanCount === 0) {
                    newObj.goals.push({
                      ...newModuleConcept,
                      startup_goal: false,
                      startup_tactic: EmptyModuleConcept,
                    });
                  }
                  updatingObjs.push(newObj);
                }
              }
            }
          });
        }
      }
      newObject[field] = newModuleConcept;
    } else {
      newObject[field] = EmptyModuleConcept;
    }

    updatingObjs.push(newObject);

    dispatch(updateObjects(updatingObjs));
  };

  return (
    <Fluid>
      <ThemedSelect
        value={currentFieldModuleConcept?.uuid || EmptyModuleConcept.uuid}
        onChange={(event: SelectChangeEvent<unknown>) => {
          onChange(event.target.value as string);
        }}
      >
        {canBeEmpty ? <ThemedMenuItem value={EmptyModuleConcept.uuid}>Select a {field}</ThemedMenuItem> : null}
        {allOptions.map((optionConcept, index) => {
          const optionObj = getObjectByModuleConcept(current!, optionConcept);
          const prefix = `${optionConcept.module}::`;

          if (optionObj) {
            const isDeleted = optionObj._mod === Mod.Deletion;
            if (isDeleted && currentFieldModuleConcept.uuid !== optionConcept.uuid) {
              return null;
            }
            return (
              <ThemedMenuItem key={index} value={optionConcept.uuid} disabled={isDeleted}>
                <span title={prefix + optionObj.name} className={isDeleted ? 'editor-missing' : undefined}>
                  {optionObj.name}
                </span>
              </ThemedMenuItem>
            );
          }
          return null;
        })}
        {currentFieldModuleConcept &&
        !areModuleConceptsEqual(currentFieldModuleConcept, EmptyModuleConcept) &&
        !getObjectByModuleConcept(current!, currentFieldModuleConcept) ? (
          <ThemedMenuItem value={currentFieldModuleConcept.uuid} disabled>
            <span className="editor-missing">{currentFieldModuleConcept.name}</span>
          </ThemedMenuItem>
        ) : null}
      </ThemedSelect>
    </Fluid>
  );
}

export default React.memo(DetailConceptSingleView);
