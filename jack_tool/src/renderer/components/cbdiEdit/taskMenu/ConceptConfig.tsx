import Button from '@mui/material/Button';
import Dialog from '@mui/material/Dialog';
import DialogActions from '@mui/material/DialogActions';
import DialogContent from '@mui/material/DialogContent';
import DialogTitle from '@mui/material/DialogTitle';
import { capitalize } from 'misc/utils/cbdiEdit/Helpers';
import ActionDetailContent from 'components/cbdiEdit/detail/conceptDetail/action/ActionDetailContent';
import GoalDetailContent from 'components/cbdiEdit/detail/conceptDetail/goal/GoalDetailContent';
import AgentDetailContent from 'components/cbdiEdit/detail/conceptDetail/agent/AgentDetailContent';
import TacticDetailContent from 'components/cbdiEdit/detail/conceptDetail/tactic/TacticDetailContent';
import PlanDetailContent from 'components/cbdiEdit/detail/conceptDetail/plan/PlanDetailContent';
import RoleDetailContent from 'components/cbdiEdit/detail/conceptDetail/role/RoleDetailContent';
import ServiceDetailContent from 'components/cbdiEdit/detail/conceptDetail/service/ServiceDetailContent';
import MessageDetailContent from 'components/cbdiEdit/detail/conceptDetail/message/MessageDetailContent';
import ResourceDetailContent from 'components/cbdiEdit/detail/conceptDetail/resource/ResourceDetailContent';
import {
  CBDIEditorRootConceptType,
  ModuleConcept,
  OptionData,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import EnumDetailContent from '../detail/conceptDetail/enum/EnumDetailContent';

interface Props {
  conceptType: string | undefined;
  concept: OptionData | undefined;
  openConfigDialog: boolean;
  closeConfigDialog: () => void;
}

export default function ConceptConfig(props: Props) {
  const getDetailContent = (conceptType: string, concept: ModuleConcept) => {
    switch (conceptType) {
      case CBDIEditorRootConceptType.TeamConceptType: {
        return <AgentDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.AgentConceptType: {
        return <AgentDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.TacticConceptType: {
        return <TacticDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.PlanConceptType: {
        return <PlanDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.RoleConceptType: {
        return <RoleDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.GoalConceptType: {
        return <GoalDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.ServiceConceptType: {
        return <ServiceDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.ActionConceptType: {
        return <ActionDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.MessageConceptType: {
        return <MessageDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.ResourceConceptType: {
        return <ResourceDetailContent moduleConcept={concept} />;
      }
      case CBDIEditorRootConceptType.EnumConceptType: {
        return <EnumDetailContent moduleConcept={concept} />;
      }
      default: {
        return null;
      }
    }
  };
  if (props.conceptType && props.concept && props.concept.moduleConcept) {
    return (
      <Dialog open={props.openConfigDialog}>
        <DialogTitle>
          Configure {capitalize(props.conceptType!)} {props.concept.label}
        </DialogTitle>
        <DialogContent>
          {getDetailContent(props.conceptType!, props.concept.moduleConcept)}
        </DialogContent>
        <DialogActions>
          <Button variant="contained" onClick={props.closeConfigDialog}>
            Close
          </Button>
        </DialogActions>
      </Dialog>
    );
  }
  return null;
}
