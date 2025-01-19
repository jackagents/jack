import { Fluid } from 'components/common/base/BaseContainer';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import EnumDetailValue from './EnumDetailValue';

/* ---------------------------- EnumDetailContent --------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function EnumDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="value"
        content={<EnumDetailValue moduleConcept={moduleConcept} />}
      />
    </Fluid>
  );
}
