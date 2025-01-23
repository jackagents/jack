import FlashAutoIcon from '@mui-extra/icons/FlashAutoIcon';
import { DrivingMode, IwdIconButtonId } from 'types/iwd/iwdTypes';
import TemplateIconButton from '../templateIconButton/TemplateIconButton';

interface Props {
  mode: DrivingMode;
  onClick: () => void;
}

export default function AutoModeButton({ mode, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.auto}
      icon={FlashAutoIcon}
      label="Auto"
      onClick={onClick}
      optionalStyle={
        (mode === DrivingMode.AUTO && {
          color: (theme) => theme.iwd?.button.activated,
        }) ||
        undefined
      }
    />
  );
}
