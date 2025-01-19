import React, { useEffect, useMemo, useState } from 'react';
import { styled } from '@mui/material';
import { AgentSummaryGoalInfo } from 'types/cbdi/cbdi-models-non-flatbuffer';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { Allotment } from 'allotment';
import AgentGoalGroupView from './children/AgentGoalGroupView/AgentGoalGroupView';
import AgentGoalPursueTable from './children/AgentGoalPursueTable/AgentGoalPursueTable';
import { AgentGoalGroupDic } from './helper';
import SummaryCellsView from './children/SummaryCellsView/SummaryCellsView';

interface Props {
  agentSummaryIntentionInfo: AgentSummaryGoalInfo | undefined;
}

const IntentionsContainer = styled('div')({
  display: 'flex',
  flexWrap: 'wrap',
  alignContent: 'flex-start',
  overflow: 'auto',
  gap: 10,
  padding: 10,
  height: '100%',
});

function AgentSummaryView({ agentSummaryIntentionInfo }: Props) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { setInspectAgentGoal } = useExplainabilityContext();
  /* ---------------------------- useState hooks ---------------------------- */
  const [selectedGoalTemplateName, setSelectedGoalTemplateName] = useState<string | undefined>();
  const [selectedPlanTemplateName, setSelectedPlanTemplateName] = useState<string | undefined>();
  const [intentionDetailMode, setIntentionDetailMode] = useState<'intentionCell' | 'pursueTable'>('intentionCell');
  /* ---------------------------- useMemo hooks ---------------------------- */
  const agentGoalGroupDic = useMemo(() => {
    const magentGoalGroupDic: AgentGoalGroupDic = {};
    if (agentSummaryIntentionInfo) {
      agentSummaryIntentionInfo.currentIntentions.forEach((goalInfoItem) => {
        if (!magentGoalGroupDic[goalInfoItem.goalTemplateName]) {
          magentGoalGroupDic[goalInfoItem.goalTemplateName] = {
            active: [],
            complete: [],
          };
        }
        magentGoalGroupDic[goalInfoItem.goalTemplateName].active.push(goalInfoItem);
      });

      agentSummaryIntentionInfo.recentImportantIntentions.forEach((goalInfoItem) => {
        if (!magentGoalGroupDic[goalInfoItem.goalTemplateName]) {
          magentGoalGroupDic[goalInfoItem.goalTemplateName] = {
            active: [],
            complete: [],
          };
        }
        magentGoalGroupDic[goalInfoItem.goalTemplateName].complete.push(goalInfoItem);
      });
    }
    return magentGoalGroupDic;
  }, [agentSummaryIntentionInfo]);
  /* ---------------------------- useEffect hooks ---------------------------- */
  useEffect(() => {
    setInspectAgentGoal(undefined);
  }, [selectedGoalTemplateName]);

  useEffect(() => {
    if (selectedGoalTemplateName && !agentGoalGroupDic[selectedGoalTemplateName]) {
      setSelectedGoalTemplateName(undefined);
      setSelectedPlanTemplateName(undefined);
    }
  }, [selectedGoalTemplateName, agentGoalGroupDic]);

  return (
    <Allotment vertical defaultSizes={[60, 40]}>
      <Allotment.Pane preferredSize="60%">
        <IntentionsContainer
          onClick={() => {
            setSelectedGoalTemplateName(undefined);
            setSelectedPlanTemplateName(undefined);
          }}
        >
          {Object.entries(agentGoalGroupDic).map(([goalTemplateName, goalInfoItemDic], index) => (
            <AgentGoalGroupView
              selected={selectedGoalTemplateName === goalTemplateName}
              setSelectedGoalTemplateName={setSelectedGoalTemplateName}
              selectedPlanTemplateName={selectedPlanTemplateName}
              setSelectedPlanTemplateName={setSelectedPlanTemplateName}
              key={index as number}
              goalTemplateName={goalTemplateName}
              goalInfoItemDic={goalInfoItemDic}
            />
          ))}
        </IntentionsContainer>
      </Allotment.Pane>
      <Allotment.Pane visible={!!selectedGoalTemplateName}>
        <div style={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
          <div style={{ flex: 0, display: 'flex', borderBottom: '1px solid black' }}>
            <button
              type="button"
              onClick={() => {
                setIntentionDetailMode('intentionCell');
              }}
              style={{ fontSize: 'inherit', borderRadius: 0, backgroundColor: intentionDetailMode === 'intentionCell' ? 'grey' : '', padding: 5 }}
            >
              Intention Cell
            </button>
            <button
              type="button"
              onClick={() => {
                setIntentionDetailMode('pursueTable');
              }}
              style={{ fontSize: 'inherit', borderRadius: 0, backgroundColor: intentionDetailMode === 'pursueTable' ? 'grey' : '', padding: 5 }}
            >
              Pursue Table
            </button>
          </div>
          <div style={{ flex: 1, overflow: 'hidden' }}>
            {intentionDetailMode === 'intentionCell' && (
              <SummaryCellsView
                selectedGoalTemplateName={selectedGoalTemplateName!}
                selectedPlanTemplateName={selectedPlanTemplateName}
                agentGoalGroupDic={agentGoalGroupDic}
              />
            )}
            {intentionDetailMode === 'pursueTable' && (
              <AgentGoalPursueTable
                key={selectedGoalTemplateName}
                selectedGoalTemplateName={selectedGoalTemplateName}
                selectedPlanTemplateName={selectedPlanTemplateName}
                agentGoalGroupDic={agentGoalGroupDic}
              />
            )}
          </div>
        </div>
      </Allotment.Pane>
    </Allotment>
  );
}

export default React.memo(AgentSummaryView);
