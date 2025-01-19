import React from 'react';
import { Menu, MenuItem, styled } from '@mui/material';
import SortIcon from '@mui/icons-material/Sort';

const SortIconButton = styled(SortIcon)(({ theme }) => ({
  '&:hover': { color: theme.sceanario.aewcf.highlight, cursor: 'pointer' },
}));

export interface SortButtonProps {
  selectedOption?: string;
  sortOptions: readonly string[];
  onSortSelect: (option: string) => void;
}

export default function SortButton({ selectedOption, sortOptions, onSortSelect }: SortButtonProps) {
  const [anchorEl, setAnchorEl] = React.useState<Element | null>(null);

  const handleClick: React.MouseEventHandler<SVGSVGElement> = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handleClose = () => {
    setAnchorEl(null);
  };

  const handleSortSelect = (option: string) => {
    onSortSelect(option);
    handleClose();
  };

  return (
    <>
      <SortIconButton onClick={handleClick} />

      <Menu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={handleClose} MenuListProps={{ sx: { padding: 0 } }}>
        {sortOptions.map((option) => (
          <MenuItem
            key={option}
            onClick={() => handleSortSelect(option)}
            sx={{
              fontFamily: 'futura',
              backgroundColor: selectedOption === option ? 'lightblue' : undefined,
              ':hover': { backgroundColor: selectedOption === option ? 'lightblue' : undefined },
            }}
          >
            {option}
          </MenuItem>
        ))}
      </Menu>
    </>
  );
}
