import { styled } from '@mui/material';
import { useCallback, useEffect, useMemo, useState } from 'react';
import { GoalInfoItem } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { BDILogLevel } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import { IntentionSeverityColorArr } from 'misc/icons/cbdi/cbdiIcons';
import { AgentGoalGroupDic } from '../../helper';
import SummaryCell from './SummaryCell';

const TitleContainer = styled('div')({
  padding: 10,
  fontWeight: 'bold',
  fontSize: 20,
});

const IntentionsContainer = styled('div')({
  display: 'flex',
  flexWrap: 'wrap',
  alignContent: 'flex-start',
  gap: 10,
  padding: 10,
});

const intentionLevelOptions: BDILogLevel[] = [BDILogLevel.NORMAL, BDILogLevel.IMPORTANT, BDILogLevel.CRITICAL];

interface Props {
  selectedGoalTemplateName: string | undefined;
  selectedPlanTemplateName: string | undefined;
  agentGoalGroupDic: AgentGoalGroupDic;
}

function SummaryCellsView({ selectedGoalTemplateName, selectedPlanTemplateName, agentGoalGroupDic }: Props) {
  /* ---------------------------- useState hooks ---------------------------- */
  const [intentionLevel, setIntentionLevel] = useState<BDILogLevel[]>([BDILogLevel.NORMAL, BDILogLevel.IMPORTANT, BDILogLevel.CRITICAL]);
  /* ---------------------------- useMemo hooks ---------------------------- */
  const goalIntentionCellDic = useMemo(() => {
    const mgoalIntentionCellDic: {
      active: GoalInfoItem[];
      complete: GoalInfoItem[];
    } = { active: [], complete: [] };
    if (Object.keys(agentGoalGroupDic).length === 0) {
      return mgoalIntentionCellDic;
    }
    if (!selectedGoalTemplateName || !agentGoalGroupDic[selectedGoalTemplateName]) {
      return mgoalIntentionCellDic;
    }
    if (!selectedPlanTemplateName) {
      agentGoalGroupDic[selectedGoalTemplateName].active.forEach((activeGoalInfoItem) => {
        if (intentionLevel.includes(activeGoalInfoItem.bdiLogLevel)) {
          mgoalIntentionCellDic.active.push(activeGoalInfoItem);
        }
      });

      agentGoalGroupDic[selectedGoalTemplateName].complete.forEach((completeGoalInfoItem) => {
        if (intentionLevel.includes(completeGoalInfoItem.bdiLogLevel)) {
          mgoalIntentionCellDic.complete.push(completeGoalInfoItem);
        }
      });
      return mgoalIntentionCellDic;
    }

    agentGoalGroupDic[selectedGoalTemplateName].active.forEach((activeGoalInfoItem) => {
      if (activeGoalInfoItem.planTemplateName === selectedPlanTemplateName && intentionLevel.includes(activeGoalInfoItem.bdiLogLevel)) {
        mgoalIntentionCellDic.active.push(activeGoalInfoItem);
      }
    });

    agentGoalGroupDic[selectedGoalTemplateName].complete.forEach((completeGoalInfoItem) => {
      if (completeGoalInfoItem.planTemplateName === selectedPlanTemplateName && intentionLevel.includes(completeGoalInfoItem.bdiLogLevel)) {
        mgoalIntentionCellDic.complete.push(completeGoalInfoItem);
      }
    });

    return mgoalIntentionCellDic;
  }, [selectedGoalTemplateName, selectedPlanTemplateName, agentGoalGroupDic, intentionLevel]);
  /* ---------------------------- useCallback hooks ---------------------------- */
  const onIntentionLevelOptionClick = useCallback((option: BDILogLevel) => {
    setIntentionLevel((prev) => {
      if (prev.includes(option)) {
        return prev.filter((el) => el !== option);
      }
      const result = [...prev, option];

      return result;
    });
  }, []);
  /* ---------------------------- useEffect hooks ---------------------------- */
  useEffect(() => {
    setIntentionLevel([BDILogLevel.NORMAL, BDILogLevel.IMPORTANT, BDILogLevel.CRITICAL]);
  }, [selectedGoalTemplateName]);

  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
      <div style={{ flex: '0 0 auto', display: 'flex', gap: 5, padding: 5, overflowX: 'auto', width: '100%', borderBottom: '1px solid black' }}>
        {intentionLevelOptions.map((option) => (
          <button
            type="button"
            onClick={() => onIntentionLevelOptionClick(option)}
            key={option}
            style={{
              backgroundColor: intentionLevel.includes(option) ? IntentionSeverityColorArr[option] : undefined,
              fontSize: 12,
              padding: 5,
              whiteSpace: 'nowrap',
              border: '1px solid black',
            }}
          >
            {option}
          </button>
        ))}
      </div>
      <div style={{ flex: 1, overflow: 'auto' }}>
        <TitleContainer>In Progress</TitleContainer>
        <IntentionsContainer>
          {goalIntentionCellDic.active.map((el, index) => (
            <SummaryCell key={index as number} goalInfoItem={el} type="current" />
          ))}
        </IntentionsContainer>
        <TitleContainer>Complete</TitleContainer>
        <IntentionsContainer>
          {goalIntentionCellDic.complete.map((el, index) => (
            <SummaryCell key={index as number} goalInfoItem={el} type="recent" />
          ))}
        </IntentionsContainer>
      </div>
    </div>
  );
}

export default SummaryCellsView;
