import BoltOutlinedIcon from '@mui/icons-material/BoltOutlined';
import TemplateIconButton from 'components/iwd/iconButtons/templateIconButton/TemplateIconButton';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';

interface Props {
  activated: boolean;
  onClick: () => void;
}

export default function ActivatedButton({ activated, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.activate}
      icon={BoltOutlinedIcon}
      label="Activated"
      onClick={onClick}
      optionalStyle={
        activated
          ? { color: (theme) => theme.iwd?.button.activated }
          : undefined
      }
    />
  );
}
