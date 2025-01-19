import { Popover, Typography, styled } from '@mui/material';
import React from 'react';

const EllipsisWithPopOver = styled(Typography)({
  overflow: 'hidden',
  textOverflow: 'ellipsis',
  maxWidth: '100%',
  whiteSpace: 'nowrap',
  fontFamily: 'futura',
});

interface Props {
  label: string;
}

export default function EllipsisLabelWithPopOver({ label }: Props) {
  const typoRef = React.useRef<HTMLSpanElement>(null);
  const [open, setOpen] = React.useState(false);
  const [anchorEl, setAnchorEl] = React.useState<HTMLElement | null>(null);

  const handlePopoverClose = React.useCallback(() => {
    setOpen(false);
    setAnchorEl(null);
  }, []);

  const handleMouseEnter = React.useCallback<React.MouseEventHandler<HTMLSpanElement>>((event) => {
    if (typoRef.current && typoRef.current.offsetWidth < typoRef.current.scrollWidth) {
      setOpen(true);
      setAnchorEl(event.currentTarget);
    }
  }, []);

  return (
    <>
      <EllipsisWithPopOver variant="body1" ref={typoRef} onMouseEnter={handleMouseEnter} onMouseLeave={handlePopoverClose}>
        {label}
      </EllipsisWithPopOver>

      <Popover
        id="mouse-over-popover-eventstemplatesidebar"
        sx={{
          pointerEvents: 'none',
        }}
        open={open}
        anchorEl={anchorEl}
        anchorOrigin={{
          vertical: 'bottom',
          horizontal: 'left',
        }}
        transformOrigin={{
          vertical: 'top',
          horizontal: 'left',
        }}
        onClose={handlePopoverClose}
        disableRestoreFocus
        disableAutoFocus
        disableEnforceFocus
      >
        {label}
      </Popover>
    </>
  );
}
