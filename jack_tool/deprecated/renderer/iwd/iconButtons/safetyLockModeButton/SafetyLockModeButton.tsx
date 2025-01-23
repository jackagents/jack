import { DrivingMode, IwdIconButtonId } from 'types/iwd/iwdTypes';
import TemplateIconButton from 'components/iwd/iconButtons/templateIconButton/TemplateIconButton';
import LockOutlinedIcon from '@mui/icons-material/LockOutlined';

interface Props {
  mode: DrivingMode;
  onClick: () => void;
}

export default function SafetyLockModeButton({ mode, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.safety}
      icon={LockOutlinedIcon}
      label="Safety lock"
      onClick={onClick}
      optionalStyle={
        (mode === DrivingMode.SAFETY_LOCK && {
          color: (theme) => theme.iwd?.button.activated,
        }) ||
        undefined
      }
    />
  );
}
