# Documentation for developers

# Project structure

```
/jack_tool
 |-- tsconfig.json
 |-- package.json /** Single package.json */
 |-- src/ /** Repo src */
    |-- renderer/ /** electron frontend components can be reused */
    |-- main/ /** electron main process */
    |-- redux /** can be used across repo */
```

# Requirements

- Use node 18: `nvm use 18`
  - Run `yarn install`

# Development

- Electron apps:

  - Run `yarn start`

# Packaging electron app

- Run `yarn package`
