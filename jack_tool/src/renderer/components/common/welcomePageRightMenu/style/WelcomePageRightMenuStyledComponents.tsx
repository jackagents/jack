import CloseIcon from '@mui/icons-material/Close';
import { styled } from '@mui/material';

export const Button = styled('div')(({ theme }) => ({
  padding: 5,
  borderRadius: 5,
  cursor: 'pointer',
  '& .MuiSvgIcon-root': {
    fontSize: '2em',
  },
  '&:hover': {
    backgroundColor: theme.editor.welcomePage.hoveringBgColor,
  },
}));

export const Right = styled('div')({
  flex: '0 0 40%',
  display: 'flex',
  flexDirection: 'column',
  paddingTop: '5%',
  gap: 30,
});

export const RightMenuList = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'flex-start',
  gap: 10,
  '& > span:first-of-type': {
    marginBottom: 10,
  },
  '&>span': {
    fontSize: '1.5em',
  },
  '&>div': {
    width: '100%',
  },
});

export const RightPathList = styled(RightMenuList)(({ theme }) => ({
  position: 'relative',
  width: '80%',
  overflow: 'overlay',
  '&::-webkit-scrollbar': {
    width: 10,
    height: 5,
  },
  '&::-webkit-scrollbar-track': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb': {
    background: theme.editor.scrollBar.thumbBgColor,
    borderRadius: '2px',
  },
  '&::-webkit-scrollbar-corner': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb:hover': {
    background: theme.editor.scrollBar.thumbHoverBgColor,
  },
}));

export const RightButton = styled(Button)(({ theme }) => ({
  display: 'flex',
  gap: 10,
  maxWidth: '80%',
  alignItems: 'flex-end',
  color: theme.editor.welcomePage.linkColor,
}));

export const RecentFileContainer = styled(Button)(({ theme }) => ({
  position: 'relative',
  display: 'flex',
  flexDirection: 'column',
  gap: 10,
  maxWidth: '80%',
  textOverflow: 'ellipsis',
  whiteSpace: 'nowrap',
  '& > span:first-of-type': {
    color: theme.editor.welcomePage.linkColor,
  },
  '&:hover .deletePath': {
    display: 'block!important',
  },
}));

export const EllipsisText = styled('span')({
  overflow: 'hidden',
  textOverflow: 'ellipsis',
  whiteSpace: 'nowrap',
});

export const CloseButton = styled(CloseIcon)({
  display: 'none!important',
  position: 'absolute',
  top: 5,
  right: 5,
  fontSize: '1em!important',
  borderRadius: '50%',
  '&:hover': {
    backgroundColor: '#8e8e8f',
    color: 'black',
  },
});
