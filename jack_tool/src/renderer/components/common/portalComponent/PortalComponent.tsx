import React, { PropsWithChildren } from 'react';
import ReactDOM from 'react-dom';

interface Props extends PropsWithChildren {
  id: string;
}
export default function PortalComponent({ id, children }: Props) {
  const portal = React.useMemo(() => {
    const parentElement = document.getElementById(id);

    if (parentElement) {
      return ReactDOM.createPortal(children, parentElement);
    }

    return null;
  }, [children]);

  return portal;
}
