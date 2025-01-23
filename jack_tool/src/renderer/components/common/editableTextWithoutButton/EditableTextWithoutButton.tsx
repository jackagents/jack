import React, { CSSProperties, useEffect, useRef, useState } from 'react';

interface EditableTextWithoutButtonProps {
  value: string;
  onChangeValue: (newValue: string) => void;
  isEditing: boolean;
  setIsEditing: (isEditing: boolean) => void;
  style?: CSSProperties;
}

function EditableTextWithoutButton({ value, onChangeValue, isEditing, setIsEditing, style }: EditableTextWithoutButtonProps) {
  const [text, setText] = useState(value);
  const inputRef = useRef<HTMLInputElement | null>(null);

  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus();
    }
  }, [isEditing]);

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

  // Always set text in textbox to original value.
  // Set textbox value to original value when start editing.
  React.useEffect(() => {
    setText(value);
  }, [value, isEditing]);

  return (
    <div
      style={{
        fontSize: '1em',
        overflow: 'hidden',
        textOverflow: 'ellipsis',
        whiteSpace: 'nowrap',
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
        <span style={{ width: '100%' }}>
          {/* // Display the original value not the edited text, 
          so that when it fails to update, it will be reverted. 
          https://gitlab.aosgrp.net/applications/aewcf/-/issues/1002 */}
          {value}
        </span>
      )}
    </div>
  );
}

export default EditableTextWithoutButton;
