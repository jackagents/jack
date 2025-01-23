import { Fluid } from 'components/common/base/BaseContainer';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailConceptListView from '../../children/DetailConceptListView';
import DetailUseMessagesAs from '../../children/DetailUseMessagesAs';
import DetailParamsView from '../../children/DetailParamsView';
/* ---------------------------- GoalDetailContent --------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function GoalDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="query messages"
        tooltipText="Messages that will be unpacked during code generation"
        content={
          <DetailConceptListView
            moduleConcept={moduleConcept}
            addingItemField="query_messages"
            addingItemType="messages"
          />
        }
      />
      <DetailField
        fieldName="use messages as"
        content={
          <DetailUseMessagesAs
            moduleConcept={moduleConcept}
            listItems={[
              'Precondition',
              'Drop Condition',
              'Satisfied',
              'Heuristic',
            ]}
          />
        }
      />
      <DetailField
        fieldName="message"
        content={
          <DetailParamsView moduleConcept={moduleConcept} type="goal_message" />
        }
      />
    </Fluid>
  );
}
