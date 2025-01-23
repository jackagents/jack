import { Badge, Box, InputAdornment, TextField, styled } from '@mui/material';
import {
  CBDIEditorRootConceptType,
  CBDIEditorTBasicMessageSchema,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import React, { useEffect, useMemo, useState } from 'react';
import './TextEdit.css';
/* -------------------------------- TextEdit -------------------------------- */
interface Props {
  text: string | number;
  onDoneEditing: (text: string | number) => void;
  saveOnChange?: boolean;
  onBlur?: () => void;
  forbiddenValues?: (string | number)[];
  disabled?: boolean;
  autoFocus?: boolean;
  inputType?: CBDIEditorTBasicMessageSchema;
  multiLine?: boolean;
  badgeType?:
    | CBDIEditorRootConceptType.ActionConceptType
    | CBDIEditorRootConceptType.GoalConceptType;
  presetText?: string;
}
/* --------------------------------- Styles --------------------------------- */
const StyledTextField = styled(TextField)(({ theme }) => ({
  fontSize: 12,
  width: '100%',
  '& .MuiOutlinedInput-root:not(.Mui-focused) .MuiOutlinedInput-notchedOutline':
    {
      borderColor: theme.editor.textInput.textColor,
    },
  '& .MuiOutlinedInput-root.Mui-focused': {
    '& .MuiOutlinedInput-notchedOutline': {
      borderColor: theme.editor.textInput.textColor,
    },
  },
  '& .MuiInputBase-input': {
    padding: 4,
    fontSize: 12,
  },
  '& .MuiOutlinedInput-root': {
    minHeight: 31,
    height: '100%',
    padding: 0,
    paddingLeft: 10,
    borderRadius: 0,
  },
}));

export default React.forwardRef((props: Props, ref) => {
  /* ------------------------------- Properties ------------------------------- */
  let prevText: string | number;
  let timeouts: any;
  const [text, setText] = useState(props.text);

  /* ------------------------------ useMemo hooks ----------------------------- */
  const inputRegex = useMemo(() => {
    switch (props.inputType) {
      case CBDIEditorTBasicMessageSchema.I8Type:
        return /^-?([0-9]|[1-9][0-9]|1[01][0-9]|12[0-7]|-)$/;
      case CBDIEditorTBasicMessageSchema.I16Type:
        return /^-?([0-9]|[1-9][0-9]{0,2}|[1-2][0-9]{3}|3[01][0-9]{2}|32[0-6][0-9]{2}|327[0-5][0-9]|3276[0-7]|-)$/;
      case CBDIEditorTBasicMessageSchema.I32Type:
        return /^-?2147483648$|^-?$|^-?[0-9]{1,9}$/;
      case CBDIEditorTBasicMessageSchema.I64Type:
        return /^-?9223372036854775808$|^-?$|^-?[0-9]{1,18}$/;
      case CBDIEditorTBasicMessageSchema.U8Type:
        return /^(0|[1-9][0-9]?|1[01][0-9]?|12[0-8]?|2[0-4][0-9]?|25[0-5]?)$/;
      case CBDIEditorTBasicMessageSchema.U16Type:
        return /^(0|[1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$/;
      case CBDIEditorTBasicMessageSchema.U32Type:
        return /^0$|^[1-9][0-9]*$/;
      case CBDIEditorTBasicMessageSchema.U64Type:
        return /^0$|^[1-9][0-9]*$/;
      case CBDIEditorTBasicMessageSchema.F32Type:
        return /^-|\d+(\.\d*)?$/;
      case CBDIEditorTBasicMessageSchema.F64Type:
        return /^-|\d+(\.\d*)?$/;

      default:
        return undefined;
    }
  }, [props.inputType]);

  const startAdornment = React.useMemo(() => {
    if (!props.badgeType) {
      return undefined;
    }
    const [badgeContent, badgeColor]: [
      string,
      'success' | 'warning' | 'default',
    ] = (() => {
      if (props.badgeType === CBDIEditorRootConceptType.ActionConceptType) {
        return ['Action', 'success'];
      }
      if (props.badgeType === CBDIEditorRootConceptType.GoalConceptType) {
        return ['Goal', 'warning'];
      }
      return ['', 'default'];
    })();
    return (
      <InputAdornment position="start">
        <Badge
          sx={{
            marginLeft: 3,
            marginRight: 4,
          }}
          badgeContent={badgeContent}
          color={badgeColor}
        />
        <Box
          sx={{
            color: (theme) =>
              props.forbiddenValues?.includes(text)
                ? theme.editor.textInput.errorTextColor
                : theme.editor.textInput.textColor,
          }}
        >
          {props.presetText}
        </Box>
      </InputAdornment>
    );
  }, [props.badgeType, props.presetText]);

  /* ----------------------------- useEffect hooks ---------------------------- */
  useEffect(() => {
    prevText = '';
    timeouts = null;
  }, []);

  useEffect(() => {
    if (props.text !== prevText) {
      prevText = props.text;
      setText(props.text);
    }
  }, [props.text]);

  /* -------------------------------- Callbacks ------------------------------- */
  const onTextChange = (mtext: string) => {
    setText(mtext);

    if (props.saveOnChange) {
      if (
        !props.forbiddenValues ||
        (!props.forbiddenValues.includes(mtext) && props.text !== mtext)
      ) {
        clearTimeout(timeouts);
        timeouts = setTimeout(() => props.onDoneEditing(mtext), 500);
      }
    }
  };

  const onBlur = () => {
    const newText = typeof text === 'string' ? text.trim() : text;
    if (
      (!props.forbiddenValues || !props.forbiddenValues.includes(newText)) &&
      props.text !== newText
    ) {
      props.onDoneEditing(newText);
    }
    if (props.onBlur) {
      props.onBlur();
    }
  };

  return (
    <StyledTextField
      ref={ref as React.Ref<HTMLDivElement>}
      InputProps={{
        startAdornment,
        sx: {
          color: (theme) =>
            props.forbiddenValues?.includes(text)
              ? theme.editor.textInput.errorTextColor
              : theme.editor.textInput.textColor,
          '& .Mui-disabled': {
            WebkitTextFillColor: (theme) => theme.editor.textInput.disableColor,
          },
        },
      }}
      multiline={props.multiLine}
      type="string"
      autoFocus={props.autoFocus}
      disabled={props.disabled}
      error={props.forbiddenValues?.includes(text)}
      className={
        props.forbiddenValues?.includes(text) ? 'duplicated-name' : undefined
      }
      variant="outlined"
      value={text === undefined ? '' : text}
      onBlur={onBlur}
      onFocus={(e) => {
        e.target.select();
      }}
      onKeyDown={(e) => {
        if (e.code === 'Enter') {
          onBlur();
        }
      }}
      onChange={(
        event: React.ChangeEvent<{
          value: unknown;
        }>,
      ) => {
        if (
          inputRegex === undefined ||
          event.target.value === '' ||
          inputRegex.test(event.target.value as string)
        ) {
          onTextChange(event.target.value as string);
        }
      }}
    />
  );
});
