import {
  Checkbox,
  Grid,
  Stack,
  styled,
  TextField,
  Typography,
  Switch,
  FormGroup,
  FormControlLabel,
  ButtonGroup,
  Button,
  ListItemButton,
  ListItem,
  ListItemText,
} from '@mui/material';
import { SearchBar } from 'components/common/searchBar/SearchBar';
import { Z_INDEX } from 'misc/constant/common/cmConstants';

/* ************************************************************************************************
 * Absolute Grids
 * *********************************************************************************************** */
export const Fluid = styled('div')({
  width: '100%',
  height: '100%',
});

export const Row = styled('div')({
  position: 'absolute',
  width: '100%',
});

export const Col = styled('div')({
  position: 'absolute',
  height: '100%',
});

/* ************************************************************************************************
 * Flex Grids
 * *********************************************************************************************** */
export const Flex = styled('div')({
  width: '100%',
  height: '100%',
  display: 'flex',
  flex: '1 1 auto',
});

export const FlexRow = styled(Flex)({
  flex: '0 1 auto',
  flexFlow: 'row nowrap',
});

export const FlexCol = styled(Flex)({
  flex: '0 1 auto',
  flexFlow: 'column nowrap',
});

export const Between = styled(Flex)({
  justifyContent: 'space-between',
  alignItems: 'center',
});

/* ************************************************************************************************
 * Drag Region
 * *********************************************************************************************** */
export const DragRegion = styled('div')({
  width: '100%',
  height: '100%',
  WebkitAppRegion: 'drag',
});

/* ************************************************************************************************
 * List
 * *********************************************************************************************** */
export const List = styled('ul')({
  listStyleType: 'none',
  padding: 0,
  margin: 0,
});

/* ************************************************************************************************
 * Custom
 * *********************************************************************************************** */

export const SideMenuButtonGroup: any = styled(ButtonGroup)({
  width: '100%',
});
export const SideMenuButton = styled(Button)(({ theme }) => ({
  width: '100%',
  height: '50px',
  padding: '.5em',
  border: '2px solid transparent',
  color: theme.custom?.sideMenu.iconColor,
  '& .MuiSvgIcon-root': {
    fontSize: '30px',
  },
  '&.checked': {
    color: theme.custom?.sideMenu.checkedIconColor,
    borderLeft: theme.custom?.border.left,
  },
}));

export const SideMenuRoot = styled(FlexCol)(({ theme }) => ({
  justifyContent: 'space-between',
  backgroundColor: theme.custom?.sideMenu.bgColor,
}));

export const Root = styled(Fluid)({
  overflow: 'hidden',
});

export const MenubarRow = styled(Row)({
  height: '2em',
  top: 0,
});

export const ClientRow = styled(Row)({
  top: '4.5em',
  bottom: 0,
});

export const ToolbarRow = styled(Row)({
  height: '2.5em',
  top: '2em',
});

export const Container = styled(Grid)(() => ({
  width: 'auto',
  alignItems: 'center',
}));

export const CustomSwitch = styled(Switch)(({ theme }) => ({
  padding: 8,
  '& .MuiSwitch-switchBase.Mui-checked': {
    color: theme.custom?.switch.switchBaseCheckedColor,
  },
  '& .MuiSwitch-switchBase.Mui-checked + .MuiSwitch-track': {
    backgroundColor: theme.custom?.switch.switchBaseCheckedTrackBgColor,
  },
  '&.Mui-disabled': {
    backgroundColor: theme.custom?.switch.switchDisabledColor,
  },
  '& .MuiSwitch-track': {
    borderRadius: 22 / 2,
    backgroundColor: theme.custom?.switch.switchTrackBgColor,
    '&:before, &:after': {
      content: '""',
      position: 'absolute',
      top: '50%',
      transform: 'translateY(-50%)',
    },
    '&:before': {
      left: 12,
    },
    '&:after': {
      right: 12,
    },
  },
  '& .MuiSwitch-thumb': {
    boxShadow: 'none',
    width: '1em',
    height: '1em',
    margin: 2,
  },
}));

