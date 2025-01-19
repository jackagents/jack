import React from 'react';
import { useLeafletContext } from '@react-leaflet/core';
import * as L from 'leaflet';
import './LegendLayer.css';

export function LegendLayer(props) {
  const context = useLeafletContext();
  /* Legend specific */
  L.Control.Legend = L.Control.extend({
    onAdd() {
      const div = L.DomUtil.create('div', 'legend');
      div.innerHTML += `<h4>${props.title}</h4>`;
      for (let i = 0; i < props.legends.length; i += 1) {
        const element = props.legends[i];
        div.innerHTML += `<i style=${
          element.color
            ? `"background: ${element.color}"`
            : `"background-image: url(${element.img});background-repeat: no-repeat;"`
        }></i><span>${element.legend}</span><br>`;
      }
      return div;
    },
  });

  React.useEffect(() => {
    const container = context.layerContainer || context.map;
    const legend = new L.Control.Legend({
      position: props.position || 'bottomright',
    });
    legend.addTo(container);

    return () => {
      container.removeControl(legend);
    };
  }, []);

  return null;
}
