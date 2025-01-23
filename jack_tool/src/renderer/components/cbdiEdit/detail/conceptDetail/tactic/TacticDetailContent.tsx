import { Fluid } from 'components/common/base/BaseContainer';
import { CBDIEditorRootConceptType, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailConceptSingleView from '../../children/DetailConceptSingleView';
import TacticPlanPolicy from './TacticPlanPolicy';

/* --------------------------- TacticDetailContent -------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function TacticDetailContent({ moduleConcept }: Props) {
  return (
    <Fluid>
      <DetailField
        fieldName="goal"
        content={
          <DetailConceptSingleView moduleConcept={moduleConcept} field="goal" optionType={CBDIEditorRootConceptType.GoalConceptType} canBeEmpty />
        }
      />
      <DetailField fieldName="policy" content={<TacticPlanPolicy tacticModuleConcept={moduleConcept} />} />
    </Fluid>
  );
}
