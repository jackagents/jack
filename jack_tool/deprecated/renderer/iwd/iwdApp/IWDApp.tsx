import Frontend from 'components/iwd/frontend/IwdFrontEnd';
import { Provider } from 'react-redux';
import { store } from 'projectRedux/Store';
import { SnackbarProvider } from 'notistack';
import SliceProvider from 'projectRedux/sliceProvider/SliceProvider';
import { iwdClientSlice } from 'projectRedux/reducers/iwd/iwdClientReducer';

export default function IwdApp() {
  return (
    <SnackbarProvider
      maxSnack={1}
      anchorOrigin={{ horizontal: 'right', vertical: 'bottom' }}
      autoHideDuration={3000}
    >
      <Provider store={store}>
        <SliceProvider slice={iwdClientSlice}>
          <Frontend />
        </SliceProvider>
      </Provider>
    </SnackbarProvider>
  );
}
