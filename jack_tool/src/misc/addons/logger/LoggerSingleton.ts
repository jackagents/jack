import { EDebugLogLevel, LoggerBase } from './LoggerBase';

class LoggerSingleton {
  private static instance: LoggerBase;

  constructor() {
    if (!LoggerSingleton.instance) {
      LoggerSingleton.instance = new LoggerBase();
    }
  }

  static getInstance() {
    return LoggerSingleton.instance;
  }

  info = (...args: any[]) => {
    LoggerSingleton.instance.log(EDebugLogLevel.info, ...args);
  };

  error = (...args: any[]) => {
    LoggerSingleton.instance.log(EDebugLogLevel.error, ...args);
  };

  debug = (...args: any[]) => {
    LoggerSingleton.instance.log(EDebugLogLevel.debug, ...args);
  };

  warn = (...args: any[]) => {
    LoggerSingleton.instance.log(EDebugLogLevel.warn, ...args);
  };

  setLogLevel = (level: number) => {
    LoggerSingleton.instance.setLogLevel(level);
  };

  getLogLevel = () => {
    return LoggerSingleton.instance.getLogLevel();
  };
}
const LOGGER = new LoggerSingleton();

export default LOGGER;
