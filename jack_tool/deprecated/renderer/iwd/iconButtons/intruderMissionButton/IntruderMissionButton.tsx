import { IncognitoIcon } from '@mui-extra/icons/IncognitoIcon';
import TemplateIconButton from 'components/iwd/iconButtons/templateIconButton/TemplateIconButton';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';

interface Props {
  activated: boolean;
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
}

export default function IntruderMissionButton({ activated, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.intruder}
      icon={IncognitoIcon}
      label="Intruder mission"
      onClick={onClick}
      activated={activated}
    />
  );
}
