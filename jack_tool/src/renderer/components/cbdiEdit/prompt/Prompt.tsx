/* eslint-disable import/prefer-default-export */
/* eslint-disable react/require-default-props */
/* eslint-disable react/jsx-props-no-spreading */
import { Button, Dialog, DialogTitle, DialogContent, DialogActions, Slide, styled } from '@mui/material';
import { TransitionProps } from '@mui/material/transitions';
import * as React from 'react';

/* --------------------------------- Sytles --------------------------------- */
const ConfirmButton = styled(Button)({
  '&&&': {
    color: '#FFFFFF',
    backgroundColor: '#068cfa',
    '&:hover': {
      backgroundColor: '#0574c2',
    },
  },
});

const CancelButton = styled(Button)({
  '&&&': {
    color: '#FFFFFF',
    backgroundColor: '#6B6B6B',
    '&:hover': {
      backgroundColor: '#5C5C5C',
    },
  },
});

const Transition = React.forwardRef(
  (
    props: TransitionProps & {
      children: React.ReactElement<any, any>;
    },
    ref: React.Ref<unknown>,
  ) => <Slide direction="up" ref={ref} {...props} />,
);

interface Props {
  open: boolean;
  title: string;
  content: string | JSX.Element;
  onClose?: () => void;
  onConfirm: () => void;
}

export function Prompt(props: Props) {
  const [open, setOpen] = React.useState(false);

  React.useEffect(() => {
    setOpen(props.open);
  }, [props.open]);

  return (
    <Dialog open={open} TransitionComponent={Transition} keepMounted onClose={props.onClose} aria-describedby="alert-dialog-slide-description">
      <DialogTitle>{props.title}</DialogTitle>
      <DialogContent>{props.content}</DialogContent>
      <DialogActions>
        <ConfirmButton
          onClick={(event) => {
            event.stopPropagation();
            props.onConfirm();
          }}
        >
          Confirm
        </ConfirmButton>
        {props.onClose ? (
          <CancelButton
            onClick={(event) => {
              event.stopPropagation();
              if (props.onClose) {
                props.onClose();
              }
            }}
          >
            Cancel
          </CancelButton>
        ) : null}
      </DialogActions>
    </Dialog>
  );
}
