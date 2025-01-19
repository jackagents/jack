/* eslint-disable no-nested-ternary */

import React, { useMemo } from 'react';
import { styled } from '@mui/material';
import { Flex, Fluid } from 'components/common/base/BaseContainer';
import Placeholder from 'components/common/placeholder/Placeholder';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { CBDIEditorProject, CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { ModuleConcept, Mod, CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { isModuleConceptOverview } from 'misc/utils/common/commonUtils';
import DetailHeader from './children/DetailHeader';
import PlanDetailContent from './conceptDetail/plan/PlanDetailContent';
import TaskHeader from './conceptDetail/planTask/TaskHeader';
import AgentDetailContent from './conceptDetail/agent/AgentDetailContent';
import RoleDetailContent from './conceptDetail/role/RoleDetailContent';
import GoalDetailContent from './conceptDetail/goal/GoalDetailContent';
import ResourceDetailContent from './conceptDetail/resource/ResourceDetailContent';
import MessageDetailContent from './conceptDetail/message/MessageDetailContent';
import TacticDetailContent from './conceptDetail/tactic/TacticDetailContent';
import ActionDetailContent from './conceptDetail/action/ActionDetailContent';
import ServiceDetailContent from './conceptDetail/service/ServiceDetailContent';
import EnumDetailContent from './conceptDetail/enum/EnumDetailContent';
import TaskDetailContent from './conceptDetail/planTask/TaskDetailContent';
import EntityDetailContent from './conceptDetail/entity/EntityDetailContent';
import EventDetailContent from './conceptDetail/event/EventDetailContent';
import EmptyDetail from './EmptyDetail';
import OverviewDetail from './OverviewDetail';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)(({ theme }) => ({
  backgroundColor: theme.editor.detailView.bgColor,
  color: theme.editor.detailView.textColor,
  padding: 10,
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontWeight: 'lighter',
  overflow: 'overlay',
  '&::-webkit-scrollbar': {
    width: 10,
    height: 5,
  },
  '&::-webkit-scrollbar-track': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb': {
    background: theme.editor.scrollBar.thumbBgColor,
    borderRadius: '2px',
  },
  '&::-webkit-scrollbar-corner': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb:hover': {
    background: theme.editor.scrollBar.thumbHoverBgColor,
  },
}));

const DetailSlot = styled('div')({
  paddingRight: 5,
  overflow: 'overlay',
  '&::-webkit-scrollbar': {
    width: 5,
    height: 5,
  },
  '&::-webkit-scrollbar-track': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb': {
    backgroundColor: 'gray',
    borderRadius: '2px',
  },
  '&::-webkit-scrollbar-corner': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb:hover': {
    background: '#ffffff50',
  },
});

const TaskDetailSlot = styled(DetailSlot)({
  marginBottom: 50,
});

const BlankDetail = styled(Flex)({
  padding: '0 10px',
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontWeight: 'lighter',
  justifyContent: 'center',
  alignItems: 'center',
});

/* --------------------------------- Detail --------------------------------- */

function Detail() {
  /* ---------------------------------- Redux --------------------------------- */
  const current: CBDIEditorProject | null = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const selectedGraphNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.graph.selectedNodeConcept);

  const dispatch = useDispatch();
  const { switchTeamAgent } = cbdiEditActions;

  /* -------------------------------- Functions ------------------------------- */

  const renderContent = (moduleConcept: ModuleConcept) => {
    const object = getObjectByModuleConcept(current, moduleConcept);
    if (!object) {
      return <EmptyDetail />;
    }
    if (object._mod === Mod.Deletion) {
      return (
        <BlankDetail>
          <span>Object Missing</span>
        </BlankDetail>
      );
    }

    switch (object._objectType) {
      case CBDIEditorRootConceptType.TeamConceptType: {
        return <AgentDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.AgentConceptType: {
        return <AgentDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.TacticConceptType: {
        return <TacticDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.PlanConceptType: {
        return <PlanDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.RoleConceptType: {
        return <RoleDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.GoalConceptType: {
        return <GoalDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.ServiceConceptType: {
        return <ServiceDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.ActionConceptType: {
        return <ActionDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.MessageConceptType: {
        return <MessageDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.ResourceConceptType: {
        return <ResourceDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.EnumConceptType: {
        return <EnumDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.EntityConceptType: {
        return <EntityDetailContent moduleConcept={moduleConcept} />;
      }
      case CBDIEditorRootConceptType.EventConceptType: {
        return <EventDetailContent moduleConcept={moduleConcept} />;
      }
      default: {
        return <Placeholder label="Detail Content" />;
      }
    }
  };

  /* -------------------------------- useMemo hooks -------------------------------- */

  const selectedGraphNodeObj = useMemo(() => getObjectByModuleConcept(current, selectedGraphNodeConcept), [current, selectedGraphNodeConcept]);

  const selectedTreeNodeObj = useMemo(() => getObjectByModuleConcept(current, selectedTreeNodeConcept), [current, selectedTreeNodeConcept]);

  const [isPlanTaskSelected, seletedTask] = useMemo(() => {
    if (selectedGraphNodeConcept && selectedTreeNodeObj && selectedTreeNodeObj._objectType === CBDIEditorRootConceptType.PlanConceptType) {
      const planObj = selectedTreeNodeObj as CBDIEditorProjectPlan;
      const selectedGraphTask = planObj.tasks.find((mtask) => mtask.nodeId === selectedGraphNodeConcept!.uuid);
      if (selectedGraphTask) {
        return [true, selectedGraphTask];
      }
    }
    return [false, undefined];
  }, [selectedGraphNodeConcept, selectedTreeNodeObj]);

  /* ------------------------------- Components ------------------------------- */
  // if no project loaded or no selected tree node
  if (!current || !selectedTreeNodeConcept) {
    return (
      <BlankDetail>
        <span>No Concept Selected</span>
      </BlankDetail>
    );
  }

  // when selected tree node is the task in plan editor's graph, it should return TaskHeader
  if (isPlanTaskSelected) {
    return (
      <Root>
        <TaskDetailSlot>
          <TaskHeader task={seletedTask!} />
          <TaskDetailContent task={seletedTask!} />
        </TaskDetailSlot>
        <DetailHeader moduleConcept={selectedTreeNodeConcept!} />
        <DetailSlot>
          <PlanDetailContent moduleConcept={selectedTreeNodeConcept!} />
        </DetailSlot>
      </Root>
    );
  }
  return (
    <Root>
      {selectedGraphNodeConcept && selectedGraphNodeObj ? (
        <div>
          <DetailHeader moduleConcept={selectedGraphNodeObj} />
          <DetailSlot>{renderContent(selectedGraphNodeConcept)}</DetailSlot>
        </div>
      ) : isModuleConceptOverview(selectedGraphNodeConcept) ? (
        <OverviewDetail />
      ) : (
        <EmptyDetail />
      )}
    </Root>
  );
}

export default React.memo(Detail);
