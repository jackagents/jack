import { ThemedButton } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { styled } from '@mui/material';
import { CheckBoxOutlineBlank as CheckBoxUncheckedIcon, CheckBox as CheckBoxCheckedIcon } from '@mui/icons-material';
import { FieldRow } from '../detail/conceptDetail/message/DraggableMessageFieldItem/styledComponents/StyledComponents';

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

interface Props {
  onToggle: () => void;
  currentValue: boolean;
  label: string;
  disabled?: boolean;
  titile?: string;
}

function BooleanValueToggler({ onToggle, currentValue, label, disabled, titile }: Props) {
  return (
    <FieldRow>
      <TextView title={titile}>{label}</TextView>
      <ThemedButton disabled={disabled} onClick={() => onToggle()}>
        {currentValue ? <CheckBoxCheckedIcon /> : <CheckBoxUncheckedIcon />}
      </ThemedButton>
    </FieldRow>
  );
}

export default BooleanValueToggler;
