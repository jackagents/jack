import { styled } from '@mui/material';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import React from 'react';
import { ModuleConcept } from 'types/cbdiEdit/cbdiEditTypes';
import { RootState } from 'projectRedux/Store';
import { useSelector } from 'react-redux';
import { CBDIEditorProjectAgent, CBDIEditorProjectTeam } from 'misc/types/cbdiEdit/cbdiEditModel';
import DetailConceptListItem from '../../children/DetailConceptListItem';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  display: 'flex',
  flexDirection: 'column',
});

interface Props {
  moduleConcept: ModuleConcept;
}

export default function PlanDetailAgentList({ moduleConcept }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  /* ------------------------------ useMemo hooks ----------------------------- */
  const agentReferConceptList = React.useMemo(() => {
    const magentReferConceptList: ModuleConcept[] = [];
    if (current) {
      const currentProject = current;
      currentProject.teams.forEach((teamModuleConcept) => {
        const team: CBDIEditorProjectTeam | undefined = getObjectByModuleConcept(currentProject, teamModuleConcept);
        if (team && team.plans.some((item) => item.uuid === teamModuleConcept.uuid)) {
          magentReferConceptList.push(teamModuleConcept);
        }
      });

      currentProject.agents.forEach((agentModuleConcept) => {
        const agent: CBDIEditorProjectAgent | undefined = getObjectByModuleConcept(currentProject, agentModuleConcept);
        if (agent && agent.plans.some((item) => item.uuid === moduleConcept.uuid)) {
          magentReferConceptList.push(agentModuleConcept);
        }
      });
    }
    return magentReferConceptList;
  }, [current, moduleConcept]);

  return (
    <Root>
      {agentReferConceptList.length > 0 ? (
        agentReferConceptList.map((item, index) => <DetailConceptListItem key={index as number} moduleConcept={item} />)
      ) : (
        <div style={{ paddingLeft: 8 }}>null</div>
      )}
    </Root>
  );
}
