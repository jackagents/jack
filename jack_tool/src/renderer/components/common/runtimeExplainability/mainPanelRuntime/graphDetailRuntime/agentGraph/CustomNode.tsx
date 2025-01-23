import { styled } from '@mui/material';
import { bdiStatusIcon, nodeIcon } from 'misc/icons/cbdi/cbdiIcons';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Handle, Position } from 'reactflow';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import { TaskStatus } from 'misc/constant/common/cmConstants';
import { startTransition } from 'react';
import { CustomNodeData, useExplainabilityContext } from '../../../context/explainabilityContext';

export const EXPLAINABILITY_NODE_WIDTH = 200;
export const EXPLAINABILITY_NODE_HEIGHT = 300;

interface CustomNodeProps {
  data: CustomNodeData;
}

/* --------------------------------- Styles --------------------------------- */
interface CustomNodeContainerProps {
  type: string;
  opacity?: number;
}

const CustomNodeContainer = styled('div')<CustomNodeContainerProps>(({ type, opacity }) => ({
  position: 'relative',
  width: EXPLAINABILITY_NODE_WIDTH,
  padding: EXPLAINABILITY_NODE_WIDTH / 20,
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  flexDirection: 'column',
  opacity: opacity || 1,
  cursor:
    type === CBDIEditorRootConceptType.AgentConceptType ||
    type === CBDIEditorRootConceptType.TeamConceptType ||
    type === CBDIEditorRootConceptType.GoalConceptType
      ? 'pointer'
      : 'not-allowed',
  borderRadius: EXPLAINABILITY_NODE_WIDTH / 20,
  '&:hover': {
    backgroundColor:
      type === CBDIEditorRootConceptType.AgentConceptType ||
      type === CBDIEditorRootConceptType.TeamConceptType ||
      type === CBDIEditorRootConceptType.GoalConceptType
        ? 'grey'
        : 'undefined',
  },
  '&.selected': {
    backgroundColor:
      type === CBDIEditorRootConceptType.AgentConceptType ||
      type === CBDIEditorRootConceptType.TeamConceptType ||
      type === CBDIEditorRootConceptType.GoalConceptType
        ? 'grey'
        : 'undefined',
  },
}));

const DelegationTagContainer = styled('img')({
  position: 'absolute',
  top: 0,
  right: 0,
  width: EXPLAINABILITY_NODE_WIDTH / 8,
  height: EXPLAINABILITY_NODE_WIDTH / 8,
  borderRadius: EXPLAINABILITY_NODE_WIDTH / 20,
});

export function CustomNode({ data }: CustomNodeProps) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { inspectNodeData, setInspectNodeData, setInspectAgentGoal } = useExplainabilityContext();
  /* -------------------------------- Callbacks ------------------------------- */
  const handleNodeClick = () => {
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
  };

  const statusImgSrc = (() => {
    if (data.delegationStatus === undefined || data.delegationStatus === 'Pending') {
      return undefined;
    }
    if (data.delegationStatus === 'Failed') {
      return bdiStatusIcon[TaskStatus.FAILED];
    }
    if (data.delegationStatus === 'Success') {
      return bdiStatusIcon[TaskStatus.SUCCESS];
    }
    return undefined;
  })();

  return (
    <div>
      <Handle type="target" position={Position.Top} isConnectable={false} />
      <CustomNodeContainer
        className={areObjectsEqual(data, inspectNodeData) ? 'selected' : undefined}
        type={data.type}
        opacity={data.opacity}
        title={`Inspect ${data.label}`}
        onClick={handleNodeClick}
      >
        <img src={nodeIcon[data.type]} alt={data.label} />
        <div
          style={{
            width: '100%',
            wordWrap: 'break-word',
            textAlign: 'center',
            fontSize: 24,
            height: 80,
          }}
        >
          {data.label}
        </div>
        {statusImgSrc ? <DelegationTagContainer src={statusImgSrc} /> : null}
        <Handle type="source" position={Position.Bottom} id="bottom" isConnectable={false} />
      </CustomNodeContainer>
    </div>
  );
}

export function HiddenNode({ data }: CustomNodeProps) {
  // This component won't render anything, effectively hiding the node.
  return null;
}
