import {
  Button,
  ListItem,
  SxProps,
  TextField,
  Theme,
  styled,
} from '@mui/material';
import React from 'react';
import { Row } from 'components/common/base/BaseContainer';

interface Props {
  defaultValue: string;
  request: string;
  sx?: SxProps<Theme>;
  buttonStyle?: SxProps<Theme>;
  label: string;
}

const StyledTextfield = styled(TextField)(({ theme }) => ({
  '& .MuiOutlinedInput-input': {
    color: theme.custom?.setting.textColor,
  },
}));

function SettingItem(props: Props) {
  const [mapAPI, setMapAPI] = React.useState('');

  React.useEffect(() => {
    setMapAPI(props.defaultValue);
  }, [props.defaultValue]);

  return (
    <ListItem sx={props.sx}>
      <Row>
        <StyledTextfield
          id="outlined-basic"
          label={props.label}
          variant="outlined"
          type="string"
          value={mapAPI}
          onChange={(event) => {
            setMapAPI(event.target.value);
          }}
        />
        <Button
          onClick={() => {
            window.ipcRenderer.invoke(props.request, mapAPI);
          }}
          variant="contained"
          sx={{ ...props.buttonStyle }}
        >
          Save
        </Button>
      </Row>
    </ListItem>
  );
}

SettingItem.defaultProps = {
  sx: null,
  buttonStyle: { top: 10, left: 20 },
};

export default React.memo(SettingItem);
