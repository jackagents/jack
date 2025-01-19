/* eslint-disable jsx-a11y/no-static-element-interactions */
/* eslint-disable jsx-a11y/click-events-have-key-events */
import { styled } from '@mui/material';
import { GoalInfoItem } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { useMemo } from 'react';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { nodeColor } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { PlanGroupDic } from './type';
import AgentGoalGroupTable, { AgentGoalGroupTableWidth, AgentGoalGroupTableHeight } from './children/AgentGoalGroupTable';

/* --------------------------------- Styles --------------------------------- */
const Container = styled('div')({
  cursor: 'pointer',
  width: AgentGoalGroupTableWidth,
  height: AgentGoalGroupTableHeight,
  position: 'relative',
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  borderRadius: 5,
  overflow: 'hidden',
});

const Row = styled('div')({
  width: '100%',
  display: 'flex',
  alignItems: 'center',
  minHeight: 40,
  gap: 5,
  paddingLeft: 10,
  paddingRight: 10,
});

interface Props {
  goalTemplateName: string;
  goalInfoItemDic: {
    active: GoalInfoItem[];
    complete: GoalInfoItem[];
  };
  selected: boolean;
  setSelectedGoalTemplateName: React.Dispatch<React.SetStateAction<string | undefined>>;
  selectedPlanTemplateName: string | undefined;
  setSelectedPlanTemplateName: React.Dispatch<React.SetStateAction<string | undefined>>;
}

function AgentGoalGroupView({
  goalTemplateName,
  goalInfoItemDic,
  selected,
  setSelectedGoalTemplateName,
  selectedPlanTemplateName,
  setSelectedPlanTemplateName,
}: Props) {
  /* --------------------------------- useMemo hooks --------------------------------- */
  const planGroupDic = useMemo(() => {
    const mplanGroupDic: PlanGroupDic = {};

    goalInfoItemDic.active.forEach((goalInfoItem) => {
      if (goalInfoItem.planTemplateName) {
        if (!mplanGroupDic[goalInfoItem.planTemplateName]) {
          mplanGroupDic[goalInfoItem.planTemplateName] = {
            active: [goalInfoItem],
            complete: [],
          };
        } else {
          mplanGroupDic[goalInfoItem.planTemplateName].active.push(goalInfoItem);
        }
      } else if (!mplanGroupDic['No Plan']) {
        mplanGroupDic['No Plan'] = {
          active: [goalInfoItem],
          complete: [],
        };
      } else {
        mplanGroupDic['No Plan'].active.push(goalInfoItem);
      }
    });

    goalInfoItemDic.complete.forEach((goalInfoItem) => {
      if (goalInfoItem.planTemplateName) {
        if (!mplanGroupDic[goalInfoItem.planTemplateName]) {
          mplanGroupDic[goalInfoItem.planTemplateName] = {
            active: [],
            complete: [goalInfoItem],
          };
        } else {
          mplanGroupDic[goalInfoItem.planTemplateName].complete.push(goalInfoItem);
        }
      } else if (!mplanGroupDic['No Plan']) {
        mplanGroupDic['No Plan'] = {
          active: [],
          complete: [goalInfoItem],
        };
      } else {
        mplanGroupDic['No Plan'].complete.push(goalInfoItem);
      }
    });
    return mplanGroupDic;
  }, [goalInfoItemDic]);

  /* -------------------------------- Callbacks ------------------------------- */

  const handleClickRow = (row: Record<string, undefined>) => {
    if (selectedPlanTemplateName !== row.planName) {
      setSelectedGoalTemplateName(goalTemplateName);
      setSelectedPlanTemplateName(row.planName);
    } else {
      setSelectedGoalTemplateName(goalTemplateName);
      setSelectedPlanTemplateName(undefined);
    }
  };

  return (
    <Container
      onClick={(e) => {
        e.stopPropagation();
        if ((e.target as HTMLElement).getAttribute('role') !== 'gridcell') {
          setSelectedPlanTemplateName(undefined);
          setSelectedGoalTemplateName(goalTemplateName);
        }
      }}
      style={{ outline: selected ? '2px solid green' : '1px solid black' }}
    >
      <Row style={{ backgroundColor: nodeColor[CBDIEditorRootConceptType.GoalConceptType], justifyContent: 'center' }}>
        {removeBeforeFirstDotAndDot(goalTemplateName)}
        {` (${goalInfoItemDic.active.length + goalInfoItemDic.complete.length})`}
      </Row>
      <div style={{ flex: 1, width: '100%', overflowY: 'auto' }}>
        <AgentGoalGroupTable planGroupDic={planGroupDic} handleClickRow={handleClickRow} selectedPlanTemplateName={selectedPlanTemplateName} />
      </div>
    </Container>
  );
}

export default AgentGoalGroupView;
