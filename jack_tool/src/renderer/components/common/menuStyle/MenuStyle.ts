import { SxProps, Theme } from '@mui/material';

export const functionContainerStyle: SxProps<Theme> = {
  width: 380,
  zIndex: 400,
  height: '100%',
  backgroundColor: '#333333',
  flexDirection: 'column',
  justifyContent: 'flex-start',
  alignItems: 'center',
  alignContent: 'center',
};

export const functionBoxStyle: SxProps<Theme> = {
  width: 280,
  margin: 2,

  border: 1,
  borderColor: 'white',
  height: 'auto',
};

export const functionLabelStyle: SxProps<Theme> = {
  position: 'relative',
  backgroundColor: '#ededed',
  color: 'black',
  width: 'fit-content',
  padding: '0 5px 0 5px',
};

export const functionLabelContainerStyle: SxProps<Theme> = {
  justifyContent: 'center',
  height: '10px',
};

export const buttonContainerStyle: SxProps<Theme> = {
  alignContent: 'center',
  flexDirection: 'row',
};

export const functionButtonStyle: SxProps<Theme> = {
  width: '50px',
  height: '50px',
  margin: '10px 10px 10px 10px',
  justifyContent: 'flex-start',
  '&:hover': {
    backgroundColor: 'green',
  },
};
export const iconStyle: SxProps<Theme> = { color: 'white' };

export const detailTextTitleStyle: SxProps<Theme> = {
  color: 'yellow',
  margin: 'auto',
};

export const detailTextValueStyle: SxProps<Theme> = {
  color: 'white',
  margin: 'auto',
  whiteSpace: 'nowrap',
  overflow: 'hidden',
  textOverflow: 'ellipsis',
};
export const buttonLabelContainerStyle: SxProps<Theme> = {
  alignContent: 'center',
  display: 'flex',
  alignItems: 'center',
};

export const buttonLabelStyle: SxProps<Theme> = {
  width: 'auto',
  alignItems: 'center',
  color: 'white',
};

export const containerStyle: SxProps<Theme> = { margin: '10px' };
export const labelStyle: SxProps<Theme> = { marginLeft: '10px' };
export const clickedItem: SxProps<Theme> = {
  backgroundColor: 'red',
  '&:hover': { backgroundColor: 'red' },
};

export const notClickedItem: SxProps<Theme> = {
  '&:hover': { backgroundColor: 'green' },
};

export const configLabel: SxProps<Theme> = {
  fontSize: 18,
  margin: '10px',
  color: 'white',
};
export const configStaticLabelContainer: SxProps<Theme> = {
  margin: '0 10px 10px',
};

export const textFieldStyle: SxProps<Theme> = {
  m: 1,
  width: '29ch',
  '& p': {
    color: 'white',
  },
  // Normal
  '& .MuiOutlinedInput-input': {
    color: 'white',
  },
  '& .MuiInputLabel-root': {
    color: 'white',
  },
  '& .MuiOutlinedInput-root .MuiOutlinedInput-notchedOutline': {
    borderColor: 'white',
  },
  // Hover
  '&:hover .MuiOutlinedInput-input': {
    color: 'white',
  },
  '&:hover .MuiInputLabel-root': {
    color: 'white',
  },
  '&:hover .MuiOutlinedInput-root .MuiOutlinedInput-notchedOutline': {
    borderColor: 'green',
  },
  // Focus
  '& .MuiOutlinedInput-root.Mui-focused .MuiOutlinedInput-input': {
    color: 'white',
  },
  '& .MuiInputLabel-root.Mui-focused': {
    color: 'white',
  },
  '& .MuiOutlinedInput-root.Mui-focused .MuiOutlinedInput-notchedOutline': {
    borderColor: 'yellow',
  },
};

export const listContainer: SxProps<Theme> = {
  width: '100%',
  position: 'relative',
  overflow: 'auto',
  '& ul': { padding: 0 },
};
