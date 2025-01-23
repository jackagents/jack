import { Fluid } from 'components/common/base/BaseContainer';
import {
  ModuleConcept,
  CBDIEditorRootConceptType,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailConceptSingleView from '../../children/DetailConceptSingleView';

/* --------------------------- EventDetailContent -------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function EventDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="message"
        content={
          <DetailConceptSingleView
            moduleConcept={moduleConcept}
            field="message"
            optionType={CBDIEditorRootConceptType.MessageConceptType}
            canBeEmpty
          />
        }
      />
    </Fluid>
  );
}
