/* eslint-disable jsx-a11y/no-static-element-interactions */
/* eslint-disable jsx-a11y/click-events-have-key-events */
import React from 'react';
import planImgSvg from 'assets/cbdi/icons/plan_icon.svg';
import { GoalInfoItem } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { IntentionSeverityColorArr } from 'misc/icons/cbdi/cbdiIcons';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { nodeColor } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

const NodeWidth = 30;
const LabelFontSize = NodeWidth / 2;

interface SummaryNode {
  imgUrl: string;
  label: string | undefined;
  title: string;
}

interface Props {
  goalInfoItem: GoalInfoItem;
  type: 'current' | 'recent';
}

export default function ({ goalInfoItem, type }: Props) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { setInspectAgentGoal, inspectAgentGoal } = useExplainabilityContext();

  /* -------------------------------- Callbacks ------------------------------- */
  const handleClick = () => {
    if (inspectAgentGoal?.goalId === goalInfoItem.goalId) {
      setInspectAgentGoal(undefined);
    } else {
      setInspectAgentGoal({ agentId: goalInfoItem.agentId, goalId: goalInfoItem.goalId });
    }
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const summaryNode: SummaryNode = React.useMemo(() => {
    const planNode: SummaryNode = {
      imgUrl: planImgSvg,
      label: goalInfoItem.planTemplateName || 'No Plan',
      title: goalInfoItem.goalTemplateName,
    };

    return planNode;
  }, [goalInfoItem]);

  const goalContextMsgLabel = React.useMemo(() => {
    if (goalInfoItem.goalContextMsg && goalInfoItem.goalContextMsg.length > 0) {
      const firstMsg = goalInfoItem.goalContextMsg[0];
      return firstMsg?.tag;
    }

    return undefined;
  }, [goalInfoItem.goalContextMsg]);

  return (
    <div
      style={{
        cursor: 'pointer',
        width: NodeWidth * 6,
        display: 'flex',
        flexDirection: 'column',
        overflow: 'auto',
        borderRadius: 5,
        fontSize: 12,
        backgroundColor: IntentionSeverityColorArr[goalInfoItem.bdiLogLevel],
        minHeight: NodeWidth * 2,
        maxHeight: NodeWidth * 4,
        outline: goalInfoItem.goalId === inspectAgentGoal?.goalId ? '2px solid green' : '1px solid black',
      }}
      onClick={handleClick}
    >
      <div
        style={{
          flexBasis: NodeWidth * 2,
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'space-around',
          alignItems: 'center',
          borderBottom: '1px solid black',
        }}
      >
        <div
          style={{
            wordWrap: 'break-word',
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
            justifyContent: 'center',
          }}
          title={removeBeforeFirstDotAndDot(summaryNode.title)}
        >
          <div
            style={{
              textAlign: 'center',
              overflow: 'hidden',
              textOverflow: 'ellipsis',
              fontSize: LabelFontSize,
            }}
          >
            {removeBeforeFirstDotAndDot(summaryNode.label)}
          </div>
        </div>
        <div>{goalContextMsgLabel ? `[${goalContextMsgLabel}]` : ''}</div>
      </div>

      {!goalInfoItem.goalResult ? (
        <div
          style={{
            whiteSpace: 'nowrap',
            backgroundColor: nodeColor[CBDIEditorRootConceptType.PlanConceptType],
            overflow: 'hidden',
            textOverflow: 'ellipsis',
            textAlign: 'center',
          }}
        >
          INTENTION {goalInfoItem.intentionResult || 'RUNNING'}
        </div>
      ) : null}

      {goalInfoItem.goalResult && (
        <div
          style={{
            whiteSpace: 'nowrap',
            backgroundColor: nodeColor[CBDIEditorRootConceptType.GoalConceptType],
            overflow: 'hidden',
            textOverflow: 'ellipsis',
            textAlign: 'center',
          }}
        >
          GOAL {goalInfoItem.goalResult}
        </div>
      )}
    </div>
  );
}
