import React, { startTransition } from 'react';
import { styled } from '@mui/material';
import { CustomNodeData, useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { bdiStatusIcon, nodeIcon } from 'misc/icons/cbdi/cbdiIcons';
import { TaskStatus } from 'misc/constant/common/cmConstants';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- Styles --------------------------------- */
interface CustomNodeContainerProps {
  opacity?: number;
  inspectable: boolean;
  type: 'root' | 'children';
}
const CustomNodeContainer = styled('div')<CustomNodeContainerProps>(({ opacity, inspectable, type }) => ({
  width: '100%',
  height: '100%',
  padding: '5%',
  position: 'relative',
  overflow: 'hidden',
  textOverflow: 'ellipsis',
  border: type === 'children' ? '1px solid black' : 'none',
  borderRadius: type === 'children' ? '5%' : 'none',
  backgroundColor: '#dedede',
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  flexDirection: 'column',
  gap: '10%',
  cursor: inspectable ? 'pointer' : 'default',
  opacity: opacity || 1,
  '&.selected': {
    backgroundColor: 'grey',
  },
}));

const DelegationTagContainer = styled('img')({
  position: 'absolute',
  top: 5,
  right: 5,
  height: '20%',
  borderRadius: '5%',
});

interface Props {
  data: CustomNodeData | undefined;
  type: 'root' | 'children';
  fontSize: number;
}

function TreeNodeUnit({ data, type, fontSize }: Props) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData, setInspectNodeData, setInspectAgentGoal } = useExplainabilityContext();

  /* -------------------------------- Callbacks ------------------------------- */
  const handleNodeClick = () => {
    if (data !== undefined) {
      if (areObjectsEqual(data, inspectNodeData)) {
        setInspectNodeData(undefined);
      } else {
        if (data.type === CBDIEditorRootConceptType.GoalConceptType) {
          setInspectAgentGoal({ agentId: data.delegatingAgentAddress?.id, goalId: data.subGoalId });
        }

        startTransition(() => {
          setInspectNodeData(data);
        });
      }
    }
  };
  /* ------------------------------ useMemo hooks ----------------------------- */
  const statusImgSrc = React.useMemo(() => {
    if (data !== undefined) {
      if (data.delegationStatus === undefined || data.delegationStatus === 'Pending') {
        return undefined;
      }
      if (data.delegationStatus === 'Failed') {
        return bdiStatusIcon[TaskStatus.FAILED];
      }
      if (data.delegationStatus === 'Success') {
        return bdiStatusIcon[TaskStatus.SUCCESS];
      }
    }
    return undefined;
  }, [data]);

  const inspectable = React.useMemo(() => {
    if (
      data &&
      (data.type === CBDIEditorRootConceptType.AgentConceptType ||
        data.type === CBDIEditorRootConceptType.TeamConceptType ||
        data.type === CBDIEditorRootConceptType.GoalConceptType)
    ) {
      return true;
    }
    return false;
  }, [data]);

  const inspected = React.useMemo(() => {
    const result = areObjectsEqual(data, inspectNodeData);
    return result;
  }, [data, inspectNodeData]);
  /* -------------------------------- Component ------------------------------- */
  if (data === undefined) {
    return null;
  }

  return (
    <CustomNodeContainer
      type={type}
      inspectable={inspectable}
      className={inspected ? 'selected' : undefined}
      opacity={data.opacity}
      title={`Inspect ${data.label}`}
      onClick={() => {
        if (inspectable) {
          handleNodeClick();
        }
      }}
    >
      <div
        style={{
          width: '100%',
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          gap: '10%',
          overflowY: 'auto',
        }}
      >
        <img
          src={nodeIcon[data.type]}
          alt={data.label}
          style={{
            display: 'block',
            width: fontSize * 2,
            height: fontSize * 2,
          }}
        />
        <div
          style={{
            width: '100%',
            wordWrap: 'break-word',
            textAlign: 'center',
            fontSize,
          }}
        >
          {data.label}
        </div>
      </div>
      {statusImgSrc ? <DelegationTagContainer src={statusImgSrc} /> : null}
    </CustomNodeContainer>
  );
}

export default TreeNodeUnit;
