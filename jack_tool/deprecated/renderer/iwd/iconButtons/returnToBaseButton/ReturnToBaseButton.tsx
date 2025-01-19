import RestartAltOutlinedIcon from '@mui/icons-material/RestartAltOutlined';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';
import TemplateIconButton from '../templateIconButton/TemplateIconButton';

interface Props {
  returnToBase: boolean;
  onClick: () => void;
}

export default function ReturnToBaseButton({ returnToBase, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.base}
      icon={RestartAltOutlinedIcon}
      label="Return to base"
      onClick={onClick}
      optionalStyle={
        (returnToBase && {
          color: (theme) => theme.iwd?.button.activated,
        }) ||
        undefined
      }
    />
  );
}
