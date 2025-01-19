import { Fluid } from 'components/common/base/BaseContainer';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailConceptListView from '../../children/DetailConceptListView';
import EntityDetailComponentsView from './EntityDetailComponentsView';

/* ---------------------------- EntityDetailContent --------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function EntityDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="children"
        content={<DetailConceptListView moduleConcept={moduleConcept} addingItemField="children" addingItemType="entities" />}
      />
      <DetailField fieldName="components" content={<EntityDetailComponentsView entityModuleConcept={moduleConcept} />} />
    </Fluid>
  );
}