export const CustomFormGroup = styled(FormGroup)({
  marginLeft: '20px',
  width: '8.7em',
});

export const CustomFormControlLabel = styled(FormControlLabel)(({ theme }) => ({
  '.MuiFormControlLabel-label': {
    color: theme.custom?.text.color,
  },
}));

export const ContentBox = styled(Grid)(({ theme }) => ({
  width: 'auto',
  margin: '15px',
  border: theme.custom?.border.thin,
  height: 'auto',
}));

export const CustomTextfield = styled(TextField)(({ theme }) => ({
  width: 'auto',
  height: 'auto',

  '& p': {
    color: theme.custom?.text.color,
  },
  // Normal
  '& .MuiOutlinedInput-input': {
    color: theme.custom?.text.color,
    fontSize: '1vw',
  },
  '& .MuiInputLabel-root': {
    color: theme.custom?.text.color,
  },
  '& .MuiOutlinedInput-root .MuiOutlinedInput-notchedOutline': {
    borderColor: theme.custom?.border.color.primary,
  },
  // Hover
  '&:hover .MuiOutlinedInput-input': {
    color: theme.custom?.text.color,
  },
  '&:hover .MuiInputLabel-root': {
    color: theme.custom?.text.color,
  },
  '&:hover .MuiOutlinedInput-root .MuiOutlinedInput-notchedOutline': {
    borderColor: theme.custom?.border.color.secondary,
  },
  // Focus
  '& .MuiOutlinedInput-root.Mui-focused .MuiOutlinedInput-input': {
    color: theme.custom?.text.color,
  },
  '& .MuiInputLabel-root.Mui-focused': {
    color: theme.custom?.text.color,
  },
  '& .MuiOutlinedInput-root.Mui-focused .MuiOutlinedInput-notchedOutline': {
    borderColor: theme.custom?.border.color.warn,
  },
  // Disabled
  '& .Mui-disabled': {
    WebkitTextFillColor: theme.custom?.text.color,
  },
}));

export const StyledSearchBar = styled(SearchBar)(({ theme }) => ({
  width: 'auto',
  margin: '10px 15px 15px',

  '& p': {
    color: theme.custom?.text.color,
  },
  // Normal
  '& .MuiOutlinedInput-input': {
    color: theme.custom?.text.color,
    fontSize: '1vw',
  },
  '& .MuiInputLabel-root': {
    color: theme.custom?.text.color,
  },
  '& .MuiOutlinedInput-root .MuiOutlinedInput-notchedOutline': {
    borderColor: theme.custom?.border.color.primary,
  },
  // Hover
  '&:hover .MuiOutlinedInput-input': {
    color: theme.custom?.text.color,
  },
  '&:hover .MuiInputLabel-root': {
    color: theme.custom?.text.color,
  },
  '&:hover .MuiOutlinedInput-root .MuiOutlinedInput-notchedOutline': {
    borderColor: theme.custom?.border.color.secondary,
  },
  // Focus
  '& .MuiOutlinedInput-root.Mui-focused .MuiOutlinedInput-input': {
    color: theme.custom?.text.color,
  },
  '& .MuiInputLabel-root.Mui-focused': {
    color: theme.custom?.text.color,
  },
  '& .MuiOutlinedInput-root.Mui-focused .MuiOutlinedInput-notchedOutline': {
    borderColor: theme.custom?.border.color.warn,
  },
}));

export const SelectTextField = styled(CustomTextfield)(({ theme }) => ({
  padding: '5px',
  width: '100%',
  height: '100%',
  '& .MuiOutlinedInput-root': {
    height: '100%',
  },
  '& .MuiInputLabel-root': {
    padding: '5px',
  },
  '& .MuiSvgIcon-root': {
    color: theme.custom?.text.color,
  },
  '& .MuiSelect-select': {
    marginTop: '.4vw',
  },
}));

