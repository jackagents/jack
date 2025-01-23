import { styled } from '@mui/material';
import { FlexCol } from 'components/common/base/BaseContainer';

export const ParamRoot = styled('li')({
  border: 'thin solid #ffffff96',
  marginBottom: 5,
  padding: '2px 5px',
  fontSize: '.9em',
});

export const Row = styled('li')({});

export const FieldRow = styled('div')({
  display: 'flex',
  width: '100%',
  marginBottom: 5,
  gap: 20,
  alignItems: 'center',
  justifyContent: 'space-between',
  '& :nth-of-type(2)': {
    flex: '0 0 80%',
  },
});

export const CenterFlexCol = styled(FlexCol)({
  justifyContent: 'center',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

export const FieldTextView = styled(TextView)({
  width: 45,
  minWidth: 45,
  lineHeight: '27px',
  marginRight: 5,
});

export const IconSlot = styled(FlexCol)({
  minWidth: 22,
  width: '22px!important',
  justifyContent: 'center',
  alignItems: 'center',
});
