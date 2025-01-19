import { BinocularsIcon } from '@mui-extra/icons/BinocularsIcon';
import TemplateIconButton from 'components/iwd/iconButtons/templateIconButton/TemplateIconButton';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';

interface Props {
  activated: boolean;
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
}

export default function IntruderMissionButton({ activated, onClick }: Props) {
  return (
    <TemplateIconButton
      id={IwdIconButtonId.scout}
      icon={BinocularsIcon}
      label="Scout mission"
      onClick={onClick}
      activated={activated}
    />
  );
}
