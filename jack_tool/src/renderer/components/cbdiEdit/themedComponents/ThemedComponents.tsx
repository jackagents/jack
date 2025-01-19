/* eslint-disable react/jsx-props-no-spreading */
import { ReactNode } from 'react';
import { Select, MenuItem, SelectProps, useTheme, styled, MenuItemProps, MenuProps, Menu, CSSObject } from '@mui/material';

type ThemedSelectProps = SelectProps & {
  hasborder?: string;
  hasborderbottom?: string;
} & {
  children: ReactNode;
};

const StyledSelect = styled(Select)({
  borderRadius: 0,
  overflow: 'hidden',
  width: '100%',
  fontSize: 14,
  height: 28,
  boxShadow: 'none',
  '& .MuiOutlinedInput-notchedOutline': { border: 0 },
});

export function ThemedSelect({ children, ...props }: ThemedSelectProps) {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();

  return (
    <StyledSelect
      displayEmpty
      sx={{
        border: props.hasborder ? `1px solid ${theme.editor.select.textColor}` : undefined,
        borderBottom: props.hasborderbottom ? `1px solid ${theme.editor.select.textColor}` : undefined,
        color: props.value === '' ? theme.editor.select.emptyTextColor : theme.editor.select.textColor,
        '& .Mui-disabled': {
          WebkitTextFillColor: theme.editor.select.disableColor,
        },
        '& .MuiSvgIcon-root': {
          fill: theme.editor.select.textColor,
        },
      }}
      inputProps={{
        MenuProps: {
          MenuListProps: {
            sx: {
              border: `1px solid ${theme.editor.select.menuItemBorderColor}`,
              padding: 0,
              backgroundColor: theme.editor.select.menuItemBgColor,
            },
          },
        },
      }}
      {...props}
    >
      {children}
    </StyledSelect>
  );
}

type CustomMenuItemProps = MenuItemProps & {
  children: ReactNode;
};

const StyledMenuItem = styled(MenuItem)(({ theme }) => ({
  fontSize: 14,
  minWidth: 150,
  height: 28,
  color: theme.editor.select.menuItemTextColor,
  '&.Mui-selected': {
    backgroundColor: 'transparent',
    '&:hover': {
      color: theme.editor.select.menuItemActiveTextColor,
      backgroundColor: theme.editor.select.menuItemActiveBgColor,
    },
  },
  '&:hover': {
    color: theme.editor.select.menuItemActiveTextColor,
    backgroundColor: theme.editor.select.menuItemActiveBgColor,
  },
}));

export function ThemedMenuItem({ children, ...props }: CustomMenuItemProps) {
  return <StyledMenuItem {...props}>{children}</StyledMenuItem>;
}

export const ThemedButton = styled('button')(({ theme }) => ({
  color: theme.editor.detailView.textColor,
  cursor: 'pointer',
  padding: 0,
  backgroundColor: 'transparent',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
}));

type ThemedMenuProps = MenuProps & {
  children: ReactNode;
};

export function ThemedMenu({ children, ...props }: ThemedMenuProps) {
  return (
    <Menu
      MenuListProps={{
        sx: {
          padding: 0,
          backgroundColor: (theme) => theme.editor.select.menuItemBgColor,
        },
      }}
      {...props}
    >
      {children}
    </Menu>
  );
}

export const GridBackgroundContainer = styled('div')(({ theme }) => ({
  backgroundImage:
    `linear-gradient(to right, ${theme.editor.graphView.gridLinearColor} 1px, transparent 1px),` +
    `linear-gradient(to bottom, ${theme.editor.graphView.gridLinearColor} 1px, transparent 1px)`,
  backgroundSize: '20px 20px',
  margin: 'auto',
  backgroundColor: theme.editor.graphView.bgColor,
}));

interface ThemedFieldWithBorderProps {
  customStyles?: CSSObject;
}

export const ThemedFieldWithBorder = styled('div')<ThemedFieldWithBorderProps>(({ theme, customStyles = {} }) => ({
  fontSize: 14,
  minWidth: 120,
  height: 28,
  color: theme.editor.detailView.textColor,
  border: `1px solid ${theme.editor.detailView.textColor}`,
  display: 'flex',
  alignItems: 'center',
  textIndent: 14,
  overflow: 'hidden',
  whiteSpace: 'nowrap',
  ...customStyles, // Merge custom styles with the default styles
}));

export const ThemedOverflowScrollContainer = styled('div')(({ theme }) => ({
  width: '100%',
  height: '100%',
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
