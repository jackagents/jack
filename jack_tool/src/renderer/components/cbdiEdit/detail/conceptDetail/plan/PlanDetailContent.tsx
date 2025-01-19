import { Fluid } from 'components/common/base/BaseContainer';
import { CBDIEditorRootConceptType, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailConceptListView from '../../children/DetailConceptListView';
import DetailUseMessagesAs from '../../children/DetailUseMessagesAs';
import PlanDetailAgentList from './PlanDetailAgentList';
import DetailConceptSingleView from '../../children/DetailConceptSingleView';

/* ---------------------------- PlanDetailContent --------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function PlanDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="handles"
        content={
          <DetailConceptSingleView moduleConcept={moduleConcept} field="handles" optionType={CBDIEditorRootConceptType.GoalConceptType} canBeEmpty />
        }
      />
      <DetailField
        fieldName="query message"
        tooltipText="Messages that will be unpacked during code generation"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="query_messages" addingItemType="messages" />}
      />
      <DetailField
        fieldName="use messages as"
        content={<DetailUseMessagesAs moduleConcept={moduleConcept} listItems={['Precondition', 'Drop Condition', 'Effects']} />}
      />
      <DetailField fieldName="plan used by" content={<PlanDetailAgentList moduleConcept={moduleConcept} />} />
    </Fluid>
  );
}
