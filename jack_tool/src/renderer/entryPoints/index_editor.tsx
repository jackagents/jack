import { Marker } from 'leaflet';
import { iconDefault } from 'misc/icons/common/cmIcons';
import { request } from 'projectEvents/common/cmEvents';
import { createRoot } from 'react-dom/client';
import CbdiEditApp from 'components/cbdiEdit/cbdiEditApp/CBDIEDITApp';

// Old global css
import 'renderer/css/index.global.css';
import 'renderer/css/App.css';

// Set Leaflet marker default icon
Marker.prototype.options.icon = iconDefault;

const container = document.getElementById('root')!;
const root = createRoot(container);
window.ipcRenderer.invoke(request.startApp);

root.render(<CbdiEditApp />);
