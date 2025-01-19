import React from 'react';
import { getAddingModuleConceptOptions, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { EmptyModuleConcept, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectAgent } from 'misc/types/cbdiEdit/cbdiEditModel';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { SelectChangeEvent } from '@mui/material';
import { copyObj } from 'misc/utils/common/commonUtils';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import AgentDetailGoalListItem from './AgentDetailGoalListItem';

export default function AgentDetailGoalListView({ moduleConcept }: { moduleConcept: ModuleConcept }) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const agentObj = React.useMemo(() => {
    const magentObj = getObjectByModuleConcept(current, moduleConcept) as CBDIEditorProjectAgent | undefined;
    return magentObj;
  }, [current, moduleConcept]);

  const moduleConceptToAdd = React.useMemo(() => {
    if (agentObj) {
      const moduleConceptsInList = agentObj.goals;
      const addingModuleConceptOptions = getAddingModuleConceptOptions(moduleConceptsInList, 'goals', current!);
      return [...new Set(addingModuleConceptOptions)];
    }
    return [];
  }, [agentObj, current]);

  if (!agentObj) {
    return null;
  }
  /* -------------------------------- Callbacks ------------------------------- */
  const onAddItem = (moduleConceptString: string) => {
    const mmoduleConcept: ModuleConcept = JSON.parse(moduleConceptString);
    const newAgentObj = copyObj(agentObj);

    newAgentObj.goals.unshift({
      ...mmoduleConcept,
      startup_goal: false,
      startup_tactic: EmptyModuleConcept,
    });
    dispatch(updateObjects(newAgentObj));
  };
  return (
    <div>
      <ThemedSelect
        value=""
        label="Select a goal"
        onChange={(event: SelectChangeEvent<unknown>) => {
          onAddItem(event.target.value as string);
        }}
      >
        <ThemedMenuItem disabled value="">
          Add a goal
        </ThemedMenuItem>
        {moduleConceptToAdd.map((mmoduleConcept: ModuleConcept, index: number) => {
          const mobject = getObjectByModuleConcept(current!, mmoduleConcept);
          const prefix = `${mmoduleConcept.module}::`;

          if (mobject && mobject._mod !== Mod.Deletion) {
            return (
              <ThemedMenuItem key={index as number} value={JSON.stringify(mmoduleConcept)}>
                <span title={prefix + mobject.name}>{mobject.name}</span>
              </ThemedMenuItem>
            );
          }
          return null;
        })}
      </ThemedSelect>
      {agentObj.goals.map((agentGoal, index) => (
        <AgentDetailGoalListItem key={index as number} agentGoal={agentGoal} agentObj={agentObj} />
      ))}
    </div>
  );
}
