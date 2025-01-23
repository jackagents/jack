import {
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogContentText,
  TextField,
  DialogActions,
} from '@mui/material';
import * as React from 'react';
import * as RegEx from 'misc/regex/regex';

interface Props {
  open: boolean;
  title: string;
  content: string;
  defaultValue: string;
  onClose: () => void;
  onConfirm: (id: number) => void;
}

export function BasePrompt(props: Props) {
  const [open, setOpen] = React.useState(false);
  const [id, setId] = React.useState(props.defaultValue);

  React.useEffect(() => {
    setOpen(props.open);
  }, [props.open]);

  const handleClose = React.useCallback(() => {
    props.onClose();
  }, []);

  const handleClick = React.useCallback(() => {
    props.onConfirm(parseInt(id, 10));
  }, [id]);

  const handleChange = React.useCallback(
    (e: React.ChangeEvent<HTMLInputElement>) => {
      if (!RegEx.ThreeDigitsNumberReg.test(e.target.value)) return;
      setId(e.target.value);
    },
    []
  );

  return (
    <Dialog open={open} onClose={handleClose}>
      <DialogTitle>{props.title}</DialogTitle>
      <DialogContent>
        <DialogContentText>{props.content}</DialogContentText>
        <TextField
          autoFocus
          margin="dense"
          id="id"
          label="Id"
          type="input"
          fullWidth
          variant="standard"
          value={id}
          onChange={handleChange}
        />
      </DialogContent>
      <DialogActions>
        <Button onClick={handleClose}>Cancel</Button>
        <Button onClick={handleClick}>Confirm</Button>
      </DialogActions>
    </Dialog>
  );
}
