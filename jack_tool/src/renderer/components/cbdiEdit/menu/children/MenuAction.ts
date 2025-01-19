export default interface MenuAction {
  label?: string;
  submenu?: MenuAction[];
  accelerator?: string;
  onClick?: () => void;
}
