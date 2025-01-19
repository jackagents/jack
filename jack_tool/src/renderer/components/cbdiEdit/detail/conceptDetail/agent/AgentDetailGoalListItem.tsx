/* eslint-disable no-param-reassign */
import React from 'react';
import { Button, SelectChangeEvent, styled } from '@mui/material';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { Report as ReportIcon, Delete as DeleteIcon } from '@mui/icons-material';
import { getAllObjOptionsForSingleItem, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorAgentGoal, CBDIEditorRootConceptType, EmptyModuleConcept, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectAgent, CBDIEditorProjectTactic, CBDIEditorProjectTeam } from 'misc/types/cbdiEdit/cbdiEditModel';
import { areModuleConceptsEqual, copyObj } from 'misc/utils/common/commonUtils';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('li')(({ theme }) => ({
  width: '100%',
  paddingLeft: 5,
  paddingTop: 5,
  backgroundColor: 'transparent',
  borderTop: `0.5px solid ${theme.editor.detailView.textColor}`,
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
}));

const TextIconContainer = styled('div')({
  gap: 5,
  overflow: 'hidden',
  whiteSpace: 'nowrap',
  paddingTop: 4,
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
});

const StartupContainer = styled('div')({
  display: 'flex',
  flexBasis: 260,
  flexDirection: 'column',
  flexShrink: 0,
  overflow: 'hidden',
  whiteSpace: 'nowrap',
  fontSize: '.9em',
});

const StartupSubContainer = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  paddingRight: 4,
  gap: 10,
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontSize: '.9em',
  overflow: 'hidden',
  textOverflow: 'ellipsis ',
  cursor: 'pointer',
});

const RemoveButton = styled(Button)(({ theme }) => ({
  minWidth: 22,
  width: 22,
  height: 22,
  padding: 0,
  color: theme.editor.detailView.textColor,
}));

/* -------------------------- AgentDetailGoalListItem ------------------------- */

interface Props {
  agentGoal: CBDIEditorAgentGoal;
  agentObj: CBDIEditorProjectAgent | CBDIEditorProjectTeam;
}

function AgentDetailGoalListItem({ agentGoal, agentObj }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects, setSelectedTreeNodeConcept, setGraphSelectedNode } = cbdiEditActions;
  /* ------------------------------ useMeno hooks ----------------------------- */
  const [object, prefix, allTacticOptions] = React.useMemo(() => {
    if (current) {
      const mobject = getObjectByModuleConcept(current, agentGoal);
      const mprefix = `${agentGoal.module}::`;
      const allTactics = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.TacticConceptType, current!);
      const mallTacticOptions = allTactics
        .map((tacticReferConcept) => {
          const tacticObj = getObjectByModuleConcept(current!, tacticReferConcept) as CBDIEditorProjectTactic | undefined;
          if (tacticObj && areModuleConceptsEqual(tacticObj.goal, agentGoal)) {
            return tacticReferConcept;
          }
          return undefined;
        })
        .filter((item) => item !== undefined) as ModuleConcept[];
      return [mobject, mprefix, mallTacticOptions];
    }
    return [undefined, '', []];
  }, [current, agentGoal]);

  const isObjectValid = React.useMemo(() => {
    if (!object || object._mod === Mod.Deletion) {
      return false;
    }
    return true;
  }, [object]);
  /* -------------------------------- Callbacks ------------------------------- */

  const onRemoveItem = () => {
    const newObj = copyObj(agentObj);
    const removingIndex = newObj.goals.findIndex((i: ModuleConcept) => areModuleConceptsEqual(i, agentGoal));
    if (removingIndex > -1) {
      newObj.goals.splice(removingIndex, 1);
    }

    dispatch(updateObjects(newObj));
  };

  const onStartupTacticChange = (moduleConceptId: string) => {
    const newAgentObj = copyObj(agentObj);
    const newTactic = allTacticOptions.find((el) => el.uuid === moduleConceptId) || EmptyModuleConcept;

    newAgentObj.goals.forEach((item) => {
      if (areModuleConceptsEqual(item, agentGoal)) {
        item.startup_tactic = newTactic;
      }
    });
    dispatch(updateObjects(newAgentObj));
  };

  const onToggleStartupGoal: any = () => {
    const newAgentObj = copyObj(agentObj);
    newAgentObj.goals.forEach((i: CBDIEditorAgentGoal) => {
      if (areModuleConceptsEqual(i, agentGoal)) {
        i.startup_goal = !i.startup_goal;
      }
    });
    dispatch(updateObjects(newAgentObj));
  };

  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <TextIconContainer>
        <div style={{ display: 'flex', alignItems: 'center' }}>
          {object && object._mod !== Mod.Deletion ? (
            <img alt="" src={nodeIcon[object._objectType]} style={{ width: 22, height: 22, padding: 2 }} />
          ) : (
            <ReportIcon style={{ color: '#d40b0b', padding: 3 }} />
          )}
          <TextView
            onClick={() => {
              if (isObjectValid) {
                dispatch(setSelectedTreeNodeConcept(agentGoal));
                dispatch(setGraphSelectedNode(agentGoal));
              }
            }}
            title={object ? prefix + object.name : prefix + agentGoal.name}
            className={!isObjectValid ? 'editor-missing' : undefined}
          >
            {object ? object.name : agentGoal.name}
          </TextView>
        </div>
        <RemoveButton onClick={onRemoveItem}>
          <DeleteIcon style={{ padding: 2 }} />
        </RemoveButton>
      </TextIconContainer>
      <StartupContainer>
        <StartupSubContainer>
          <div>Startup Tactic</div>
          <ThemedSelect
            hasborder="true"
            value={agentGoal.startup_tactic.uuid}
            onChange={(event: SelectChangeEvent<unknown>) => {
              onStartupTacticChange(event.target.value as string);
            }}
          >
            <ThemedMenuItem value={EmptyModuleConcept.uuid}>No tactic</ThemedMenuItem>
            {allTacticOptions.map((optionConcept, index) => {
              const optionObj = getObjectByModuleConcept(current!, optionConcept);
              const mprefix = `${optionConcept.module}::`;

              if (optionObj) {
                const isMissing = optionObj._mod === Mod.Deletion;

                return (
                  <ThemedMenuItem key={index as number} value={optionConcept.uuid} disabled={isMissing}>
                    <span title={mprefix + optionObj.name} className={isMissing ? 'editor-missing' : undefined}>
                      {optionObj.name}
                    </span>
                  </ThemedMenuItem>
                );
              }
              return null;
            })}
            {!areModuleConceptsEqual(agentGoal.startup_tactic, EmptyModuleConcept) &&
            !getObjectByModuleConcept(current!, agentGoal.startup_tactic) ? (
              <ThemedMenuItem value={agentGoal.startup_tactic.uuid} disabled>
                <span title={agentGoal.startup_tactic.name} className="editor-missing">
                  {agentGoal.startup_tactic.name}
                </span>
              </ThemedMenuItem>
            ) : null}
          </ThemedSelect>
        </StartupSubContainer>
        <BooleanValueToggler onToggle={onToggleStartupGoal} currentValue={agentGoal.startup_goal} label="Startup Goal" />
      </StartupContainer>
    </Root>
  );
}

export default React.memo(AgentDetailGoalListItem);
