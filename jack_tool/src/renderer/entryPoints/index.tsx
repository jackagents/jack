import { Marker } from 'leaflet';
import { iconDefault } from 'misc/icons/common/cmIcons';
import { request } from 'projectEvents/common/cmEvents';
import { createRoot } from 'react-dom/client';

// Old global css
import 'renderer/css/index.global.css';
import 'renderer/css/App.css';

// Set Leaflet marker default icon
Marker.prototype.options.icon = iconDefault;

const container = document.getElementById('root')!;
const root = createRoot(container);

import('components/cbdiEdit/cbdiEditApp/CBDIEDITApp')
  .then((editor) => {
    const CbdiEditApp = editor.default;

    return root.render(<CbdiEditApp />);
  })
  .catch((e) => {
    console.error(e);
  });
