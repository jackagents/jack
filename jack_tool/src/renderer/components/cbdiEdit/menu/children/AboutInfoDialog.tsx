/* eslint-disable react/jsx-props-no-spreading */
import * as React from 'react';
import Button from '@mui/material/Button';
import { styled } from '@mui/material';
import Dialog from '@mui/material/Dialog';
import DialogTitle from '@mui/material/DialogTitle';
import DialogContent from '@mui/material/DialogContent';
import DialogActions from '@mui/material/DialogActions';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography';
import CloseIcon from '@mui/icons-material/Close';

const BootstrapDialog = styled(Dialog)(({ theme }) => ({
  '& .MuiDialogContent-root': {
    padding: theme.spacing(2),
  },
  '& .MuiDialogActions-root': {
    padding: theme.spacing(1),
  },
}));

const DetailRow = styled('div')({
  display: 'flex',
  justifyContent: 'space-around',
});

const DetailField = styled(Typography)({
  flex: '1 1 0',
  marginLeft: 60,
});
export interface DialogTitleProps {
  id: string;
  children?: React.ReactNode;
  onClose: () => void;
}

const BootstrapDialogTitle = (props: DialogTitleProps) => {
  const { children, onClose, ...other } = props;

  return (
    <DialogTitle sx={{ m: 'auto', p: 2 }} {...other}>
      {children}
      {onClose ? (
        <IconButton
          aria-label="close"
          onClick={onClose}
          sx={{
            position: 'absolute',
            right: 8,
            top: 8,
          }}
        >
          <CloseIcon />
        </IconButton>
      ) : null}
    </DialogTitle>
  );
};

interface DialogProps {
  open: boolean;
  setClose: () => void;
  patchVersion: string;
  nodeVersion: string;
}

export default function AboutInfoDialog(props: DialogProps) {
  const { open, setClose, patchVersion, nodeVersion } = props;

  return (
    <div>
      <BootstrapDialog onClose={setClose} aria-labelledby="customized-dialog-title" open={open}>
        <BootstrapDialogTitle id="customized-dialog-title" onClose={setClose}>
          About JACK Editor
        </BootstrapDialogTitle>
        <DialogContent style={{ minWidth: 400, minHeight: 100 }} dividers>
          <DetailRow>
            <DetailField gutterBottom>Patch version:</DetailField>
            <DetailField gutterBottom>v{patchVersion}</DetailField>
          </DetailRow>
          <DetailRow>
            <DetailField gutterBottom>Electron version:</DetailField>
            <DetailField gutterBottom>v11.5.0</DetailField>
          </DetailRow>

          <DetailRow>
            <DetailField gutterBottom>Node version:</DetailField>
            <DetailField gutterBottom>{nodeVersion}</DetailField>
          </DetailRow>
        </DialogContent>
        <DialogActions>
          <Button autoFocus onClick={setClose}>
            Confirm
          </Button>
        </DialogActions>
      </BootstrapDialog>
    </div>
  );
}
