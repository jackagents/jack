import { FlagCheckeredIcon } from '@mui-extra/icons/FlagCheckeredIcon';
import TemplateIconButton from 'components/iwd/iconButtons/templateIconButton/TemplateIconButton';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';

interface Props {
  activated: boolean;
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
}

export default function PathMissionButton({ activated, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.path}
      icon={FlagCheckeredIcon}
      label="Path mission"
      onClick={onClick}
      activated={activated}
    />
  );
}
