import { Fluid } from 'components/common/base/BaseContainer';
import { ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailField from '../../children/DetailField';
import DetailParamsView from '../../children/DetailParamsView';

/* --------------------------- ActionDetailContent -------------------------- */
interface Props {
  moduleConcept: ModuleConcept;
}

export default function ActionDetailContent({ moduleConcept }: Props) {
  /* ------------------------------- Components ------------------------------- */
  return (
    <Fluid>
      <DetailField fieldName="Request Message" content={<DetailParamsView moduleConcept={moduleConcept} type="action_req" />} />
      <DetailField fieldName="Reply Message" content={<DetailParamsView moduleConcept={moduleConcept} type="action_rpy" />} />
      <DetailField fieldName="Feedback Message" content={<DetailParamsView moduleConcept={moduleConcept} type="action_feedback" />} />
    </Fluid>
  );
}
