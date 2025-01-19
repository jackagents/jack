import { styled } from '@mui/material';
import React, { ChangeEvent } from 'react';

const StyledButton = styled('div')({
  padding: 0,
  backgroundColor: 'transparent',
  fontSize: 16,
});

interface SliderPickerProps {
  value: number;
  onChangeValue: (newValue: number) => void;
  title: string;
  minValue: number;
  maxValue: number;
  step: number;
  /**
   * unit of value showing
   */
  unitText: string;
  /**
   * text for minus button
   */
  minusText?: string;
  /**
   * text for plus button
   */
  plusText?: string;
  style?: React.CSSProperties;
}

function SliderPicker({
  value,
  onChangeValue,
  minValue,
  maxValue,
  step,
  style,
  title,
  minusText = '-',
  plusText = '+',
  unitText,
}: SliderPickerProps) {
  const handleSliderChange = (event: ChangeEvent<HTMLInputElement>) => {
    const newValue = parseFloat(event.target.value);
    onChangeValue(Math.min(Math.max(newValue, minValue), maxValue));
  };

  const handleClickMinusButton = () => {
    onChangeValue(Math.max(value - step, minValue));
  };

  const handleClickPlusButton = () => {
    onChangeValue(Math.min(value + step, maxValue));
  };

  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        width: 300,
        padding: 10,
        border: '1px solid #ccc',
        borderRadius: 8,
        backgroundColor: 'white',
        fontSize: 16,
        ...style,
      }}
    >
      <div style={{ fontSize: 16 }}>{title}</div>
      <div style={{ display: 'flex', alignItems: 'center', width: '100%', height: 50 }}>
        <StyledButton
          aria-label="Minus"
          style={{ pointerEvents: value === minValue ? 'none' : 'auto', marginRight: 10 }}
          onClick={handleClickMinusButton}
        >
          {minusText}
        </StyledButton>
        <div style={{ flexGrow: 1, position: 'relative' }}>
          <input
            type="range"
            min={minValue}
            max={maxValue}
            value={value}
            onChange={handleSliderChange}
            step={step}
            style={{ width: '100%' }}
            aria-label="slider"
          />
          {/* Ruler Markers */}
          <div
            style={{
              position: 'absolute',
              top: 20,
              display: 'flex',
              justifyContent: 'space-between',
              width: '100%',
              paddingLeft: 8,
              paddingRight: 8,
            }}
          >
            {Array.from({ length: Math.max((maxValue - minValue) / step, 1) + 1 }).map((_, i) => {
              const isHidden = i === 0 || i === Math.max((maxValue - minValue) / step, 1);

              return (
                <div
                  key={i as number}
                  style={{
                    width: 2,
                    height: 5,
                    backgroundColor: isHidden ? 'transparent' : '#ccc',
                  }}
                />
              );
            })}
          </div>
        </div>
        <StyledButton
          aria-label="Plus"
          style={{ pointerEvents: value === maxValue ? 'none' : 'auto', marginLeft: 10 }}
          onClick={handleClickPlusButton}
        >
          {plusText}
        </StyledButton>
      </div>
      <div>{`${value.toFixed(1)}${unitText}`}</div>
    </div>
  );
}

export default SliderPicker;
