import { ModuleConcept } from 'types/cbdiEdit/cbdiEditTypes';
import { Fluid } from 'components/common/base/BaseContainer';
import DetailField from '../../children/DetailField';
import DetailParamsView from '../../children/DetailParamsView';

/* -------------------------- MessageDetailContent -------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function MessageDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="Message Detail"
        content={
          <DetailParamsView
            moduleConcept={moduleConcept}
            type="message_message"
          />
        }
      />
    </Fluid>
  );
}
