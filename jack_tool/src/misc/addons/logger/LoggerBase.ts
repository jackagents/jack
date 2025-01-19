import chalk from 'chalk';

export const EDebugLogLevel = {
  debug: { value: 'debug', level: 0 },
  info: { value: 'info', level: 1 },
  warn: { value: 'warn', level: 2 },
  error: { value: 'error', level: 3 },
};

export class LoggerBase {
  private currentLevel;

  constructor(debugMode = EDebugLogLevel.debug) {
    this.currentLevel = debugMode.level;
  }

  log = (debugMode: { value: string; level: number }, ...args: any[]) => {
    let content = '';

    args.forEach((arg) => {
      content += arg;
      content += ' ';
    });

    if (debugMode.level >= this.currentLevel) {
      let chalkedLevel = '';

      switch (debugMode.level) {
        case EDebugLogLevel.debug.level:
          chalkedLevel = chalk.bgGrey(debugMode.value.toUpperCase());
          break;
        case EDebugLogLevel.error.level:
          chalkedLevel = chalk.bgRed(debugMode.value.toUpperCase());
          break;
        case EDebugLogLevel.info.level:
          chalkedLevel = chalk.bgBlue(debugMode.value.toUpperCase());
          break;
        case EDebugLogLevel.warn.level:
          chalkedLevel = chalk.bgYellow(debugMode.value.toUpperCase());
          break;
        default:
          break;
      }

      console.log(`${chalkedLevel}|${content}`);
    }

    return this;
  };

  setLogLevel = (level: number) => {
    this.currentLevel = level;
  };

  getLogLevel = () => {
    return this.currentLevel;
  };
}
