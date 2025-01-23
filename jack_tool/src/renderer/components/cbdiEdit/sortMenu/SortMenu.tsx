import Menu from '@mui/material/Menu';
import ListItemIcon from '@mui/material/ListItemIcon';
import ArticleIcon from '@mui/icons-material/Article';
import {
  SortByAlpha as SortByAlphaIcon,
  Check as CheckIcon,
} from '@mui/icons-material';
import { MenuItem } from '@mui/material';

interface Props {
  anchorEl: HTMLElement | null;
  closeMenu: (e: any) => void;
  conceptType: string | undefined;
  sortWays: {
    [conceptType: string]: 'ascending' | 'descending' | 'default';
  } | null;
  setSortedWays: (sortBy: 'ascending' | 'descending' | 'default') => void;
}

export default function SortMenu(props: Props) {
  const open = Boolean(props.anchorEl);

  function handleClickSortMenuItem(
    sortBy: 'ascending' | 'descending' | 'default'
  ) {
    props.setSortedWays(sortBy);
  }

  let checkedItem = 'default';
  if (
    props.conceptType !== undefined &&
    props.sortWays !== null &&
    props.sortWays[props.conceptType] !== undefined
  ) {
    checkedItem = props.sortWays[props.conceptType];
  }

  return (
    <Menu
      anchorEl={props.anchorEl}
      id="sort-menu"
      open={open}
      onClose={props.closeMenu}
      onClick={props.closeMenu}
      PaperProps={{
        elevation: 0,
        sx: {
          overflow: 'visible',
          filter: 'drop-shadow(0px 2px 8px rgba(0,0,0,0.32))',
          mt: 1.5,
          width: '180px!important',
          '& .MuiAvatar-root': {
            width: 10,
            height: 32,
            ml: -0.5,
            mr: 1,
            backgroundColor: 'white',
          },
          '&:before': {
            content: '""',
            display: 'block',
            position: 'absolute',
            top: 0,
            right: 4,
            width: 8,
            height: 8,
            bgcolor: '#303030',
            transform: 'translateY(-50%) rotate(45deg)',
            zIndex: 0,
          },
        },
      }}
      transformOrigin={{ horizontal: 'right', vertical: 'top' }}
      anchorOrigin={{ horizontal: 'right', vertical: 'bottom' }}
    >
      <MenuItem
        onClick={() => {
          handleClickSortMenuItem('default');
        }}
      >
        <ListItemIcon style={{ marginRight: -10 }}>
          <ArticleIcon color="primary" fontSize="small" />
        </ListItemIcon>
        Default
        <ListItemIcon
          hidden={checkedItem !== 'default'}
          style={{ marginLeft: 40 }}
        >
          <CheckIcon color="primary" fontSize="small" />
        </ListItemIcon>
      </MenuItem>
      <MenuItem
        onClick={() => {
          handleClickSortMenuItem('ascending');
        }}
      >
        <ListItemIcon style={{ marginRight: -10 }}>
          <SortByAlphaIcon color="primary" fontSize="small" />
        </ListItemIcon>
        Ascending
        <ListItemIcon
          hidden={checkedItem !== 'ascending'}
          style={{ marginLeft: 20 }}
        >
          <CheckIcon color="primary" fontSize="small" />
        </ListItemIcon>
      </MenuItem>
      <MenuItem
        onClick={() => {
          handleClickSortMenuItem('descending');
        }}
      >
        <ListItemIcon style={{ marginRight: -10 }}>
          <SortByAlphaIcon color="primary" fontSize="small" />
        </ListItemIcon>
        Descending
        <ListItemIcon
          hidden={checkedItem !== 'descending'}
          style={{ marginLeft: 20 }}
        >
          <CheckIcon color="primary" fontSize="small" />
        </ListItemIcon>
      </MenuItem>
    </Menu>
  );
}
