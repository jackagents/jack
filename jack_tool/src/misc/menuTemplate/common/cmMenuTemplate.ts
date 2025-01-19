import { request } from 'misc/events/common/cmEvents';
import MenuAction from 'types/common/cmTypes';

const cmAppMenuTemplate: MenuAction = {
  submenu: [
    {
      label: 'File',
      submenu: [
        {
          label: 'New',
          submenu: [
            {
              label: 'Project',
              onClick: () => {
                window.ipcRenderer.invoke(request.project.new);
              },
            },
          ],
        },
      ],
    },
    {
      label: 'Edit',
      submenu: [
        {
          label: 'Undo',
          onClick: () => {
            console.log('You clicked on Edit -> Undo');
          },
        },
        {
          label: 'Redo',
          onClick: () => {
            console.log('You clicked on Edit -> Redo');
          },
        },
      ],
    },
    {
      label: 'Help',
      submenu: [
        {
          label: 'About',
          onClick: () => {
            window.ipcRenderer.invoke(request.project.about);
          },
        },
      ],
    },
  ],
};

export default cmAppMenuTemplate;
