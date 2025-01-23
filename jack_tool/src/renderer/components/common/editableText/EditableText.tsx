import React, { CSSProperties, useEffect, useRef, useState } from 'react';
import EditIcon from '@mui/icons-material/Edit';

interface EditableTextProps {
  value: string;
  onChangeValue: (newValue: string) => void;
  style?: CSSProperties;
}

function EditableText({ value, onChangeValue, style }: EditableTextProps) {
  const [isEditing, setIsEditing] = useState(false);
  const [text, setText] = useState(value);
  const inputRef = useRef<HTMLInputElement | null>(null);

  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus();
    }
  }, [isEditing]);

  const handleEditClick = () => {
    setIsEditing(true);
  };

  const handleBlur = () => {
    setIsEditing(false);
    onChangeValue(text);
  };

  const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setText(e.target.value);
  };

  const onKeyUp = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      setIsEditing(false);
      onChangeValue(text);
    } else if (e.key === 'Escape') {
      setIsEditing(false);
      setText(value); // Revert to original value
    }
  };

  React.useEffect(() => {
    setText(value);
  }, [value, isEditing]);

  return (
    <div
      style={{
        display: 'flex',
        gap: '0.5em',
        fontSize: '1em',
        overflow: 'hidden',
        ...style,
      }}
    >
      {isEditing ? (
        <input
          style={{ fontSize: '1em', width: '100%' }}
          type="text"
          value={text}
          onChange={handleChange}
          onBlur={handleBlur}
          onKeyUp={onKeyUp}
          ref={inputRef}
        />
      ) : (
        <span style={{ width: '100%', overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>{value}</span>
      )}
      {!isEditing && (
        <button
          type="button"
          onClick={handleEditClick}
          style={{ padding: 0, backgroundColor: 'transparent', display: 'flex', alignItems: 'center', color: 'inherit' }}
        >
          <EditIcon sx={{ fontSize: '1em', color: 'inherit' }} />
        </button>
      )}
    </div>
  );
}

export default EditableText;
