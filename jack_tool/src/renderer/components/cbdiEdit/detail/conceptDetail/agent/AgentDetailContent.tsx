import { Fluid } from 'components/common/base/BaseContainer';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailConceptListView from '../../children/DetailConceptListView';
import AgentDetailGoalListView from './AgentDetailGoalListView';

/* --------------------------- AgentDetailContent --------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function AgentDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="roles"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="roles" addingItemType="roles" />}
      />
      <DetailField
        fieldName="plans"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="plans" addingItemType="plans" />}
      />
      <DetailField fieldName="goals" content={<AgentDetailGoalListView moduleConcept={moduleConcept} />} />
      <DetailField
        fieldName="beliefs"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="beliefs" addingItemType="messages" />}
      />
      <DetailField
        fieldName="actions"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="action_handlers" addingItemType="actions" />}
      />
      <DetailField
        fieldName="services"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="services" addingItemType="services" />}
      />
      <DetailField
        fieldName="resources"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="resources" addingItemType="resources" />}
      />
    </Fluid>
  );
}
