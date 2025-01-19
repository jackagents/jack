import { Grid } from '@mui/material';
import {
  ConfigLabelTitle,
  CustomTextfield,
} from 'components/common/base/BaseContainer';
import ConfigBoxStack from 'components/common/configBoxStack/ConfigBoxStack';
import { CONSTANT_STRING } from 'constant/common/cmConstants';
import { IpcRendererEvent } from 'electron';
import BeVehicle from 'main/beBuilders/iwd/vehicle/BeVehicle';
import { iwdRequest, iwdResponse } from 'projectEvents/iwd/iwdEvents';
import React from 'react';
import { IwdVehicleModel } from 'types/iwd/iwdVehicleModel';
import ButtonGroupAdornment from 'components/iwd/configEntity/PoI/ButtonGroupAdornment';

interface Props {
  id: string;
  focus: string;
}

enum Focus {
  none = 'none',
  name = 'name',
}

export default function VehicleConfig({ id, focus }: Props) {
  const [content, setContent] = React.useState<IwdVehicleModel>();

  const [defaultContent, setDefaultContent] = React.useState<IwdVehicleModel>();

  /**
   * Callback when vehicle is updated
   */
  const handleUpdateInfo = React.useCallback(
    (_e: IpcRendererEvent, data: string) => {
      const v = JSON.parse(data) as BeVehicle;

      const { model } = v;

      if (model && model.id === id) {
        setDefaultContent(model);
        setContent(model);
      }
    },
    [id]
  );

  /**
   * Callback on vehicle info response
   */
  const handleVehicleInfo = React.useCallback(
    (_e: IpcRendererEvent, data: string) => {
      const model = JSON.parse(data) as IwdVehicleModel;

      if (model) {
        setDefaultContent(model);
        setContent(model);
      }
    },
    []
  );

  /**
   * Callback for HTML Input element changed events
   */
  const handleChange = React.useCallback(
    (event: React.ChangeEvent<HTMLInputElement>) => {
      if (!content) return;

      switch (event.target.name) {
        case 'name':
          //   if (!RegExp.LatFloatStringReg.test(event.target.value)) {
          //     return;
          //   }

          setContent({ ...content, name: event.target.value });
          break;

        default:
          break;
      }
    },
    [content]
  );

  /**
   * Callback when click button save
   */
  const handleClickBtnSave = React.useCallback(() => {
    // Save new content and dispatch to backend
    window.ipcRenderer.send(iwdRequest.vehicle.update, JSON.stringify(content));
  }, [content]);

  /**
   * Callback when click button cancel
   */
  const handleClickBtnCancel = React.useCallback(() => {
    if (!content || !defaultContent) return;

    // Clear value
    switch (focus) {
      case Focus.name:
        setContent({
          ...content,
          name: defaultContent.name,
        });
        break;

      default:
        break;
    }
  }, [focus, content, defaultContent]);

  React.useEffect(() => {
    // Ask for the current select vehicle info from backend
    window.ipcRenderer.send(iwdRequest.vehicle.info, id);

    window.ipcRenderer.on(iwdResponse.vehicle.info, handleVehicleInfo);
    window.ipcRenderer.on(iwdResponse.vehicle.update, handleUpdateInfo);

    return () => {
      window.ipcRenderer.removeAllListeners(iwdResponse.vehicle.info);
      window.ipcRenderer.removeListener(
        iwdResponse.vehicle.update,
        handleUpdateInfo
      );
    };
  }, [id]);

  /**
   * Trigger on id from parent changed
   */
  React.useEffect(() => {
    // Request the info again to update
    window.ipcRenderer.send(iwdRequest.vehicle.info, id);
  }, [id]);

  return (
    <Grid>
      <ConfigLabelTitle>Vehicle Info</ConfigLabelTitle>
      <ConfigBoxStack>
        <CustomTextfield
          label={CONSTANT_STRING.ID}
          name={CONSTANT_STRING.ID}
          value={content?.id || ''}
          disabled
        />

        <CustomTextfield
          label={CONSTANT_STRING.TYPE}
          name={CONSTANT_STRING.TYPE}
          value={content?.type || ''}
          disabled
        />

        <CustomTextfield
          label={CONSTANT_STRING.STATUS}
          name={CONSTANT_STRING.STATUS}
          value={content?.status || ''}
          disabled
        />

        <CustomTextfield
          label={CONSTANT_STRING.NAME}
          name={CONSTANT_STRING.NAME}
          value={content?.name || ''}
          onChange={handleChange}
          InputProps={{
            endAdornment: (
              <ButtonGroupAdornment
                onClickCancel={handleClickBtnCancel}
                onClickSave={handleClickBtnSave}
                focus={focus === Focus.name}
              />
            ),
          }}
        />
      </ConfigBoxStack>
    </Grid>
  );
}
