/* eslint-disable react/jsx-props-no-spreading */
import { TextField, InputAdornment, TextFieldProps, SvgIconProps } from '@mui/material';
import React from 'react';
import SearchIcon from '@mui/icons-material/Search';

export type SearchBarProps = {
  searchText: string;
  onSearch: (searchValue: string) => void;
  placeholder?: string;
  textFieldProps?: TextFieldProps;
  iconProps?: SvgIconProps;
};

export function SearchBar({ onSearch, placeholder = 'Search...', searchText, textFieldProps, iconProps }: SearchBarProps) {
  const [value, setValue] = React.useState(searchText);

  const search = (searchValue: string) => {
    setValue(searchValue);
    onSearch(searchValue);
  };

  React.useEffect(() => {
    setValue(searchText);
  }, [searchText]);

  return (
    <TextField
      name="search-bar"
      value={value}
      onChange={(e) => {
        search(e.target.value);
      }}
      placeholder={placeholder}
      InputProps={{
        startAdornment: (
          <InputAdornment position="start">
            <SearchIcon {...iconProps} />
          </InputAdornment>
        ),
        sx: {
          fontFamily: 'futura',
          '& .MuiInputBase-input': {
            padding: '0.2rem 0 0.2rem 0',
          },
        },
      }}
      {...textFieldProps}
    />
  );
}
