import React from 'react';
import { styled } from '@mui/material';
import ExpandableContainer from 'components/common/expandableContainer/ExpandableContainer';
import { IntentionSeverityColorArr } from 'misc/icons/cbdi/cbdiIcons';
import { BDILogLevel } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { ReasoningDic } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorProjectMessage, CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';

/* --------------------------------- Styles --------------------------------- */

const ListContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  gap: 10,
  overflow: 'hidden',
  overflowY: 'auto',
  padding: 10,
});

const ListItem = styled('div')({
  padding: 10,
  textAlign: 'start',
  position: 'relative',
  '&::before': {
    content: '""',
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: 'rgba(0, 0, 0, 0)',
    transition: 'background-color 0.3s',
    zIndex: 1,
  },
  '&:hover::before': {
    backgroundColor: 'rgba(0, 0, 0, 0.5)',
    zIndex: 2,
  },
});

export function ReasoningListView({
  reasoningDic,
  selectedPlan,
  setHoveringReasoningTaskId,
}: {
  reasoningDic: ReasoningDic;
  selectedPlan: CBDIEditorProjectPlan | undefined;
  setHoveringReasoningTaskId?: React.Dispatch<React.SetStateAction<string | undefined>>;
}) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { project } = useExplainabilityContext();

  return (
    <ExpandableContainer
      customStyles={{
        top: 450,
        left: 0,
        width: 500,
        maxHeight: 300,
        minHeight: 100,
        gap: 10,
        paddingTop: 10,
      }}
      buttonTitle="Reasoning List"
    >
      <div style={{ textAlign: 'center' }}>Reasoning List</div>
      {Object.keys(reasoningDic).map((intentionId) => {
        const reasoningItem = reasoningDic[intentionId];
        return (
          <ListContainer key={intentionId}>
            {/* <div title={intentionId}>{intentionId}</div> */}
            {Object.keys(reasoningItem).map((taskId, index) => {
              if (!project || !selectedPlan) {
                return null;
              }
              const matchedTask = selectedPlan.tasks.find((task) => task.nodeId.replace(/-/g, '') === taskId);
              const taskMessage: CBDIEditorProjectMessage | undefined = getObjectByModuleConcept(
                project,
                matchedTask?.nodeData.action || matchedTask?.nodeData.goal,
              );
              const taskTemplateName = taskMessage?.name;
              return (
                <ListItem
                  style={{
                    backgroundColor: IntentionSeverityColorArr[reasoningItem[taskId].level || BDILogLevel.NORMAL],
                  }}
                  key={index as number}
                  onMouseEnter={() => {
                    if (setHoveringReasoningTaskId) {
                      setHoveringReasoningTaskId(taskId);
                    }
                  }}
                  onMouseLeave={() => {
                    if (setHoveringReasoningTaskId) {
                      setHoveringReasoningTaskId(undefined);
                    }
                  }}
                >
                  <h4>{taskTemplateName}</h4>
                  <p>{reasoningItem[taskId].reasoningText}</p>
                </ListItem>
              );
            })}
          </ListContainer>
        );
      })}
    </ExpandableContainer>
  );
}
