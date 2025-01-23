import CarShiftPatternIcon from '@mui-extra/icons/CarShiftPatternIcon';
import { DrivingMode, IwdIconButtonId } from 'types/iwd/iwdTypes';
import TemplateIconButton from '../templateIconButton/TemplateIconButton';

interface Props {
  mode: DrivingMode;
  onClick: () => void;
}

export default function ManualModeButton({ mode, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.manual}
      icon={CarShiftPatternIcon}
      label="Manual"
      onClick={onClick}
      optionalStyle={
        (mode === DrivingMode.MANUAL && {
          color: (theme) => theme.iwd?.button.activated,
        }) ||
        undefined
      }
    />
  );
}