export const GreyCheckBox = styled(Checkbox)(({ theme }) => ({
  '& .MuiSvgIcon-root': {
    color: theme.custom?.checkBox.bgColor,
  },
}));

export const ConfigLabelTitle: any = styled(Typography)(({ theme }) => ({
  fontSize: theme.custom?.text.size.medium,
  margin: '10px',
  color: theme.custom?.text.color,
}));

export const CustomStack = styled(Stack)({
  margin: 10,
});

export const GridContainer = styled(Grid)(({ theme }) => ({
  width: '100%',
  zIndex: Z_INDEX.MENU,
  height: '100%',
  position: 'absolute',
  backgroundColor: theme.custom?.backgroundColor,
  flexDirection: 'column',
  justifyContent: 'flex-start',
  alignItems: 'center',
  alignContent: 'center',
}));

export const StyledCustomList = styled(List)(({}) => ({
  width: '100%',
  height: '100%',
  position: 'relative',
  overflow: 'auto',
  '& ul': { padding: 0 },
}));

export const StyledStack = styled(Stack)(({}) => ({
  height: '88vh',
  width: 'inherit',
  marginTop: '5px',
}));

export const GaugeContainer = styled(Stack)(({ theme }) => ({
  width: '33%',
  height: 'auto',
  backgroundColor: theme.custom?.sideMenu.bgColor,
  borderRadius: 4,
  marginBottom: '10px',
  border: `1px solid ${theme.custom?.border.color.primary}`,
  padding: '10px',
  pointerEvents: 'stroke',
}));

export const PopupFluid = styled(Fluid)(() => ({
  position: 'absolute',
  zIndex: 1000,
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'flex-end',
  alignItems: 'center',
  pointerEvents: 'none',
}));

export const GaugeLabel: any = styled(Typography)(({ theme }) => ({
  color: theme.custom?.text.color,
  width: '33%',
  fontSize: '1vw',
}));

export const GaugeList = styled(List)(() => ({
  width: '100%',
  position: 'relative',
  overflow: 'auto',
  '& ul': { padding: 0 },
  maxHeight: '50vh',
}));

export const ItemButton = styled(ListItemButton)(({ theme }) => ({
  height: theme.custom?.list.size.height,
  width: '100%',
}));

export const CustomItemList = styled(List)({
  height: '95%',
  width: '100%',
  position: 'relative',
  overflow: 'auto',
  '& ul': { padding: 0 },
});

export const Item = styled(ListItem)(({ theme }) => ({
  height: theme.custom?.list.size.height,
  width: '100%',
}));

export const ItemText = styled(ListItemText)(({ theme }) => ({
  color: theme.custom?.text.color,
  '& .MuiTypography-root': { fontSize: theme.custom?.text.size.medium },
}));

export const ThemeColorBtn: any = styled(Button)(({ theme }) => ({
  color: theme.custom?.text.color,
}));

/* ************************************************************************************************
 * Text
 * *********************************************************************************************** */
export const TextView = styled('div')({
  height: 25,
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontSize: '.9em',
  lineHeight: '25px',
  paddingLeft: 5,
  marginRight: 5,
});

/* ---------------------------- Shadow on borders --------------------------- */
export const Shadow = styled('div')({
  position: 'absolute',
  inset: '0 0 0 0',
  overflow: 'overlay',
  '&::-webkit-scrollbar': {
    width: 5,
    height: 5,
  },
  '&::-webkit-scrollbar-track': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb': {
    background: '#ffffff30',
    borderRadius: '2px',
  },
  '&::-webkit-scrollbar-corner': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb:hover': {
    background: '#ffffff50',
  },
  boxShadow: 'inset 0px 0px 12px 0px #000000',
  pointerEvents: 'none',
});
