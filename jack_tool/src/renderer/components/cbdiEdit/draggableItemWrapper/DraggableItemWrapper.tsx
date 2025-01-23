import cx from 'classnames';
import DragIndicatorIcon from '@mui/icons-material/DragIndicator';
import CloseIcon from '@mui/icons-material/Close';
import { PropsWithChildren } from 'react';
import './DraggableItemWrapper.css';
import { Box } from '@mui/material';

interface Props extends PropsWithChildren {
  itemSelected: number;
  dragHandleProps: object;
  immutable?: boolean;
  styles?: React.CSSProperties;
  onRemoveItem: () => void;
}

const DraggableItemWrapper = ({
  itemSelected,
  immutable,
  dragHandleProps,
  children,
  styles,
  onRemoveItem,
}: Props) => {
  const scale = itemSelected * 0.02 + 1;
  const shadow = itemSelected * 15 + 1;
  const dragged = itemSelected !== 0;
  return (
    <Box
      className={cx('simpleDraggableItem', { dragged })}
      sx={{
        transform: `scale(${scale})`,
        boxShadow: `rgba(0, 0, 0, 0.3) 0px ${shadow}px ${2 * shadow}px 0px`,
        border: (theme) => `1px solid ${theme.editor.detailView.textColor}`,
        ...styles,
      }}
    >
      {immutable || (
        <>
          <DragIndicatorIcon className="dragHandle" {...dragHandleProps} />
          <CloseIcon
            onClick={(e) => {
              e.stopPropagation();
              onRemoveItem();
            }}
            className="dragClose"
          />
        </>
      )}
      {children}
    </Box>
  );
};

export default DraggableItemWrapper;
