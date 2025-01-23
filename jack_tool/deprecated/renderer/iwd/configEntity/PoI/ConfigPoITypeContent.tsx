import {
  FormControlLabel,
  Grid,
  Radio,
  RadioGroup,
  RadioProps,
  styled,
  Typography,
} from '@mui/material';
import React from 'react';
import { useDispatch } from 'react-redux';
import { PoIType, PointOfInterestType } from 'types/iwd/iwdTypes';
import { ConfigLabelTitle } from 'components/common/base/BaseContainer';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';

const RdBtnGrp = styled(RadioGroup)({
  marginLeft: '20px',
});

const RadioLabel = styled(Typography)(({ theme }) => ({
  color: theme.custom?.text.color,
  fontSize: theme.custom?.text.size.medium,
}));

const BpIcon = styled('span')(({ theme }) => ({
  borderRadius: '50%',
  width: 16,
  height: 16,
  boxShadow:
    theme.palette.mode === 'dark'
      ? '0 0 0 1px rgb(16 22 26 / 40%)'
      : 'inset 0 0 0 1px rgba(16,22,26,.2), inset 0 -1px 0 rgba(16,22,26,.1)',
  backgroundColor: theme.palette.mode === 'dark' ? '#394b59' : '#f5f8fa',
  backgroundImage:
    theme.palette.mode === 'dark'
      ? 'linear-gradient(180deg,hsla(0,0%,100%,.05),hsla(0,0%,100%,0))'
      : 'linear-gradient(180deg,hsla(0,0%,100%,.8),hsla(0,0%,100%,0))',
  '.Mui-focusVisible &': {
    outline: '2px auto rgba(19,124,189,.6)',
    outlineOffset: 2,
  },
  'input:hover ~ &': {
    backgroundColor: theme.palette.mode === 'dark' ? '#30404d' : '#ebf1f5',
  },
  'input:disabled ~ &': {
    boxShadow: 'none',
    background:
      theme.palette.mode === 'dark'
        ? 'rgba(57,75,89,.5)'
        : 'rgba(206,217,224,.5)',
  },
}));

const BpCheckedIcon = styled(BpIcon)({
  backgroundColor: '#137cbd',
  backgroundImage:
    'linear-gradient(180deg,hsla(0,0%,100%,.1),hsla(0,0%,100%,0))',
  '&:before': {
    display: 'block',
    width: 16,
    height: 16,
    backgroundImage: 'radial-gradient(#fff,#fff 28%,transparent 32%)',
    content: '""',
  },
  'input:hover ~ &': {
    backgroundColor: '#106ba3',
  },
});

interface Props {
  poi: PointOfInterestType;
}

interface RadioBtnProps {
  type: PoIType;
}

function ConfigPoITypeContent(props: Props) {
  const dispatch = useDispatch();
  const [content, setContent] = React.useState<RadioBtnProps>({
    type: props.poi.type,
  });

  React.useEffect(() => {
    setContent({
      type: props.poi.type,
    });
  }, [props.poi]);

  const BpRadio = (radioProps: RadioProps) => {
    return (
      <Radio
        sx={{
          '&:hover': {
            bgcolor: 'transparent',
          },
        }}
        disableRipple
        color="default"
        checkedIcon={<BpCheckedIcon />}
        icon={<BpIcon />}
        // eslint-disable-next-line react/jsx-props-no-spreading
        {...radioProps}
      />
    );
  };

  /**
   * Callback when change radio button
   * @param event
   */
  const handleRadioChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    // Change component state
    setContent({ ...content, type: event.target.value as PoIType });

    const poi: PointOfInterestType = {
      ...props.poi,
      type: event.target.value as PoIType,
    };

    dispatch(iwdActions.updatePointOfInterest(JSON.stringify(poi)));
  };

  return (
    <>
      <Grid>
        <ConfigLabelTitle>Type</ConfigLabelTitle>

        <RdBtnGrp
          onChange={handleRadioChange}
          value={content.type}
          name="customized-radios"
        >
          <FormControlLabel
            value="unknown"
            control={<BpRadio />}
            label={<RadioLabel>Unknown</RadioLabel>}
          />
          <FormControlLabel
            value="hostile"
            control={<BpRadio />}
            label={<RadioLabel>Hostile</RadioLabel>}
          />
          <FormControlLabel
            value="friendly"
            control={<BpRadio />}
            label={<RadioLabel>Friendly</RadioLabel>}
          />
        </RdBtnGrp>
      </Grid>
    </>
  );
}

export default React.memo(ConfigPoITypeContent);
