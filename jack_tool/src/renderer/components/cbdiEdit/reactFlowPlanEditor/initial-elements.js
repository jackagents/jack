import React from 'react';
import { MarkerType, Position } from 'reactflow';

export const nodes = [
  {
    id: '1',
    type: 'rectangle',
    data: {
      note: '',
      type: 'action',
    },
    position: { x: 250, y: 0 },
  },
  {
    id: '2',
    type: 'rectangle',
    data: {
      note: '',
      type: 'goal',
    },
    position: { x: 100, y: 100 },
  },
  {
    id: '3',
    type: 'rectangle',
    data: {
      note: '',
      type: 'sleep',
    },
    position: { x: 400, y: 100 },
  },
  {
    id: '4',
    type: 'circle',
    data: {
      note: '',
      type: 'start',
    },
    position: { x: 300, y: 200 },
  },
  {
    id: '5',
    type: 'circle',
    data: {
      note: '',
      type: 'end',
    },
    position: { x: 400, y: 200 },
  },
  {
    id: '6',
    type: 'rectangle',
    data: {
      note: '',
      type: 'condition',
    },
    position: { x: 200, y: 200 },
  },
];

export const edges = [];
