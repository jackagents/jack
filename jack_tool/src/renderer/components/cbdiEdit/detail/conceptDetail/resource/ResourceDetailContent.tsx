import { ModuleConcept } from 'types/cbdiEdit/cbdiEditTypes';
import { Fluid } from 'components/common/base/BaseContainer';
import DetailField from '../../children/DetailField';
import ResourceDetailValue from './ResourceDetailValue';

/* -------------------------- ResourceDetailContent ------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function ResourceDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="value"
        content={<ResourceDetailValue moduleConcept={moduleConcept} />}
      />
    </Fluid>
  );
}
