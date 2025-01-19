import { styled } from '@mui/material';
import React from 'react';
import { RootState } from 'projectRedux/Store';
import { useSelector } from 'react-redux';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorProjectGoal, CBDIEditorProjectMessage, CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- Styles --------------------------------- */

const Root = styled('div')(({ theme }) => ({
  height: '100%',
  display: 'flex',
  padding: 20,
  flexDirection: 'column',
  gap: 20,
  border: `2px dashed ${theme.editor.graphView.textColor}`,
  borderRadius: 10,
  color: 'black',
}));

const Table = styled('div')({
  display: 'flex',
  border: '2px solid grey',
  flexDirection: 'column',
  fontWeight: 700,
  background: '#ffffff',
});

const TableTitle = styled('div')({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  fontWeight: 700,
  padding: 5,
  fontSize: 16,
  wordBreak: 'break-all',
});

const TableCell = styled('div')({
  display: 'flex',
  borderTop: '2px solid grey',
  fontWeight: 400,
  fontSize: 12,
  wordBreak: 'break-all',
  padding: 5,
  flexDirection: 'column',
  gap: 5,
});

const SubTitle = styled('div')({
  fontWeight: 600,
  fontSize: 14,
});

function PlanContext() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const actionNodesContextArr = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.graph.actionNodesContextArr);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);

  /* ----------------------------- useState hooks ----------------------------- */
  const [handledGoal, setHandleGoal] = React.useState<CBDIEditorProjectGoal | undefined>(undefined);
  const [handledGoalMsg, setHandleGoalMsg] = React.useState<CBDIEditorProjectMessage | undefined>(undefined);
  const [currentPlan, setCurrentPlan] = React.useState<CBDIEditorProjectPlan | undefined>(undefined);

  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    if (selectedTreeNodeConcept) {
      const selectedConcept = getObjectByModuleConcept(current!, selectedTreeNodeConcept!);
      if (selectedConcept?._objectType === CBDIEditorRootConceptType.PlanConceptType) {
        const mcurrentPlan = selectedConcept as CBDIEditorProjectPlan;
        setCurrentPlan(mcurrentPlan);
        if (mcurrentPlan.handles) {
          const mhandledGoal = getObjectByModuleConcept(current!, mcurrentPlan.handles!) as CBDIEditorProjectGoal;
          setHandleGoal(mhandledGoal);
          if (mhandledGoal) {
            const mhandledGoalMsg = getObjectByModuleConcept(current!, mhandledGoal.message) as CBDIEditorProjectMessage;
            setHandleGoalMsg(mhandledGoalMsg);
          } else {
            setHandleGoalMsg(undefined);
          }
        } else {
          setHandleGoal(undefined);
          setHandleGoalMsg(undefined);
        }
      }
    }
  }, [current, selectedTreeNodeConcept]);

  /* ------------------------------ useMemo hooks ----------------------------- */
  const customMsgNames = React.useMemo(() => {
    if (!current || !handledGoalMsg) {
      return [];
    }
    const result = handledGoalMsg.fields.map((field) => {
      if (typeof field.type === 'object' && 'Custom' in field.type) {
        return field.type.Custom.name;
      }
      return undefined;
    });
    return result.filter((el) => el !== undefined) as string[];
  }, [handledGoalMsg, current]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <Table>
        <TableTitle style={{ backgroundColor: '#f6e794' }}>Goal: {handledGoal ? handledGoal.name : 'No handles'}</TableTitle>
        <TableCell>
          <SubTitle>Parameters</SubTitle>
          {customMsgNames.length === 0 ? (
            <div>No parameters</div>
          ) : (
            customMsgNames.map((fieldName, index) => <div key={index as number}>{fieldName}</div>)
          )}
        </TableCell>
        <TableCell>
          <SubTitle>Pursued When</SubTitle>
          {handledGoal ? handledGoal.precondition.query : null}
        </TableCell>
        <TableCell>
          <SubTitle>Satisfied When</SubTitle>
          {handledGoal ? handledGoal.satisfied.query : null}
        </TableCell>
        <TableCell>
          <SubTitle>Note</SubTitle>
          {handledGoalMsg ? handledGoalMsg.note : null}
        </TableCell>
      </Table>

      <Table>
        <TableTitle style={{ backgroundColor: '#afd2ff' }}>Plan: {currentPlan ? currentPlan.name : 'No Plan'}</TableTitle>
        <TableCell>
          <SubTitle>Valid When</SubTitle>
          {currentPlan ? currentPlan.precondition.query : null}
        </TableCell>
        <TableCell>
          <SubTitle>Dropped When</SubTitle>
          {currentPlan ? currentPlan.dropcondition.query : null}
        </TableCell>
        <TableCell>
          <SubTitle>Note</SubTitle>
          {currentPlan ? currentPlan.note : null}
        </TableCell>
        {actionNodesContextArr.map((task) => (
          <TableCell key={task.taskId}>
            <h4>{task.messageName}</h4>
            {task.fields.map((field, index) => (
              <div key={index as number}>{field.name}</div>
            ))}
          </TableCell>
        ))}
      </Table>
    </Root>
  );
}

export default React.memo(PlanContext);
