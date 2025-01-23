import React from 'react';
import { useSpring, animated } from 'react-spring';

interface Props {
  value: number;
  label: string;
  direction?: 'horizontal' | 'vertical';
  min?: number;
  max?: number;
  width?: number;
  height?: number;
  labelStyle?: React.CSSProperties;
}

/**
 * Animated linear gauge
 * @param param0
 * @returns
 */
export default function LinearGauge({ label, value, direction = 'vertical', min = 0, max = 100, width = 30, height = 100, labelStyle }: Props) {
  const offSetWidth = direction === 'vertical' ? 30 : 100;
  const offSetHeight = 20;
  const range = max - min;
  const percentage = ((value - min) / range) * 100;

  // SVG path for the maximum value
  const maxValuePath = React.useMemo(
    () =>
      direction === 'horizontal' ? `M0,0 L${width},0 L${width},${height} L0,${height} Z` : `M0,${height} L0,0 L${width},0 L${width},${height} Z`,
    [direction, height, width],
  );

  // Animation properties for the SVG path
  const pathAnimation = useSpring({
    d:
      direction === 'horizontal'
        ? `M0,0 L${((100 - percentage) / 100) * width},0 L${((100 - percentage) / 100) * width},${height} L0,${height} Z`
        : `M0,${height} L0,${((100 - percentage) / 100) * height} L${width},${((100 - percentage) / 100) * height} L${width},${height} Z`,
    config: { duration: 500 }, // Adjust duration as needed
  });

  return (
    <div>
      <svg
        width={width + offSetWidth}
        height={height + offSetHeight}
        viewBox={`-${offSetWidth / 2} -${offSetWidth - 5} ${width + offSetWidth} ${height + offSetHeight + 5}`}
      >
        {/* Path for the maximum value, not filled */}
        <path d={maxValuePath} fill="none" stroke="rgba(0, 123, 255, 0.5)" strokeWidth="1" />

        <animated.path d={pathAnimation.d} fill="rgba(0, 123, 255, 1)" stroke="rgba(0, 123, 255, 1)" strokeWidth="1" />

        {/* Labels for the start and end of the gauge */}
        {direction === 'horizontal' && (
          <>
            <text
              x={-30}
              y={height / 2} // +5 for padding
              textAnchor="middle"
              dominantBaseline="hanging"
              color="grey"
              style={{ ...labelStyle }}
            >
              {label}
            </text>
            <text x={width / 2} y={height / 2} textAnchor="middle" dominantBaseline="hanging" fill="grey">
              {`${percentage.toFixed(percentage < 10 ? 1 : 0)}%`}
            </text>
          </>
        )}

        {direction === 'vertical' && (
          <>
            <text x={width / 2} y={-offSetHeight} textAnchor="middle" dominantBaseline="hanging" fill={labelStyle?.color} style={{ ...labelStyle }}>
              {label}
            </text>
            <text x={width / 2} y={height / 2 - 5} textAnchor="middle" dominantBaseline="hanging" fill={labelStyle?.color} style={{ ...labelStyle }}>
              {`${percentage.toFixed(percentage < 10 ? 1 : 0)}%`}
            </text>
          </>
        )}
      </svg>
    </div>
  );
}
