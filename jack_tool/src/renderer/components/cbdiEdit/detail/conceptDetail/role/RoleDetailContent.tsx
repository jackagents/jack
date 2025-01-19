import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { Fluid } from 'components/common/base/BaseContainer';
import DetailField from '../../children/DetailField';
import DetailConceptListView from '../../children/DetailConceptListView';

/* ************************************************************************************************
 * RoleDetailContent
 * *********************************************************************************************** */

interface Props {
  moduleConcept: ModuleConcept;
}

export default function RoleDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="goals"
        content={
          <DetailConceptListView
            moduleConcept={moduleConcept}
            addingItemField="goals"
            addingItemType="goals"
          />
        }
      />
      <DetailField
        fieldName="messages"
        content={
          <DetailConceptListView
            moduleConcept={moduleConcept}
            addingItemField="messages"
            addingItemType="messages"
          />
        }
      />
    </Fluid>
  );
}
