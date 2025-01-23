import { BullseyeArrowIcon } from '@mui-extra/icons/BullseyeArrowIcon';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';
import TemplateIconButton from 'components/iwd/iconButtons/templateIconButton/TemplateIconButton';

interface Props {
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
  runningMission: boolean;
}

export default function CurrentMissionButton({
  onClick,
  runningMission,
}: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.auto}
      icon={BullseyeArrowIcon}
      label="Current mission"
      onClick={onClick}
      disabled={!runningMission}
    />
  );
}
