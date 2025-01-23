import { TextField } from '@mui/material';
import React from 'react';

interface CustomTreeItemTextfieldProps {
  label: React.ReactNode;
  style?: React.CSSProperties;
  component?: React.ElementType;
  className?: string;
  setEdit: React.Dispatch<React.SetStateAction<boolean>>;
  onUpdateName: (value: string) => void;
}

export default function TreeItemRenameTextField({ label, style, component, className, setEdit, onUpdateName }: CustomTreeItemTextfieldProps) {
  const [value, setValue] = React.useState<string>(label as string);
  const inputRef = React.useRef<HTMLDivElement>(null);

  const handleChange = (evt: React.ChangeEvent<HTMLInputElement>) => {
    setValue(evt.target.value);
  };

  const handleBlur = React.useCallback(() => {
    if (value === label) {
      setEdit(false);
      return;
    }

    // Update name
    onUpdateName(value);

    // Turn off textfield editing
    setEdit(false);
  }, [value]);

  const handleKeyUp: React.KeyboardEventHandler<HTMLDivElement> = React.useCallback(
    (evt) => {
      switch (evt.code.toLowerCase()) {
        case 'numpadenter':
        case 'enter': {
          // Apply
          if (inputRef.current) {
            inputRef.current.blur();
          }
          break;
        }
        case 'escape': {
          setEdit(false);
          break;
        }
        default:
          break;
      }
    },
    [inputRef],
  );

  return (
    <TextField
      InputProps={{
        style: {
          height: '30px',
          backgroundColor: 'white',
          ...style,
        },
      }}
      component={component}
      className={className}
      inputRef={inputRef}
      autoFocus
      value={value}
      onChange={handleChange}
      onBlur={handleBlur}
      onKeyUp={handleKeyUp}
    />
  );
}
