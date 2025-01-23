import { ModuleConcept } from 'types/cbdiEdit/cbdiEditTypes';
import { Fluid } from 'components/common/base/BaseContainer';
import DetailField from '../../children/DetailField';
import DetailConceptListView from '../../children/DetailConceptListView';
import ServiceTopics from './ServiceTopics/ServiceTopics';

/* -------------------------- ServiceDetailContent -------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function ServiceDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="actions"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="action_handlers" addingItemType="actions" />}
      />
      <DetailField fieldName="topics" content={<ServiceTopics serviceModuleConcept={moduleConcept} />} />
    </Fluid>
  );
}
